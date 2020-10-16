//https://github.com/dwelch67/raspberrypi/blob/master/bootloader02/bootloader02.c

#include "rpi.h"
#include "mini_uart.h"
#include "timer.h"
#include "gpio.h"

#define ARMBASE 0x80000

void notmain(void) {

    unsigned char xstring[256];
    unsigned int ra;
    unsigned int addr;
    unsigned int block;
    unsigned int crc;

    uart_init(115200);

    //SOH 0x01
    //ACK 0x06
    //NAK 0x15
    //EOT 0x04

    //block numbers start with 1

    //132 byte packet
    //starts with SOH
    //block number byte
    //255-block number
    //128 bytes of data
    //checksum byte (whole packet)
    //a single EOT instead of SOH when done, send an ACK on it too

    PUT32(AUX_MU_IIR_REG, 6);
    while(1)
    {   
        delay_ms(4000);
        uart_putc(0x15);
        if(GET32(AUX_MU_LSR_REG)&0x01) 
            break;
    }

    block=1;
    addr=ARMBASE;

    while(1)
    {
        xstring[0]=uart_getc();

        if(xstring[0]==0x04){
            uart_putc(0x06);
            break;
        }

        if(xstring[0]!=0x01) 
            break;

        crc=0x01;

        for(ra=1;ra<132;ra++){
            xstring[ra]=uart_getc();
            crc+=xstring[ra];
        }

        if(xstring[2]!=(255-xstring[1])) 
            break;

        crc-=xstring[131];
        crc&=0xFF;

        if(xstring[131]!=crc){
            uart_putc(0x15);
        }

        for(ra=0;ra<128;ra++){
            PUT8(addr++,xstring[ra+3]);
        }

        if(addr > (0x200000 + ARMBASE)){
            uart_putc(0x15);
            break;
        }

        uart_putc(0x06);
        block=(block+1)&0xFF;
    }

    if(xstring[0]==0x04){
        BRANCHTO(ARMBASE);
    } else{
        reboot(); // this for now
    }

    // should not reach this
    reboot();

}