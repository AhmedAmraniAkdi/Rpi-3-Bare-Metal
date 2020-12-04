#ifndef __RPI_H__
#define __RPI_H__

#include <stdint.h>
/***********HELPERS************/

// https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64)
extern void PUT32(unsigned, unsigned);
extern void PUT8(unsigned, unsigned);
extern uint32_t GET32(unsigned);
extern uint8_t GET8(unsigned);
extern void CYCLE_DELAY(unsigned);
extern void DUMMY();
extern void BRANCHTO(unsigned);

extern uint64_t GETPC(void);
extern uint64_t GETEL(void);

extern void DSB(void);
extern void DMB(void);
extern void ISB(void);

extern uint32_t CORE_ID(void);

void reboot(void);
void clean_reboot(void);

void delay(unsigned ticks);
unsigned timer_get_time(void);
void delay_us(unsigned us);
void delay_ms(unsigned ms);
void delay_core_timer(unsigned divisor);


extern void ENABLE_CORE_TIMER(void);
extern void SET_CORE_TIMER(uint32_t);
extern uint64_t READ_TIMER_FREQ(void);
extern uint64_t READ_TIMER(void);
extern uint32_t READ_TIMER_CONTROL(void);

extern void WAIT_UNTIL_EVENT(void);
extern void WAKE_CORES(void);

/************DEBUGGING***********/
// Took these from https://github.com/dddrrreee/cs140e-20win/tree/master/libpi
// will be helpful

// change these two function pointers to control where pi output goes.
extern int (*rpi_putchar)(int c);
void rpi_reset_putc(void);
void rpi_set_putc(int (*fp)(int));

// int putk(const char *msg);
extern int (*putk)(const char *p);

// call to change output function pointers.
void rpi_set_output(int (*putc_fp)(int), int (*puts_fp)(const char *));

int printk(const char *format, ...);

/********MISC**************/

typedef enum{
    CORE0 = 0,
    CORE1 = 1,
    CORE2 = 2,
    CORE3 = 3,
} core_number_t;

/***********helpers************/

#define is_aligned(x, a)        (((x) & ((typeof(x))(a) - 1)) == 0)
#define is_aligned_ptr(x, a)        is_aligned((uintptr_t)x,a)

#define pi_roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

#define NULL ((void*)0)

#endif