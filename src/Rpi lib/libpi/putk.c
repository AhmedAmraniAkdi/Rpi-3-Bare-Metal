#include "rpi.h"

int internal_putk(const char *p) {
    for(; *p; p++)
        rpi_putchar(*p);
//  we were adding this.
//  rpi_putchar('\n');
    return 1;
}

// redundant?
int (*putk)(const char *p) = internal_putk;