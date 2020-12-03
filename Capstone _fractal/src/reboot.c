#include "rpi.h"
#include "interrupt.h"

void delay(unsigned ticks) {
        CYCLE_DELAY(ticks);
}

unsigned timer_get_time(void) {
	return GET32(0x3F003004);
}

// dwelch says it runs ~1MHz
void delay_us(unsigned us) {
    unsigned rb = timer_get_time();
    while (1) {
        unsigned ra = timer_get_time();
        if ((ra - rb) >= us) {
            break;
        }
    }
}

void delay_ms(unsigned ms) {
	delay_us(ms*1000);
}

void delay_core_timer(unsigned divisor){
    uint64_t t = READ_TIMER();
    uint64_t f = READ_TIMER_FREQ();
    while(1){
        if((READ_TIMER() - t) > (f/divisor)) break;
    }
}

// not finding much documentation
// all i found is references to "power management, reset controller and watchdog" on bcm2835, but no documentation
// changed base address of io from 0x20000000 to 0x3F000000
// https://github.com/ultibohub/Core/blob/master/source/rtl/ultibo/core/bcm2837.pas useful
void reboot(void) {
        const int PM_RSTC = 0x3F10001c;
        const int PM_WDOG = 0x3F100024;
        const int PM_PASSWORD = 0x5a000000;
        const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

	int i;
        for(i = 0; i < 100000; i++)
                DUMMY();

        PUT32(PM_WDOG, PM_PASSWORD | 1);
        PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	while(1); 
}


// print out a special message so bootloader exits
void clean_reboot(void) {
    putk("DONE!!!\n");
    delay_ms(100);       // (hopefully) enough time for message to get flushed.
    reboot();
}


