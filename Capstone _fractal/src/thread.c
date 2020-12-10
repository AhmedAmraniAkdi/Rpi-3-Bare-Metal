#include <stdint.h>
#include "kassert.h"
#include "rpi.h"
#include "thread.h"
#include "interrupt.h"
#include "mmu.h"

static core_tasks_ctlr core_tasks[CORE_NUM] = {0};
static const struct cpu_context empty_context = {0};

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);

void preempt_disable(void){
	unsigned core = CORE_ID();
	core_tasks[core].current->preempt_count = 1; 
}
void preempt_enable(void){
	unsigned core = CORE_ID();
	core_tasks[core].current->preempt_count = 0;
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

void core_timer_clearer(void){
	SET_CORE_TIMER(TIMER_INT_PERIOD);
}

void switch_to(struct task_struct * next){
	unsigned core = CORE_ID();
	if (core_tasks[core].current == next) 
		return;
	struct task_struct *prev = core_tasks[core].current;
	core_tasks[core].current = next;
	context_switch(prev, next);
}

void schedule(void){
	unsigned core = CORE_ID();
	preempt_disable();
	struct task_struct *temp = core_tasks[core].current; 

	int found = 0;
	while(temp != NULL){
		if(temp->state == TASK_READY){ 
			found  = 1;
			break;
		}
		temp = temp->next;
	}

	if(!found){
		preempt_enable();
		return;
	}

	if(core_tasks[core].current->state != TASK_ZOMBIE)
		core_tasks[core].current->state = TASK_READY;
	temp->state = TASK_RUNNING;
	switch_to(temp);
	preempt_enable();
}

void join_task(struct task_struct *p){
	while(p->state != TASK_ZOMBIE)
	 	schedule();
}

void join_all_core_tasks(void){
	unsigned core = CORE_ID();
	while(core_tasks[core].tasks_num > 1){
		schedule();
	}
}

void join_all(void){
	while(core_tasks[0].tasks_num > 1
			|| core_tasks[1].tasks_num != 1
			|| core_tasks[2].tasks_num != 1
			|| core_tasks[3].tasks_num != 1){
		schedule();
	}
	disable_core_timer_int();
}

void exit_task(void){
	unsigned core = CORE_ID();
	preempt_disable();
	core_tasks[core].current->state = TASK_ZOMBIE;
	core_tasks[core].tasks_num--;

	core_tasks[core].current->next->previous = core_tasks[core].current->previous;
	core_tasks[core].current->previous->next = core_tasks[core].current->next;

	schedule(); 
}

// core0 forks the tasks for the other cores when starting
struct task_struct* fork_task(core_number_t core, void (*fn)(void *, void *), void *arg, void *ret){
	if(core_tasks[core].tasks_num >= NR_TASKS)
		panic("max threads reached");

	struct task_struct *p = NULL;
	for(int i = 1; i < NR_TASKS; i++){ // there can be zombies in the slots, so need to go through all the array
		if(core_tasks[core].tasks[i].state == TASK_ZOMBIE){
					p = &core_tasks[core].tasks[i];
					break;
				}
	}

	if(p == NULL){
		panic("could not find a slot for the task to fork core: %d function address: 0x%x\n", core, fn);
	}

	p->cpu_context = empty_context;
	p->next = NULL;
	p->previous = NULL;
	p->state = TASK_READY;
	p->preempt_count = 1; //disable preemtion until starting to execute thread
	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.x21 = (uint64_t)ret;
	p->cpu_context.pc  = (uint64_t)&ret_from_fork;
	uintptr_t stack_pointer = ((uintptr_t) &p->stack[8191]) - 16;
	p->cpu_context.sp  = (uint64_t)pi_roundup(stack_pointer, 16); // this makes sure sp is aligned 16 and not cloberring the other stuff
	demand(is_aligned(p->cpu_context.sp, 16), "sp not aligned 16 when forking task");
	
	if(core_tasks[core].tasks_num == 0){ // if there are no tasks, current is task 0 (main)
		core_tasks[core].tasks[0].next = p;
		core_tasks[core].tasks[0].previous = p;

		p->next = &(core_tasks[core].tasks[0]);
		p->previous = &(core_tasks[core].tasks[0]);

		core_tasks[core].current = &core_tasks[core].tasks[0];
		core_tasks[core].tasks_num++; // we effectively add 2 tasks at the start, p + main
	} else {
		struct task_struct *q = core_tasks[core].current->next;

		core_tasks[core].current->next = p;
		p->next = q;

		q->previous = p;
		p->previous = core_tasks[core].current;
	}
	core_tasks[core].tasks_num++;
	return p;
}

void secondary_cores_threading_init(void){
	unsigned core = CORE_ID();
	if(core_tasks[core].tasks_num == 0) return; // if there was no task for us
	
	enable_core_timer_int();
	//reset_mains_stack();
	core_tasks[core].current->state = TASK_RUNNING;

	SET_CORE_TIMER(TIMER_INT_PERIOD);
	join_all_core_tasks();
	disable_core_timer_int();
	demand(core_tasks[core].tasks_num == 1, "not idle core %d went to sleep, num of tasks %d\n", core, core_tasks[core].tasks_num);
}

void threading_init(void){
	enable_core_timer_int();
	PUT32(CORE_MAILBOX_WRITETOSET + 16, (uintptr_t)&secondary_cores_threading_init);
	PUT32(CORE_MAILBOX_WRITETOSET + 32, (uintptr_t)&secondary_cores_threading_init);
	PUT32(CORE_MAILBOX_WRITETOSET + 48, (uintptr_t)&secondary_cores_threading_init);
	WAKE_CORES();

	core_tasks[0].current->state = TASK_RUNNING;

	SET_CORE_TIMER(TIMER_INT_PERIOD);
}



/**
 * 
 * 
 * #pragma omp parallel for schedule(dynamic, 1)
 *for (int y = 0; y < s->height; y++) {
 *    ... 
 *}
 *This is an Open Multi-Processing (OpenMP) pragma. It’s a higher-level threading API than POSIX or Win32 threads.
 * 
 * 
 */