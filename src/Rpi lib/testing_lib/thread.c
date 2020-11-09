#include <stdint.h>
#include "rpi.h"
#include "assert.h"
#include "thread.h"
#include "helper_macros.h"

// no need for circular q? an array is enough, keeping it simple
static struct task_struct *current;
static struct task_struct * task[NR_TASKS];
static int nr_tasks;
static int task_id;

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);

// laziness, using an array as a circular q
int next_thread_idx(uint8_t task_idx){
	int nxt = -1;

	if(nr_tasks == 0)
		return nxt;

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
	return nxt;
}

void preempt_disable(void){
	current->preempt_count++; // why no just 1 / 0 ?
}

void preempt_enable(void){
	current->preempt_count--;
}

// accepts as arguments a function pointer address and a pointer to the address of it argument, has to be a struct if many
// returns thread id
int fork_task(struct task_struct *p, void*(fn)(void), void *arg){
	
	if(nr_tasks >= NR_TASKS)
		return -1;

	preempt_disable();

	p = (struct task_struct *) kmalloc(THREAD_SIZE, 16); //since we are round in up, there would be at least <alingement> bytes unused (up to 16 in this case), np
	if (!p)
		return -1;

	//p->priority = current->priority;
	p->state = TASK_RUNNING;
	//p->counter = p->priority;
	p->preempt_count = 1; //disable preemtion until schedule_tail

	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.pc  = (uint64_t)ret_from_fork;
	p->cpu_context.sp  = (uint64_t)p + THREAD_SIZE;
    // change below for adapting to circular q round robin scheduler
	int pid = nr_tasks++;
	task[pid] = p;	
	preempt_enable();
	return pid;
}

void scheduler_tick(void){
    if (current->preempt_count != 0) {
        return;
    }
	// when entering exception, interrupts are disabled, but we enable them bcs we can have interrupts happen while scheduling
    enable_irq();
    schedule();
    disable_irq();
}

void timer_tick_clear(void){
 // does nothing, need to define it in main program
 // depends on which timer is used
}

void switch_to(struct task_struct * next) 
{
	if (current == next) 
		return;
	struct task_struct * prev = current;
	current = next;
	context_switch(prev, next);
}

//struct task_struct *next, *prev; // ? we round robin, we know which one is the curretn adn which one is the next one
//find next task that is ready
// |ti|...|current|...|tj|tj+1|tj+2|...|nr_tasks-1|
// go from current to nr_tasks - 1
// if nothing, go from ti to current
// laziness for not usinglinked list
// the ppl who came up with all this are geniuses
void schedule(void){

	if(nr_tasks == 0 && task_id == 0) // if only init task return
		return;

	preemt_disable();
	int nxt = next_thread_idx(task_id);

	if(nxt == -1){
		preemt_enable();
		return;
	}

	task[task_id]->state = TASK_READY;
	task_id = nxt;
	task[task_id]->state = TASK_RUNNING;
	switch_to(task[task_id]);
	preemt_enable();
}

// force scheduling
void yield_task(void){
	schedule();
}

// waits until a thread finishes
// how to return? make another array of free tasks with ret values?
// put ret value on task_struct when forking a new task?
void join_task(struct task_struct *p, void* ret){
	 while(p->state != TASK_ZOMBIE || p != NULL){}
}

//after the code of a task ends pc is sent here
void exit_task(void){
	// hear me out on this one
	// when freeing current will still exist in memory bcs we are just changing the location->is_allocated to 0
	// and we can interpret this function just as a continuation of the task, so we don't need to disable preemption
	// need to test
	current->state = TASK_ZOMBIE;
	kfree(current);
	schedule();
}

//void create_thread_pool(uint8_t num_threads){} why no make some sort of threads cache, that way we don't have to keep allocating and deallocating mem?
