#include "rpi.h"
#include "timer.h"

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