#include "rpi.h"
#include "assert.h"
#include "thread.h"
#include "thqueue.h"
#include <stdint.h>

// static thread circular q that keeps track of threads
static task_q q;

static struct task_struct *current;
static struct task_struct * task[NR_TASKS];
static int nr_tasks;

extern void ret_from_fork(void);
extern void contex_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);


void preempt_disable(void){
	current->preempt_count++; // why no just 1 / 0 ?
}

void preempt_enable(void){
	current->preempt_count--;
}

void scheduler_tick(){
	if (current->preempt_count >0) {
		return;
	}
    // when entering exception, interrupts are disabled, but we enable them bcs we can have interrupts happen while scheduling
	enable_irq();
	_schedule();
	disable_irq();
}

void switch_to(struct task_struct * next) 
{
	if (current == next) 
		return;
	struct task_struct * prev = current;
	current = next;
	cpu_switch_to(prev, next);
}

// accepts as arguments a function pointer address and a pointer to the address of it argument, has to be a struct if many
int fork_task(uint64_t fn, uint64_t arg)
{
	preempt_disable();
	struct task_struct *p;

	p = (struct task_struct *) kmalloc(THREAD_SIZE, 16); //since we are round in up, there would be at least <alingement> bytes unused (up to 16 in this case), np
	if (!p)
		return 1;

	//p->priority = current->priority;
	p->state = TASK_RUNNING;
	//p->counter = p->priority;
	p->preempt_count = 1; //disable preemtion until schedule_tail

	p->cpu_context.x19 = fn;
	p->cpu_context.x20 = arg;
	p->cpu_context.pc = (uint64_t)ret_from_fork;
	p->cpu_context.sp = (uint64_t)p + THREAD_SIZE;
    // change below for adapting to circular q round robin scheduler
	int pid = nr_tasks++;
	task[pid] = p;	
	preempt_enable();
	return 0;
}

// schedule// schedule_nit // scheduler and Q -> tests and we see if hmed is truly the jebroski hmed