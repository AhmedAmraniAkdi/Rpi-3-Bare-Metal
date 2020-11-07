#include "rpi.h"
#include "assert.h"
#include "thread.h"
//#include "thqueue.h"
#include <stdint.h>

// static thread circular q that keeps track of threads
//static task_q q;

// no need for circular q? an array is enough, keeping it simple
static struct task_struct *current;
static struct task_struct * task[NR_TASKS];
static int nr_tasks;
static int task_id;

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
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
	context_switch(prev, next);
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

void scheduler_tick(void){
    if (current->preempt_count != 0) {
        return;
    }
    enable_irq();
    schedule();
    disable_irq();
}


void schedule(void){

	if(nr_tasks == 0 && task_id == 0) // if only init task return
		return;

	preemt_disable();

	//struct task_struct *next, *prev; // ? we round robin, we know which one is the curretn adn which one is the next one
	//find next task that is ready
	// |ti|...|current|...|tj|tj+1|tj+2|...|nr_tasks-1|
	// go from current to nr_tasks - 1
	// if nothing, go from ti to current
	/*for(int i = task_id + 1; i < nr_tasks; i++){
		if(task[i]->state == TASK_READY){

			if(task[i]->type) 
				task[i]->state = TASK_WAITING;
			else
				task[i]->state = TASK_READY;
			
			current = task[i];
			task_id = i;
			switched = 1;
			break;
		} else {
			if(task[i]->check_waiting()){

				if(task[i]->type) 
					task[i]->state = TASK_WAITING;
				else
					task[i]->state = TASK_READY;

				current = task[i];
				task_id = i;
				switched = 1;
				break;
			}
		}
	}*/
	// laziness for not using circular linked list
	int nxt = -1;
	for(int i = task_id + 1; i < nr_tasks; i++){
		if(task[i]->state == TASK_READY){
			nxt = i;
			break;
		}
	}
	if(nxt == -1){
		for(int i = 0; i < task_id + 1; i++){
			if(task[i]->state == TASK_READY){
				nxt = i;
				break;
			}
		}
	}

	if(nxt == -1){
		preemt_enable();
		return;
	}

	task[task_id]->state = TASK_READY;	
	current = task[task_id];
	task_id = nxt;
	task[task_id]->state = TASK_RUNNING;
	switch_to(current);

	preemt_enable();
}