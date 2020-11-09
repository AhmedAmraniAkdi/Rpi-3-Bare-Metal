#ifndef _THREAD_H
#define _THREAD_H
#include <stdint.h>

//#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#define THREAD_SIZE	4096
#define NR_TASKS 64 

#define INIT_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

// will define more
#define TASK_RUNNING 0
#define TASK_WAITING 1
#define TASK_READY   2
#define TASK_ZOMBIE  3

// callee registers, let'see, there are 2 types:
// caller: if you will use them after the function call, the caller saves them before calling and restores them after calling
// callee: these ones maintain their values through function calls, so it's the callee responsability 
// to store them before using and to restore them after ending the function call
// the function call here is context_switch, so from the point of view of the task, we just entering a function and returning, it doesn't know, that
// we changed to another task
// added d8-d15, lower halves of v8-v15 neon registers(simd/fp)
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
	uint64_t pc;
};

struct task_struct {
	struct cpu_context cpu_context;
	uint64_t state;
	uint8_t preempt_count; // 1 if executing critical stuff
};


void schedule(void);
void scheduler_tick(void);
void timer_tick_clear(void);
int  fork_task(struct task_struct *p, void*(fn)(void), void *arg);
void yield_task(void);
void join_task(struct task_struct *p, void* ret);

#endif
