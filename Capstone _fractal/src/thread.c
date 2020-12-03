#include <stdint.h>
#include "kassert.h"
#include "rpi.h"
#include "thread.h"
#include "helper_macros.h"
#include "interrupt.h"
#include "mmu.h"

static core_tasks_ctlr core_tasks[CORE_NUM] = {0};
static int initialized = 0;

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);
extern void WAIT_UNTIL_EVENT(void);
extern void WAKE_CORES(void);

void preempt_disable(void){
	unsigned core = CORE_ID();
	core_tasks[core].current->preempt_count = 1; 
}
void preempt_enable(void){
	unsigned core = CORE_ID();
	core_tasks[core].current->preempt_count = 0;
}

void print_registers(uint64_t x0, uint64_t x1){
	printk("x0 0x%x x1 0x%x\n", x0, x1);
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
		panic("could not find a slot for the task to fork core: %d function address: 0x%x\n", core, fn);
	}

	p->state = TASK_READY;
	p->preempt_count = 1; //disable preemtion until starting to execute thread
	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.x21 = (uint64_t)ret;
	p->cpu_context.pc  = (uint64_t)&ret_from_fork;
	uintptr_t stack_pointer = ((uintptr_t) &p->stack[8191]) - 16;
	p->cpu_context.sp  = (uint64_t)pi_roundup(stack_pointer, 16); // this makes sure sp is aligned 16 and not cloberring the other stuff
	demand(is_aligned(p->cpu_context.sp, 16), "sp not aligned 16 when forking task");
	
	// easiest : index the new p after the current task.
	if(core_tasks[core].tasks_num == 0)
		core_tasks[core].current = p;
	else {
	struct task_struct *q = core_tasks[core].current->next;
	core_tasks[core].current->next = p;
	p->next = q;
	}

	core_tasks[core].tasks_num++;
	/*if(core == 3){
		printk("%d\n", core_tasks[core].tasks_num);
	}*/
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

	// current has state == running, won't get chosen in loop
	int found = 0;
	while(temp != NULL){
		if(temp->state == TASK_READY){ 
			found  = 1;
			break;
		}
		temp = temp->next;
		if(temp == core_tasks[core].current) break; // full circle
	}

	if(!found){
		preempt_enable();
		return;
	}

	// only possible states are zombie, ready and running for now
	// have to change if adding TASK_WAITING
	if(core_tasks[core].current->state != TASK_ZOMBIE)
		core_tasks[core].current->state = TASK_READY;
	temp->state = TASK_RUNNING;
	switch_to(temp);
	preempt_enable();
}

void yield_task(void){
	schedule();
}

void join_task(struct task_struct *p){
	while(p->state != TASK_ZOMBIE)
	 	yield_task();
}

void join_all_core_tasks(void){
	unsigned core = CORE_ID();
	while(core_tasks[core].tasks_num > 1){
		schedule();
	}
}

void join_all(void){
	while(core_tasks[0].tasks_num > 1
			|| core_tasks[1].tasks_num != 0
			|| core_tasks[2].tasks_num != 0
			|| core_tasks[3].tasks_num != 0){
		schedule();
	}
}

void secondary_cores_threading_init(void){
	ENABLE_CORE_TIMER();
	unsigned core = CORE_ID();
	//printk("\nhi\n");
	if(!is_mmu_enabled(core)){ // means first time here: we enable mmu and interrupts
		mmu_enable();
		enable_irq();
	}
	if(core_tasks[core].tasks_num != 0){
	
	core_tasks[core].tasks_num++;
	struct task_struct *temp = core_tasks[core].current;
	core_tasks[core].current = &core_tasks[core].tasks[0];
	core_tasks[core].current->next = temp;
	core_tasks[core].current->state = TASK_RUNNING;

	//close list
	struct task_struct *q = core_tasks[core].current;
	while(q->next){
		q = q->next;
	}
	q->next = core_tasks[core].current;

	SET_CORE_TIMER(TIMER_INT_PERIOD);

	/*if(core == 3){
		printk("secondaryinit %d\n", core_tasks[core].tasks_num);
	}*/

	join_all_core_tasks();

	/*if(core == 3){
		printk("secondaryinit after join %d\n", core_tasks[core].tasks_num);
	}*/

	DISABLE_CORE_TIMER();
	
	core_tasks[core].tasks_num--;
	}
	demand(core_tasks[core].tasks_num == 0, "not idle core %d went to sleep, num of tasks %d", core, core_tasks[core].tasks_num);
}

void threading_init(void){
	ENABLE_CORE_TIMER();

	//PUT32(CORE_MAILBOX_WRITETOSET + 16, (uintptr_t)&secondary_cores_threading_init);
	//PUT32(CORE_MAILBOX_WRITETOSET + 32, (uintptr_t)&secondary_cores_threading_init);
	PUT32(CORE_MAILBOX_WRITETOSET + 48, (uintptr_t)&secondary_cores_threading_init);
	WAKE_CORES();

	//add main to tasks array
	if(!initialized){
	core_tasks[0].tasks_num++;
	struct task_struct *temp = core_tasks[0].current;
	core_tasks[0].current = &core_tasks[0].tasks[0];
	core_tasks[0].current->next = temp;
	core_tasks[0].current->state = TASK_RUNNING;
	initialized = 1;
	}

	//close list
	struct task_struct *q = core_tasks[0].current;
	while(q->next){
		q = q->next;
	}
	q->next = core_tasks[0].current;

	SET_CORE_TIMER(TIMER_INT_PERIOD);
}

void exit_task(void){
	unsigned core = CORE_ID();
	preempt_disable();
	core_tasks[core].current->state = TASK_ZOMBIE;
	core_tasks[core].tasks_num--;
	/*if(core == 3){
		printk("exit_task %d\n", core_tasks[core].tasks_num);
	}*/
	// no need to clean the struct because we can simply recycle it
	schedule(); 
	// after this the pc of current(the one zombified) will stop somewhere in context_switch and won't return ever
}

/*void say_bonjour(void){
	printk("\ncore %d awake\n", CORE_ID());
}*/