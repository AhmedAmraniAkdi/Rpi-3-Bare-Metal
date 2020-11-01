#include "armtimer.h"
#include "rpi.h"

void timer_interrupt_init(unsigned ncycles){

    // Arm timer interrupt IRQ will be enabled on registration

    /* Setup the system timer interrupt */
    /* Timer frequency = Clk/256 * Load --- so smaller = more frequent. */
    PUT32(arm_timer_Load, ncycles);

    // Setup the ARM Timer: experiment by changing the prescale [defined on p197]
    PUT32(arm_timer_Control,
            RPI_ARMTIMER_CTRL_32BIT |
            RPI_ARMTIMER_CTRL_ENABLE |
            RPI_ARMTIMER_CTRL_INT_ENABLE |
            RPI_ARMTIMER_CTRL_PRESCALE_256);
    
}