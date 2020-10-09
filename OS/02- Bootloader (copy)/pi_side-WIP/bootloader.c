#include "rpi.h"
#include "mini_uart.h"
#include "timer.h"
#include "gpio.h"

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"   


static void put_byte(unsigned char uc)  { 
    uart_putc(uc); 
}
static unsigned char get_byte(void) { 
    return uart_getc(); 
}

static unsigned get_uint(void) {
	unsigned u = get_byte();
        u |= get_byte() << 8;
        u |= get_byte() << 16;
        u |= get_byte() << 24;
	return u;
}
static void put_uint(unsigned u) {
    put_byte((u >> 0)  & 0xff);
    put_byte((u >> 8)  & 0xff);
    put_byte((u >> 16) & 0xff);
    put_byte((u >> 24) & 0xff);
}


static void die(int code) {
    put_uint(code);
    reboot();
}


void putk(const char *msg) {
    put_uint(PRINT_STRING);
    int n;
    for(n = 0; msg[n]; n++)
        ;
    put_uint(n);
    for(n = 0; msg[n]; n++)
        put_byte(msg[n]);
}


void notmain(void) {

    uart_init(115200);

    //gpio_set_output(GPIO_PIN16);
    while(1)
    {   
        while(1){
        if(GET32(AUX_MU_LSR_REG) & 0x20){
            break;
        }
    }
        PUT32(AUX_MU_IO_REG, 49);
    }/*
    while(get_byte() != 111){
        put_byte(159);
    }*/
    /*put_uint(PRINT_STRING);
    putk("testing");
    if(get_byte() != 123){
        reboot();
    }
    if(get_byte() != 124){
        reboot();
    }
    gpio_set_on(GPIO_PIN16);
    while(1){};*/

    // should not get back here, but just in case.
    reboot();
}