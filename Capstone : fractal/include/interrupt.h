#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define IRQ_IS_BASIC(x) ((x >= 64 ))
#define IRQ_IS_GPU2(x) ((x >= 32 && x < 64 ))
#define IRQ_IS_GPU1(x) ((x < 32 ))
#define NUM_IRQS 72 // doc is confusing, but the image of p110 makes it clear, 72 total

typedef enum {
    IRQ_BASIC_PENDING = 0x3F00B200,
    IRQ_GPU_PENDING1  = 0x3F00B204,
    IRQ_GPU_PENDING2  = 0x3F00B208,
    FIQ_CONTROL       = 0x3F00B20C,
    IRQ_GPU_ENABLE1   = 0x3F00B210,
    IRQ_GPU_ENABLE2   = 0x3F00B214,
    IRQ_BASIC_ENABLE  = 0x3F00B218,
    IRQ_GPU_DISABLE1  = 0x3F00B21C,
    IRQ_GPU_DISABLE2  = 0x3F00B220,
    IRQ_BASIC_DISABLE = 0x3F00B224,
} interrupt_registers_t;

// 72 possible irqs
// i'll add as i need them
typedef enum {
    SYSTEM_TIMER_1 = 1,
    USB_CONTROLER = 9,
    GPIO_INT0 = 49,
    GPIO_INT1 = 50,
    GPIO_INT2 = 51,
    GPIO_INT3 = 52,
    UART_INT = 57,
    ARM_TIMER = 64,
} irq_number_t;

typedef void (*interrupt_handler_f)(void);
typedef void (*interrupt_clearer_f)(void);

extern void disable_irq(void);
extern void enable_irq(void);
extern void irq_vector_init(void);

void interrupt_init(void);
void register_irq_handler(irq_number_t irq_num, interrupt_handler_f handler, interrupt_clearer_f clearer);
void unregister_irq_handler(irq_number_t irq_num);
int is_pending(irq_number_t irq_num);

void irq_handler(void);
void show_invalid_entry_message(int type, uint64_t esr, uint64_t address);


#endif