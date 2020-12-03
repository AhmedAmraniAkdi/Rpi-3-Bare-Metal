// since probably all cores would be fighting for uart
// I will have them write a value on some static variables and increment it until 10
// and then core 0, task 1 will be printing it.
// this will test threadidng/interrupts/timer

// the good thing? i have an entire folder on src where everything works
// so i can easily test little by little by replacing the code
// should work

#include "rpi.h"
#include "interrupt.h"
#include "thread.h"
#include "mini_uart.h"
#include "kassert.h"
#include <stdint.h>
#include "mmu.h"
#include "VCmailbox.h"
#include "helper_macros.h"

static volatile int a0 = 0; 
static volatile int a1 = 0; // core 0
static volatile int b0 = 0; 
static volatile int b1 = 0; // core 1
static volatile int c0 = 0; 
static volatile int c1 = 0; // core 2
static volatile int d0 = 0; 
static volatile int d1 = 0; // core 3

void task00(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        a0++;
    }
}

void task01(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        a1++;
    }
}

void task10(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        b0++;
    }
}

void task11(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        b1++;
    }
}

void task20(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        c0++;
    }
}

void task21(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        c1++;
    }
}

void task30(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        d0++;
    }
}

void task31(void *arg, void *ret){
    for(int i = 0; i <= 10; i++){
        delay_ms(500);
        d1++;
    }
}

// core 0 executes notmain
int notmain(void){
    set_max_freq();
    uart_init();
    interrupt_init();
    populate_tables();
    mmu_enable();

    printk("early cnfiguration done\n");

    register_irq_handler(bTIMER_CORE0, CORE0, scheduler_tick, core_timer_clearer);
    register_irq_handler(bTIMER_CORE1, CORE1, scheduler_tick, core_timer_clearer);
    register_irq_handler(bTIMER_CORE2, CORE2, scheduler_tick, core_timer_clearer);
    register_irq_handler(bTIMER_CORE3, CORE3, scheduler_tick, core_timer_clearer);

    printk("registered all irq handlers for all 4 cores\n");

    fork_task(CORE0, &task00, NULL, NULL);
    fork_task(CORE0, &task01, NULL, NULL);

    fork_task(CORE1, &task10, NULL, NULL);
    fork_task(CORE1, &task11, NULL, NULL);

    fork_task(CORE2, &task20, NULL, NULL);
    fork_task(CORE2, &task21, NULL, NULL);

    fork_task(CORE3, &task30, NULL, NULL);
    fork_task(CORE3, &task31, NULL, NULL);

    threading_init(); 

    while(1){
        delay_ms(500);
        printk("a0 = %d a1 = %d b0 = %d b1 = %d c0 = %d c1 = %d d0 = %d d1 = %d\n",
        a0, a1, b0, b1, c0, c1, d0, d1);
    }
};