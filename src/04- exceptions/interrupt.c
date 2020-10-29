#include "rpi.h"
#include "interrupt.h"

static interrupt_handler_f handlers[NUM_IRQS] = {0};
static interrupt_clearer_f clearers[NUM_IRQS] = {0};

extern void disable_irq(void);
extern void enable_irq(void);
extern void irq_vector_init(void);

// irqs will be enabled when registering the handlers
void interrupt_init(void){
    PUT32(IRQ_BASIC_DISABLE, 0xFFFFFFFF);
    PUT32(IRQ_GPU_DISABLE1, 0xFFFFFFFF);
    PUT32(IRQ_GPU_DISABLE2, 0xFFFFFFFF);
    irq_vector_init();
    enable_irq();
}


