#include <stdint.h>
#include "rpi.h"
#include "assert.h"
#include "thread.h"
#include "helper_macros.h"

/*
static struct task_struct *current;
static node* entry;
static int nr_tasks;

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

// accepts as arguments a function pointer address and a pointer to the address of it argument, has to be a struct if many
// the task_struct will be defined in main, that way we can still access it after freeing the thread alloc
// also will pass the ret value if any as a pointer, will be defined in main
// can't have the data in stack when it's freed
void fork_task(struct task_struct *p, void*(fn)(void), void *arg, void *ret){
	
	if(nr_tasks >= NR_TASKS)
		panic("max threads reached");

	preempt_disable();

	p->stack_start = (uint64_t*) kmalloc(THREAD_SIZE, 16); //since we are round in up, there would be at least <alingement> bytes unused (up to 16 in this case), np
	if (!p->stack_start)
		panic("could not allocate thread stack for thread %d", nr_tasks);

	p->state = TASK_READY;
	p->preempt_count = 1; //disable preemtion until starting to execute thread
	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.x21 = (uint64_t)ret;
	p->cpu_context.pc  = (uint64_t)ret_from_fork;
	p->cpu_context.sp  = (uint64_t) p->stack_start + (uint64_t) THREAD_SIZE;
	int pid = nr_tasks++;
	//task[pid] = p;
	add_circular(entry, p);
	preempt_enable();
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

	if(nr_tasks == 0) // if only init task return
		return;

	preemt_disable();
	node* temp = entry;
	struct task_struct* temp_ = NULL;
	while(temp->next != entry){
		temp_ = (struct task_struct*) temp->ptr;
		if(temp_->state == TASK_READY){
			break;
		}
		temp = temp->next;
	}
	if(temp->next == entry){
		preemt_enable();
		return;
	}

	current->state = TASK_READY;
	temp_->state = TASK_RUNNING;
	switch_to(temp_);
	preemt_enable();
}

// force scheduling
void yield_task(void){
	schedule();
}

// waits until a thread finishes
// how to return? make another array of free tasks with ret values?
// put ret value on task_struct when forking a new task?
void join_task(struct task_struct *p){
	 if(p->state != TASK_ZOMBIE || p != NULL)
	 	yield_task();
}

//after the code of a task ends pc is sent here
void exit_task(void){
	current->state = TASK_ZOMBIE;
	kfree(current->stack_start);
	remove_circular(entry, current);
	nr_tasks--;
	schedule();
}*/


