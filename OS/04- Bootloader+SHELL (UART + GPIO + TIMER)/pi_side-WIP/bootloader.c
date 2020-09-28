/* engler, cs140e.
 *
 * very simple bootloader.  more robust than xmodem, which seems to 
 * have bugs in terms of recovery with inopportune timeouts.
 */

#include "rpi.h"
#include "mini_uart.h"
#include "timer.h"

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"         // holds crc32

// blocking calls to send / receive a single byte from the uart.
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

// send the unix side an error code and reboot.
static void die(int code) {
    put_uint(code);
    reboot();
}

// We do not have interrupts and the UART can only hold 
// 8 bytes before it starts dropping them.   so if you print at the
// same time the UNIX end can send you data, you will likely
// lose some, leading to weird bugs.  If you print, safest is 
// to do right after you have completely received a message.
void putk(const char *msg) {
    put_uint(PRINT_STRING);
    int n;
    for(n = 0; msg[n]; n++)
        ;
    put_uint(n);
    for(n = 0; msg[n]; n++)
        put_byte(msg[n]);
}

// send a <GET_PROG_INFO> message every 300ms.
// 
// NOTE: look at the idiom for comparing the current usec count to 
// when we started.  
void wait_for_data(void) {
    while(1) {
        put_uint(GET_PROG_INFO);

        unsigned s = timer_get_usec();
        // the funny subtraction is to prevent wrapping.
        while((timer_get_usec() - s) < 300*1000) {
            // the UART says there is data: start eating it!
            if(uart_can_getc())
                return;
        }
    }
}

void wait(void){
    unsigned s = timer_get_usec();
    while((timer_get_usec() - s) < 300*1000) {
            // the UART says there is data: start eating it!
            if(uart_can_getc())
                return;
        }
        die(BOOT_ERROR);
}

void notmain(void) {
    uart_init(115200);

    // 1. keep sending GET_PROG_INFO until there is data.
    wait_for_data();

    // 2. expect: [PUT_PROG_INFO, addr, nbytes, cksum] 
    //    we echo cksum back in step 4 to help debugging.
    if(get_uint() != PUT_PROG_INFO){
        die(NOT_PUT_PROG_INFO);
    }

    unsigned addr = get_uint();
    unsigned n = get_uint();
    unsigned cksum = get_uint();

    // 3. If the binary will collide with us, abort. 
    //    you can assume that code must be below where the booloader code
    //    gap starts.
    if(addr < ARMBASE){
        die(BAD_CODE_ADDR);
    }

    // 4. send [GET_CODE, cksum] back.
    put_uint(GET_CODE);
    put_uint(cksum);


    // 5. expect: [PUT_CODE, <code>]
    //  read each sent byte and write it starting at 
    //  ARMBASE using PUT8

    if(get_uint() != PUT_CODE){
        die(NOT_PUT_CODE);
    }
    char *buf = (char*)calloc(n, sizeof(char));
    for(int i = 0; i < n; i++){
        buf[i] = get_byte();
    }

    // 6. verify the cksum of the copied code.
    if(cksum != crc32(buf, n)){
        die(BAD_CODE_CKSUM);
    }

    for(int i = 0; i < n; i++){
        PUT8(ARMBASE + i, buf[i]);
    }

    // 7. no previous failures: send back a BOOT_SUCCESS!
    put_uint(BOOT_SUCCESS);

    delay_ms(500);

    putk(buf); // send the program?

    // run what we got.
    BRANCHTO(ARMBASE);

    // should not get back here, but just in case.
    reboot();
}