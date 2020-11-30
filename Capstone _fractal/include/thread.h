#ifndef _THREAD_H
#define _THREAD_H
#include <stdint.h>

#define NR_TASKS 8 
#define CORE_NUM 4

// will define more
#define TASK_EMPTY   0
#define TASK_READY   1
#define TASK_RUNNING 2
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
	uint64_t pc; // link register, not the pc per say, where we come back to after context switchign
};

struct task_struct {
	struct cpu_context cpu_context;
	uint64_t stack_start;
	int state;
	int preempt_count; // 1 if executing critical stuff
	// ok so since i don't want to make it harder by implementing condition variables
	// when allocating stuff on the heap (could also give each core it heap)...
	// I'll just give each task a stack of 8192 from the get go, no need to allocate anything
	// with this i don't need linked lists nor kmalloc with this threading functions
	char stack[8192];
	// stack_start = &stack[127] - 16 + some rounding up to 16 alignement
	// would be different if i didn't have 1GB memory laying around like nothing hahaha
	struct task_struct *next; // points to the next task in line after current, can be zombie/ready
	int task_id; // identifies the task with an ID
};

typedef struct core_tasks_ctrl{
	struct task_struct tasks[NR_TASKS];
	struct task_struct *current; // current running task
	int initialized;
	int tasks_num;
} core_tasks_ctlr;

void threading_init(void);
void schedule(void);
void scheduler_tick(void);
struct task_struct* fork_task(core_number_t core, void (fn)(void *, void*), void *arg, void *ret);
void yield_task(void);
void join_task(struct task_struct *p);

#endif
