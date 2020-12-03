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

//extern void SET_CORE_TIMER(unsigned val);
//extern void ENABLE_CORE_TIMER(void);
extern void WAKE_CORES(void);
extern void WAIT_UNTIL_EVENT(void);
//extern uint64_t READ_TIMER(void);
//extern uint64_t READ_TIMER_FREQ(void);
extern uint64_t get_scr(void);

static volatile uint64_t core0 = 0;
static volatile uint64_t core1 = 0;
static volatile uint64_t core2 = 0;
static volatile uint64_t core3 = 0;

static volatile int we_went_in = 0;

void WEAREIN(void){
    we_went_in++;
}


void timer_handler(){
    int core = CORE_ID();

    switch(core){
        case 0:
            core0++;
            break;
        case 1:
            core1++;
            break;
        case 2:
            core2++;
            break;
        case 3:
            core3++;
            break;
    }
}

void timer_clearer(){
    SET_CORE_TIMER(READ_TIMER_FREQ()/100);
}

/*
void core1_test_function(void){
    int core = CORE_ID();
    while(1){
        core1_counter01++;
        core1_counter00 = core1_counter00 * core;
    }
}*/
/*
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

void iddle_task(void){
    ENABLE_CORE_TIMER();
    enable_irq();
    mmu_enable();
    SET_CORE_TIMER(READ_TIMER_FREQ()/100);
    while(1){};
}

void notmain(){
    set_max_freq();
    uart_init();
    interrupt_init();
    populate_tables();
    mmu_enable();
    
    register_irq_handler(bTIMER_CORE0, 0, &timer_handler, &timer_clearer);
    register_irq_handler(bTIMER_CORE1, 1, &timer_handler, &timer_clearer);
    register_irq_handler(bTIMER_CORE2, 2, &timer_handler, &timer_clearer);
    register_irq_handler(bTIMER_CORE3, 3, &timer_handler, &timer_clearer);

    ENABLE_CORE_TIMER();
    SET_CORE_TIMER(READ_TIMER_FREQ()/100);

    delay_ms(500);
    
    for(int i = 1; i < 4; i++){
        unsigned r = GET32(0x400000C0 + i * 16);
        PUT32(0x400000C0 + i * 16, r);
        PUT32(0x40000080 + i * 16, (uintptr_t)&iddle_task);
    }
    
    WAKE_CORES();

    while(1){
        delay_ms(1000);
        printk("core0 = %d; core1 = %d; core2 = %d; core3 = %d\n", core0, core1, core2, core3);
    };
}