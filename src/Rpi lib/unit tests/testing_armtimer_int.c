#include "testing_lib/rpi.h"
#include "testing_lib/assert.h"
#include "testing_lib/interrupt.h"
#include "testing_lib/armtimer.h"
#include "testing_lib/timer.h"
#include "testing_lib/mini_uart.h"

static volatile unsigned cnt, period_sum, last_clk, clk;

void arm_timer_handler(){
    cnt++;
    printk("%d\n", cnt);

    clk = timer_get_time();
    period_sum += clk - last_clk;
    last_clk = clk;
}

void arm_timer_clearer(){
    //PUT32(arm_timer_Control, 0x3E0020);
    PUT32(arm_timer_IRQClear, 1);
    printk("clearing int\n");
}


void notmain(){
    uart_init();

    printk("initializing interrupts\n");
    interrupt_init();

    printk("registering irq_handler\n");
    register_irq_handler(ARM_TIMER, arm_timer_handler, arm_timer_clearer);

    printk("initializing arm timer\n");
    timer_interrupt_init(0x1000);

    unsigned time1 = timer_get_time();
    unsigned iter = 0;
    unsigned N = 25;
    while(cnt < N){
        iter++;
    };
    unsigned tot        = timer_get_time() - time1,
             tot_sec    = tot / (1000*1000),
             tot_ms     = (tot / 1000) % 1000,
             tot_usec   = (tot % 1000);

    printk("-----------------------------------------\n");
    printk("summary:\n");
    printk("\t%10d: total iterations\n", iter);
    printk("\t%10d: tot interrupts\n", N);
    printk("\t%10d: iterations / interrupt\n", iter/N);
    printk("\t%10d: average period\n\n", period_sum/(N-1));
    printk("total execution time: %dsec.%dms.%dusec\n", 
                    tot_sec, tot_ms, tot_usec);

    printk("unregistering handler");

    unregister_irq_handler(ARM_TIMER);

    while(1){};
}   
