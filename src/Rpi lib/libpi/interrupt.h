/*#ifndef INTERRUPTS_H
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


#endif*/

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define IRQ_IS_BASIC(x) ((x >= 64 && x < 72))
#define IRQ_IS_GPU2(x) ((x >= 32 && x < 64 ))
#define IRQ_IS_GPU1(x) ((x < 32 ))
#define IRQ_IS_ARM_CONTROL(x) ((x >= 72))

// commented this line, this was with only the bmc2837 interrupt peripheric
// #define NUM_IRQS 72 // doc is confusing, but the image of p110 makes it clear, 72 total
// 72 from interrupt peripheric(bcm2837) + 12 (irq sources qa7rev) per core , fiq not included
// https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=221698&p=1359883&hilit=interrupts+multicore#p1359883
// the 64+8 : 64 will only be routed to core 0 (gpu routing) + basic
// the 83 (local timer qa7rev) routed to core 0
// this can be changed later, not for now
// only irq
#define NUM_IRQS (72 + 12 * 4)

#define TIMER_INT_PERIOD 1000 // 1s

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
} interrupt_registers_bmc2837_t;

// core here means cpu core, not videocore(gpu)
// not all of them, since most of them I won't use/can access it directly using cortex registers
typedef enum {
    CORE_TIMER_INTERRUPT_CTRL = 0x40000040, // core 0, for i_core add i*4
    CORE_MAILBOX_INTERRUPT    = 0x40000050, // core 0, for i_core add i*4
    CORE_MAILBOX_WRITETOSET   = 0x40000080, // core 0 mailbox 0, for i mailbox of j core : add j*16+i
    CORE_MAILBOX_WRITETOCLEAR = 0x400000C0, // core 0 mailbox 0, for i mailbox of j core : add j*16+i
    CORE_INTERRUPT_IRQ_SOURCE = 0x40000060, // core 0, for i_core add i*4
    
    LOCAL_TIMER_CONTROL       = 0x40000034, // will be routed to core 0 always
    LOCAL_TIMER_CLEAR_RELOAD  = 0x40000038,
    LOCAL_TIMER_ROUTING       = 0x40000024,
    GPU_ROUTING_INTERRUPT     = 0x4000000C, // will be routed to core 0 always
} interrupt_regusters_qa7rev34_t;

// not all of them
// will prolly only end up using mailbox0 to communicate
typedef enum {
    // bmc2837
    SYSTEM_TIMER_1 = 1,
    ARM_TIMER      = 64,
    // qa7rev
    bTIMER_CORE0   = 73, // per core physical timer (non secure physical timer)
    MAILBOX0_CORE0 = 76,
    MAILBOX1_CORE0 = 77,
    MAILBOX2_CORE0 = 78,
    MAILBOX3_CORE0 = 79,
    LOCAL_TIMER    = 83, // will always be routed to core 0

    bTIMER_CORE1   = 85,
    MAILBOX0_CORE1 = 88,
    MAILBOX1_CORE1 = 89,
    MAILBOX2_CORE1 = 90,
    MAILBOX3_CORE1 = 91,

    bTIMER_CORE2   = 97,
    MAILBOX0_CORE2 = 100,
    MAILBOX1_CORE2 = 101,
    MAILBOX2_CORE2 = 102,
    MAILBOX3_CORE2 = 103,

    bTIMER_CORE3   = 109,
    MAILBOX0_CORE3 = 112,
    MAILBOX1_CORE3 = 113,
    MAILBOX2_CORE3 = 114,
    MAILBOX3_CORE3 = 115,
} irq_number_t;

typedef int core_number_t;
typedef void (*interrupt_handler_f)(void);
typedef void (*interrupt_clearer_f)(void);

extern void disable_irq(void);
extern void enable_irq(void);
extern void irq_vector_init(void);
extern void ENABLE_CORE_TIMER(void);
extern void SET_CORE_TIMER(uint32_t);
extern uint64_t READ_TIMER_FREQ(void);
extern uint64_t READ_TIMER(void);

void core_timer_clearer(void);

void interrupt_init(void);
void register_irq_handler(irq_number_t irq_num, core_number_t core, interrupt_handler_f handler, interrupt_clearer_f clearer);
void unregister_irq_handler(irq_number_t irq_num, core_number_t core);
int is_pending(irq_number_t irq_num);

void irq_handler(void);
void show_invalid_entry_message(int type, uint64_t esr, uint64_t address);


#endif