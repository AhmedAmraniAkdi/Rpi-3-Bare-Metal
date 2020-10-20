// https://github.com/dwelch67/raspberrypi/blob/master/bootloader02/bootloader02.c
// https://cs140e.sergio.bz/assignments/1-shell/
// https://github.com/dwelch67/raspberrypi/blob/master/bootloader06/bootloader06.c

// dunno why my version doesn't work

#include "rpi.h"
#include "mini_uart.h"
#include "timer.h"

#define ARMBASE 0x80000
#define SOH 0x01
#define ACK 0x06
#define NAK 0x15
#define EOT 0x04
#define CAN 0x18

/*void notmain(void) {

    unsigned char xstring[256];
    unsigned int ra;
    unsigned int rx;
    unsigned int addr;
    unsigned int block;
    unsigned int crc;
    int error = 0;

    uart_init();

    //send NAK every 4s
    rx = timer_get_time();
    while(1)
    {
        ra = timer_get_time();
        if((ra-rx) >= 4000000)
        {
            uart_putc(NAK);
            rx += 4000000;
        }
        if(uart_can_getc()) 
            break;
    }

    block = 1;
    addr = ARMBASE;

    while(1)
    {
        xstring[0] = uart_getc();

        if(xstring[0] == EOT){
            uart_putc(ACK);
            break;
        }

        if(xstring[0] != SOH){
            uart_putc(NAK);
            continue;
        }

        crc = SOH;

        for(ra=1; ra<132; ra++){
            xstring[ra] = uart_getc();
            crc += xstring[ra];
        }

        if(xstring[2] != block){
            uart_putc(NAK);
            continue;
        }

        if(xstring[2] != (255 - xstring[1])) {
            uart_putc(NAK); 
            continue;
        }

        crc -= xstring[131];
        crc &= 0xFF;

        if(xstring[131] != crc){
            uart_putc(NAK);
            continue;
        }

        if(addr > (0x199500 +  ARMBASE)){
            error = 1;
            uart_putc(CAN);
            break;
        }

        for(ra=0;ra<128;ra++){
            PUT8(addr++,xstring[ra+3]);
        }

        uart_putc(ACK);
        block = (block + 1) & 0xFF;
    } 

    if(error){
        notmain();
    } else {
        BRANCHTO(ARMBASE);
    }

    // should not reach this
    reboot();

}*/

#define NAKSOH_TIMEOUT 4000000
#define WAIT_AFTER_EOT_TIMEOUT 2000000
#define WAIT_AFTER_CAN_TIMEOUT 2000000

int notmain ( void )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int rx;
    unsigned int block;
    unsigned int state;
    unsigned int csum;
    unsigned char *xstring;

    uart_init();


    while(1)
    {
        block=1;
        state=192;
        csum=0;
        xstring=(unsigned char *)ARMBASE;

        uart_putc(NAK);
        rx=timer_get_time();
        while(1)
        {
            ra=timer_get_time();
            if((ra-rx)>=NAKSOH_TIMEOUT)
            {
                uart_putc(NAK);
                rx+=NAKSOH_TIMEOUT;
            }
            if(uart_can_getc()) break;
        }
        while(state!=255)
        {
            rb=uart_getc();
            switch(state)
            {
                //wait for SOH or EOT
                case 192:
                {
                    switch(rb)
                    {
                        case SOH:
                        {
                            csum=rb;
                            state++;
                            break;
                        }
                        case EOT:
                        {
                            uart_putc(ACK);
                            rx=timer_get_time();
                            while(1)
                            {
                                ra=timer_get_time();
                                if((ra-rx)>=WAIT_AFTER_EOT_TIMEOUT) break;
                            }
                            uart_putc(0x0D);
                            uart_putc(0x0A);
                            uart_putc(0x0A);
                            while(1)
                            {
                                uart_puts("Press g or G to start program:\n");
                                rb=uart_getc();
                                if((rb=='g')||(rb=='G'))
                                {
                                    uart_putc(0x0D);
                                    uart_putc('-');
                                    uart_putc('-');
                                    uart_putc(0x0D);
                                    uart_putc(0x0A);
                                    uart_putc(0x0A);
                                    BRANCHTO(ARMBASE);
                                }
                            }
                            break;
                        }
                        default:
                        {
                            state=255;
                            break;
                        }
                    }
                    break;
                }
                //uninverted block number
                case 193:
                {
                    if(rb==block)
                    {
                        csum+=rb;
                        state++;
                    }
                    else
                    {
                        state=255;
                    }
                    break;
                }
                //inverted block number
                case 194:
                {
                    if(rb==(0xFF-block))
                    {
                        csum+=rb;
                        state=0;
                    }
                    else
                    {
                        state=255;
                    }
                    break;
                }
                //checksum
                case 128:
                {
                    csum&=0xFF;
                    if(rb==csum)
                    {
                        uart_putc(ACK);
                        block=(block+1)&0xFF;
                        xstring+=128;
                        state=192;
                    }
                    else
                    {
                        state=255;
                    }
                    break;
                }
                default:
                {
                    csum+=rb;
                    xstring[state]=rb&0xFF;
                    state++;
                    break;
                }
            }
        }
        //just bail out.
        uart_putc(CAN);
        uart_putc(CAN);
        uart_putc(CAN);
        uart_putc(CAN);
        uart_putc(CAN);
        rx=timer_get_time();
        while(1)
        {
            ra=timer_get_time();
            if((ra-rx)>=WAIT_AFTER_CAN_TIMEOUT) break;
        }
    }

    reboot();

    return 0;
}
