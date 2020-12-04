#ifndef _THREAD_H
#define _THREAD_H
#include <stdint.h>

#define NR_TASKS 9 // 8 + 1 for main
#define CORE_NUM 4

// will define more
#define TASK_READY   1
#define TASK_RUNNING 2
#define TASK_ZOMBIE  0

#define TIMER_INT_PERIOD (READ_TIMER_FREQ()/100)

struct cpu_context {
	uint64_t x19;
	uint64_t x20;
	uint64_t x21;
	uint64_t x22;
	uint64_t x23;
	uint64_t x24;
	uint64_t x25;
	uint64_t x26;
	uint64_t x27;
	uint64_t x28;
	uint64_t d8;
	uint64_t d9;
	uint64_t d10;
	uint64_t d11;
	uint64_t d12;
	uint64_t d13;
	uint64_t d14;
	uint64_t d15;
	uint64_t fp;
	uint64_t sp;
	uint64_t pc; // link register, not the pc per say, where we come back to after context switchign
};

struct task_struct {
	struct cpu_context cpu_context;
	int state;
	int preempt_count; // 1 if executing critical stuff
	char stack[8192];
	struct task_struct *next;
	struct task_struct *previous;
};

typedef struct core_tasks_ctrl{
	struct task_struct tasks[NR_TASKS];
	struct task_struct *current; // current running task
	int tasks_num;
} core_tasks_ctlr;

void threading_init(void);
void schedule(void);
void scheduler_tick(void);
void core_timer_clearer(void);
struct task_struct* fork_task(core_number_t core, void (fn)(void *, void*), void *arg, void *ret);
void join_task(struct task_struct *p);
void join_all(void);

#endif
