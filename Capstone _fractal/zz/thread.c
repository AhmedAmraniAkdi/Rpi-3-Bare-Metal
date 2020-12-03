#include <stdint.h>
#include "../include/assert.h"
#include "rpi.h"
#include "thread.h"
#include "helper_macros.h"
#include "interrupt.h"
#include "mmu.h"

static core_tasks_ctlr core_tasks[CORE_NUM] = {0};

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);
extern void WAIT_UNTIL_EVENT(void);
extern void WAKE_CORES(void);


void preempt_disable(int core){
	core_tasks[core].current->preempt_count = 1; 
}
void preempt_enable(int core){
	core_tasks[core].current->preempt_count = 0;
}

// core0 forks the tasks for the other cores when starting
struct task_struct* fork_task(core_number_t core, void (*fn)(void *, void *), void *arg, void *ret){
	if(core_tasks[core].tasks_num >= NR_TASKS)
		panic("max threads reached");

	struct task_struct *p = NULL;
	// i starts at 1, task 0 is always main
	for(int i = 1; i < NR_TASKS; i++){ // there can be zombies in the slots, so need to go through all the array
		if(core_tasks[core].tasks[i].state == TASK_ZOMBIE || core_tasks[core].tasks[i].state == TASK_EMPTY){ //current wont be chosen because it's RUNNING
					p = &core_tasks[core].tasks[i];
					break;
				}
	}

	if(p == NULL){
		panic("could not find a slot for the task to fork core: %d function address: %x\n", core, fn);
	}

	p->state = TASK_READY;
	p->preempt_count = 1; //disable preemtion until starting to execute thread
	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.x21 = (uint64_t)ret;
	p->cpu_context.pc  = (uint64_t)ret_from_fork;
	uintptr_t stack_pointer = ((uintptr_t) &(p->stack[8191])) - 16;
	p->cpu_context.sp  = (uint64_t)pi_roundup(stack_pointer, 16); // this makes sure sp is aligned 16 and not cloberring the other stuff
	demand(is_aligned(p->cpu_context.sp, 16), "stack pointer not aligned 16");
	
	
	if(core_tasks[core].tasks_num == 0){
		core_tasks[core].current = p;
	} else {
		struct task_struct *q = core_tasks[core].current->next;
		core_tasks[core].current->next = p;
		p->next = q;
	}	

	core_tasks[core].tasks_num++;
	return p;
}

void scheduler_tick(void){
	unsigned core = CORE_ID();
    if (core_tasks[core].current->preempt_count != 0) {
        return;
    }
	// when entering exception, interrupts are disabled, but we enable them bcs we can have interrupts happen while scheduling
    enable_irq();
    schedule();
    disable_irq();
}

void switch_to(struct task_struct * next) 
{
	unsigned core = CORE_ID();
	if (core_tasks[core].current == next) 
		return;
	struct task_struct * prev = core_tasks[core].current;
	core_tasks[core].current = next;
	context_switch(prev, next);
}

void schedule(void){
	unsigned core = CORE_ID();
	if(core_tasks[core].tasks_num == 1) // if only init task return
		return;

	preempt_disable(core);
	struct task_struct *temp = core_tasks[core].current;

	int found = 0;
	while(temp != NULL){
		if(temp->state == TASK_READY ){ 
			found  = 1;
			break;
		}
		temp = temp->next;
		if(temp == core_tasks[core].current){
			break;
		}
	} 
	if(!found){ 
		preempt_enable(core);
		return;
	}

	if(core_tasks[core].current->state != TASK_ZOMBIE)
		core_tasks[core].current->state = TASK_READY;
	temp->state = TASK_RUNNING;
	switch_to(temp);
	preempt_enable(core);
}

void yield_task(void){
	schedule();
}

void join_task(struct task_struct *p){
	while(p->state != TASK_ZOMBIE)
	 	yield_task();
}

void join_all_core_tasks(void){
	int core = CORE_ID();
	while(core_tasks[core].tasks_num > 1){
		schedule();
	}
}

void join_allcores(void){
	while( core_tasks[0].tasks_num > 1
		|| core_tasks[1].tasks_num != 0
		|| core_tasks[2].tasks_num != 0
		|| core_tasks[3].tasks_num != 0){
			schedule();
		}
		DISABLE_CORE_TIMER();
}

void threading_init(void){
	ENABLE_CORE_TIMER();

	/*PUT32(CORE_MAILBOX_WRITETOSET + 16, (uintptr_t)&secondary_cores_init);
	PUT32(CORE_MAILBOX_WRITETOSET + 32, (uintptr_t)&secondary_cores_init);
	PUT32(CORE_MAILBOX_WRITETOSET + 48, (uintptr_t)&secondary_cores_init);
	WAKE_CORES();*/

	//add main to tasks array
	if(!core_tasks[0].initialized){
		core_tasks[0].tasks_num++;

		struct task_struct *q = core_tasks[0].current;
		core_tasks[0].current = &core_tasks[0].tasks[0];
		core_tasks[0].current->next = q;

		// now we close the list of tasks onto itself, so that way it becomes a circle list

		if(core_tasks[0].tasks_num > 1){
			while(q->next){
				q = q->next;
			}
	 		q->next = core_tasks[0].current;
		}

		core_tasks[0].current->state = TASK_RUNNING;
		core_tasks[0].initialized = 1;
		
		SET_CORE_TIMER(TIMER_INT_PERIOD); 
	}
}

void secondary_cores_init(void){
	int core = CORE_ID();
	if(!is_mmu_enabled(core)){ // means first time here: we enable mmu and interrupts
		mmu_enable();
		enable_irq();
	}
	core_tasks[core].tasks_num++;

	struct task_struct *q = core_tasks[core].current;
	core_tasks[0].current = &core_tasks[0].tasks[0];
	core_tasks[0].current->next = q;
	if(core_tasks[core].tasks_num > 1){
		while(q->next){
			q = q->next;
		}
	 	q->next = core_tasks[0].current;
	}
	core_tasks[core].current->state = TASK_RUNNING;
	core_tasks[core].initialized = 1;
	SET_CORE_TIMER(TIMER_INT_PERIOD);

	join_all_core_tasks();
	DISABLE_CORE_TIMER();
	core_tasks[core].tasks_num--;
	core_tasks[core].initialized = 0; // sleep 
	demand(core_tasks[core].tasks_num == 0, "not idle core %d went to sleep", core);
}




void exit_task(void){
	int core = CORE_ID();
	preempt_disable(core);
	core_tasks[core].current->state = TASK_ZOMBIE;
	core_tasks[core].tasks_num--;
	// no need to clean the struct because we can simply recycle it
	schedule(); 
	// after this the pc of current(the one zombified) will stop somewhere in context_switch and won't return ever
}