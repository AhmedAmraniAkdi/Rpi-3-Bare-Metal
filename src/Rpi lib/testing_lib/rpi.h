#ifndef __RPI_H__
#define __RPI_H__

/***********HELPERS************/

// https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64)
extern void PUT32(unsigned, unsigned);
extern void PUT8(unsigned, unsigned);
extern unsigned GET32(unsigned);
extern unsigned GET8(unsigned);
extern void CYCLE_DELAY(unsigned);
extern void DUMMY();
extern void BRANCHTO(unsigned);

extern void DSB(void);
extern void DMB(void);
extern void ISB(void);

void reboot(void);
void clean_reboot(void);

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

#endif