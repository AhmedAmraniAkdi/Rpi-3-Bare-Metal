#include "libpi/mini_uart.h"
#include "libpi/rpi.h"

// gets arm and core frequencies
// for rpi 3b+ i get 600MHz for arm and 250MHz for core
// returns 128mb for arm and core mems?? Fixed -> needs fixup.dat file!

// add turbo later
void notmain(){
    uart_init();
    
    double a = 1.0,b = 1.5, c = 0.00000001;

    printk("%f %f %f %f %f\n", a, b, c, a + b, b * c);

    while(1) {};
}