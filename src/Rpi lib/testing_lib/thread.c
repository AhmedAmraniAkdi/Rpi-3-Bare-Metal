#include <stdint.h>
#include "rpi.h"
#include "assert.h"
#include "thread.h"
#include "helper_macros.h"

static int t_init;

// sched init? main program thread?
static struct task_struct main_task; // will be 0 initialized
static struct task_struct *current = &main_task;
static circularQ_t* entry;

extern void ret_from_fork(void);
extern void context_switch(struct task_struct* prev, struct task_struct* next);
extern void enable_irq(void);
extern void disable_irq(void);

void thread_init(void){
	node *temp = (node *) kmalloc(sizeof(node), 16);
	temp->next = NULL;
	temp->ptr = (uintptr_t) &main_task;
	add_circular(entry, temp);
	t_init = 1;
}

void preempt_disable(void){
	current->preempt_count = 1; 
}
void preempt_enable(void){
	current->preempt_count = 0;
}

// accepts as arguments a function pointer address and a pointer to the address of it argument, has to be a struct if many
// the task_struct will be defined in main, that way we can still access it after freeing the thread alloc
// also will pass the ret value if any as a pointer, will be defined in main
// can't have the data in stack when it's freed
void fork_task(struct task_struct *p, void *(fn)(void *, void *), void *arg, void *ret){
	if(!t_init)
		thread_init();

	if(entry->ptr >= NR_TASKS)
		panic("max threads reached");

	preempt_disable();

	p->stack_start = (uintptr_t) kmalloc(THREAD_SIZE, 16);
	if (!p->stack_start)
		panic("could not allocate thread stack for thread %d", entry->ptr);

	p->state = TASK_READY;
	p->preempt_count = 1; //disable preemtion until starting to execute thread
	p->cpu_context.x19 = (uint64_t)fn;
	p->cpu_context.x20 = (uint64_t)arg;
	p->cpu_context.x21 = (uint64_t)ret;
	p->cpu_context.pc  = (uint64_t)ret_from_fork;
	p->cpu_context.sp  = p->stack_start +  THREAD_SIZE;

	// make node for circular list
	node * temp = (node *) kmalloc(sizeof(node), 16);
	temp->next = NULL;
	temp->ptr = (uintptr_t) p;
	
	add_circular(entry, temp);
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


//find next task that is ready
void schedule(void){

	if(entry->ptr == 1) // if only init task return
		return;

	preempt_disable();
	node *temp = entry->next; 
	struct task_struct *temp_ = NULL;

	// current has state == running, won't get chosen in loop
	int found = 0;
	for(int i = 0; i < entry->ptr; i++, temp = temp->next){
		temp_ = (struct task_struct *) temp->ptr;
		if(temp_->state == TASK_READY){ // normally, the current task will be running, but when removing entry->next can not be running
			found  = 1;
			break;
		}
	} 

	if(!found){ // can just check if entry->next  == temp, but this is clearer
		preempt_enable();
		return;
	}

	// only possible states are zombie, ready and running for now
	// have to change if adding TASK_WAITING
	if(current->state != TASK_ZOMBIE)
		current->state = TASK_READY;
	temp_->state = TASK_RUNNING;
	entry->next = temp; // move entry to new current
	switch_to(temp_);
	preempt_enable();
}

// force scheduling
// funny idea, what happens when you yield yourself, lul
void yield_task(void){
	schedule();
}

// waits until a thread finishes
void join_task(struct task_struct *p){
	while(p->state != TASK_ZOMBIE)
	 	yield_task();
}

//after the code of a task ends pc is sent here
void exit_task(void){

	preempt_disable();
	current->state = TASK_ZOMBIE;
	// we free the stack but still using it until scheduling, will be a problem? do i need ot make stack boundaries???
	// what happens if scheduling happens right in this instant, and in the next task we allocate some memory? -> disable preemption
	kfree((void *) current->stack_start);
	node *temp = entry->next; // current node that points on current context struct
	remove_circular(entry, temp); // we remove it
	// now:
	//	entry points to the node that was after the one we removed
	//	the struct pointed by the entry->next node is not the same as current
	kfree(temp); // we free the node that points to current struct
	schedule(); // if the node that was after was waiting/zombie, won't get chosen, if it was ready yes
	// the miss alignement of entry->next->ptr and current doesn't affect
	// after this the pc of current(the one zombified) will stop somewhere in context_switch and won't return ever

}


