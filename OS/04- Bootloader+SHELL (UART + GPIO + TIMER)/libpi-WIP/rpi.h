#ifndef __RPI_H__
#define __RPI_H__

// https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64)
extern void PUT32(unsigned, unsigned);
extern unsigned GET32(unsigned);
extern void CYCLE_DELAY(unsigned);
extern void DUMMY();
extern void BRANCHTO(unsigned);

extern void DSB(void);
extern void DMB(void);
extern void ISB(void);

#endif