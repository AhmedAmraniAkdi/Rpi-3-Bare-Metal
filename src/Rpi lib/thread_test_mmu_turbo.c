#include "libpi/rpi.h"
#include "libpi/assert.h"
#include "libpi/interrupt.h"
#include "libpi/armtimer.h"
#include "libpi/timer.h"
#include "libpi/mini_uart.h"
#include "libpi/thread.h"
#include "libpi/helper_macros.h"
#include "libpi/mmu.h"
#include "libpi/VCmailbox.h"

// armtimer for now, will change ti to either sytemtimer or cpu timers
// scheduler_tick() is handler 

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

void notmain(){
    //set_max_freq();
    print_info_mem_freq();
    uart_init();

    populate_tables();
    mmu_enable();
    
    interrupt_init();
    register_irq_handler(ARM_TIMER, scheduler_tick, arm_timer_clearer);

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
    join_task(&pthreads[5]);

    while(1){
        delay_ms(500);
        printk("from main: joinable task ended\n");
    }
}