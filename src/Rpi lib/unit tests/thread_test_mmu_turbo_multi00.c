#include "libpi/rpi.h"
#include "libpi/assert.h"
#include "libpi/interrupt.h"
#include "libpi/armtimer.h"
#include "libpi/timer.h"
#include "libpi/mini_uart.h"
#include "libpi/thread.h"
#include "libpi/mmu.h"
#include "libpi/helper_macros.h"
#include "libpi/VCmailbox.h"

// armtimer for now, will change ti to either sytemtimer or cpu timers
// scheduler_tick() is handler 

extern void SET_CORE_TIMER(unsigned val);
extern void ENABLE_CORE_TIMER(void);
extern void WAKE_CORES(void);
extern void WAIT_UNTIL_EVENT(void);
extern int CORE_ID(void);
extern uint64_t READ_TIMER(void);
extern uint64_t READ_TIMER_FREQ(void);

// being able to reason that the compiler thinks that these
// two won't get updated so it omits them means i learnt a lot
static volatile uint64_t core1_counter00 = 1;
static volatile uint64_t core1_counter01 = 0;

void core1_test_function(void){
    int core = CORE_ID();
    while(1){
        core1_counter01++;
        core1_counter00 = core1_counter00 * core;
    }
}
/*
void arm_timer_clearer(){
    PUT32(arm_timer_IRQClear, 1);
}

void task(void *arg, void *ret){
    int * parameter = (int *) arg;
    while(1){
        delay_ms(500);
        printk("HELLO FROM TASK %d\n", *parameter);
    }
}

void joinable_task(void *arg, void *ret){
    int i = 0;
    while(i < 5){
        delay_ms(500);
        printk("joinable task %d/5\n", i + 1);
        i++;
    }
}
*/
void notmain(){
    set_max_freq();
    uart_init();
    //print_info_mem_freq();

    interrupt_init();

    populate_tables();
    mmu_enable();
    
    /*register_irq_handler(ARM_TIMER, scheduler_tick, arm_timer_clearer);

    struct task_struct pthreads[6];
    int k[5] = {0,1,2,3,4};

    for(int i = 0; i < 5; i++){
        fork_task(&pthreads[i], &task, &k[i], NULL);
    }

    fork_task(&pthreads[5], &joinable_task, NULL, NULL);

    timer_interrupt_init(0x1000);

    for(int i = 0; i < 5; i++){
        delay_ms(500);
        printk("back at main\n");
    }

    printk("will wait for joinable task now\n");
    join_task(&pthreads[5]);*/

    /*while(1){
        delay_ms(500);
        printk("from main: joinable task ended\n");
    }*/

    ENABLE_CORE_TIMER();
    printk("timer freq = %u", READ_TIMER_FREQ());

    delay_ms(500);

    unsigned r = GET32(0x400000D0);
    PUT32(0x400000D0, r);
    PUT32(0x40000090, (uintptr_t)&core1_test_function);
    WAKE_CORES();
    while(1){
        //printk("Time1: %u\n", READ_TIMER());
        delay_ms(1000); // how is this working? system timer is actually implemented in qemu???
        //printk("Timer2: %u\n", READ_TIMER());
        printk("main: core1counter00: %d -- core1counter01: %d\n", core1_counter00, core1_counter01);
    };
}