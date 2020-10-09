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

    while(1)
    {   
        unsigned s = timer_get_time();
        while((timer_get_time() - s) < 100*1000)
        {
            put_uint(GET_PROG_INFO);
        }
        if(get_uint() == PUT_PROG_INFO){
            break;
        }
    }
    // [PUT_PROG_INFO, addr, nbytes, cksum] 
    unsigned addr = get_uint();
    unsigned n = get_uint();
    unsigned cksum = get_uint();

    put_uint(PRINT_STRING);
    putk("Pi says: You sent:");
    put_uint(addr);
    put_uint(n);
    put_uint(cksum);

    put_uint(GET_CODE);
    if(get_uint() != PUT_CODE){
        die(BOOT_ERROR);
    }

    put_uint(PRINT_STRING);
    putk("Pi says: starting to receive the code");

    for(int i = 0; i < n; i++){
        PUT8(ARMBASE + i, get_byte());
    }

    // 6. verify the cksum of the copied code.
    if(cksum != crc32((char*)ARMBASE, n)){
        die(BAD_CODE_CKSUM);
    }

    put_uint(BOOT_SUCCESS);
    delay_ms(500);

    // run what we got.
    BRANCHTO(ARMBASE);

    // should not get back here, but just in case.
    reboot();
}