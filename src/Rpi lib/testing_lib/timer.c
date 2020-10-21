#include "rpi.h"
#include "timer.h"

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