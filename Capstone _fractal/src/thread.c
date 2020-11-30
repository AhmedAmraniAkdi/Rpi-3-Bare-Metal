#include <stdint.h>
#include "rpi.h"
#include "assert.h"
#include "thread.h"
#include "helper_macros.h"
#include "interrupt.h"

static int t_init[CORE_NUM];
static core_tasks_ctlr core_tasks[CORE_NUM] = {0};

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);
extern void WAIT_UNTIL_EVENT(void);
extern WAKE_CORES(void);

// needs change
void threading_init(void){
	int core = CODE_ID();
	ENABLE_CORE_TIMER();
	if(CODE_ID == 0){
		PUT32(CORE_MAILBOX_WRITETOSET + 16, (uintptr_t)&threading_init);
		PUT32(CORE_MAILBOX_WRITETOSET + 32, (uintptr_t)&threading_init);
		PUT32(CORE_MAILBOX_WRITETOSET + 48, (uintptr_t)&threading_init);
		WAKE_CORES();
	}
	// something here to initialize the queues
}

void preempt_disable(int core){
	core_tasks[core].current->preempt_count = 1; 
}
void preempt_enable(int core){
	core_tasks[core].current->preempt_count = 0;
}

// needs change
// core0 forks the tasks for the other cores when starting
struct task_struct* fork_task(core_number_t core, void (*fn)(void *, void *), void *arg, void *ret){
	if(!t_init[core]) // no tasks yet, not even the init task
		thread_init();

	if(core_tasks[core].tasks_num >= NR_TASKS)
		panic("max threads reached");

	preempt_disable(0); // forking will be done by core 0

	struct task_struct *p = NULL;
	int task_id = -1;
	for(int i = 0; i < NR_TASKS; i++){ // there can be zombies in the slots, so need to go through all the array
		if(core_tasks[core].tasks[i].state == TASK_ZOMBIE || core_tasks[core].tasks[i].state == TASK_EMPTY){ //current wont be chosen because it's RUNNING
					p = &core_tasks[core].tasks[i];
					p->task_id = i;
					task_id = i;
					break;
				}
	}

	if(p == NULL || task_id == -1){
		panic("could not find a slot for the task to fork core: %d function address: %x\n", core, fn);
	}

	p->state = TASK_READY;
	p->preempt_count = 1; //disable preemtion until starting to execute thread
	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.x21 = (uint64_t)ret;
	p->cpu_context.pc  = (uint64_t)ret_from_fork;
	uintptr_t stack_pointer = ((uintptr_t) &core_tasks[core].tasks[task_id].stack[8191]) - 16;
	p->cpu_context.sp  = (uint64_t)pi_roundup(stack_pointer, 16); // this makes sure sp is aligned 16 and not cloberring the other stuff

	// easiest : index the new p after the current task.
	struct task_struct *q = core_tasks[core].current->next;
	core_tasks[core].current->next = p;
	p->next = q;

	core_tasks[core].tasks_num++;
	preempt_enable(0);
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

//needs change
void schedule(void){
	unsigned core = CORE_ID();
	if(core_tasks[core].tasks_num == 1) // if only init task return
		return;

	preempt_disable(core);
	struct task_struct *temp = core_tasks[core].current->next; 

	// current has state == running, won't get chosen in loop
	int found = 0;
	while(temp != NULL){
		if(temp->state == TASK_READY){ 
			found  = 1;
			break;
		}
	} 

	if(!found){ // can just check if entry->next  == temp, but this is clearer
		preempt_enable(core);
		return;
	}

	// only possible states are zombie, ready and running for now
	// have to change if adding TASK_WAITING
	if(core_tasks[core].current->state != TASK_ZOMBIE)
		core_tasks[core].current->state = TASK_READY;
	temp->state = TASK_RUNNING;
	switch_to(temp);
	preempt_enable(core);
}

void yield_task(void){
	schedule();
}

// needs change
void join_task(struct task_struct *p){
	while(p->state != TASK_ZOMBIE)
	 	yield_task();
}

//needs change
void exit_task(void){
	int core = CORE_ID();
	preempt_disable(core);
	core_tasks[core].current->state = TASK_ZOMBIE;
	core_tasks[core].tasks_num--;
	// no need to clean the struct because we can simply recycle it
	schedule(); 
	// after this the pc of current(the one zombified) will stop somewhere in context_switch and won't return ever
}


