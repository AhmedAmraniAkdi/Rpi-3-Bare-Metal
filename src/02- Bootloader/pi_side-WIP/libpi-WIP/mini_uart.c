#include "rpi.h"
#include "mini_uart.h"
#include "gpio.h"

int uart_can_getc(void){
    return GET32(AUX_MU_LSR_REG)&0x01;
}

int uart_can_putc(void){
    return GET32(AUX_MU_LSR_REG) & 0x20;
}

int uart_getc(void){
    while(1){
        if(GET32(AUX_MU_LSR_REG) & 0x01){
            break;
        }
    }
    return(GET32(AUX_MU_IO_REG) & 0xFF);
}

void uart_putc(unsigned c){
    while(1){
        if(GET32(AUX_MU_LSR_REG) & 0x20){
            break;
        }
    }
    PUT32(AUX_MU_IO_REG, c);
}

void uart_init(unsigned baudrate){

    // enable mini uart
    PUT32(AUX_ENABLES, 1);
    // p10 if aux enables is 0, no access to registers
    // i guess this is where i put the dsb
    DSB();

    // no interrupts, polling for now
    PUT32(AUX_MU_IER_REG, 0);

    // disable r/s while we configure mini uart
    PUT32(AUX_MU_CNTL_REG, 0);

    // data format 8 bits
    PUT32(AUX_MU_LCR_REG, 3);

    //PUT32(AUX_MU_MCR_REG,0);
    //PUT32(AUX_MU_IER_REG,0);
    // clear FIFOs
    //PUT32(AUX_MU_IIR_REG, 6);
    PUT32(AUX_MU_IIR_REG, 6);
    //250000000/baudrate/8 - 1 = divisor
    //unsigned divisor = 250000000/(8*(baudrate + 1));
    PUT32(AUX_MU_BAUD_REG, 270);

    // setting gpios to alternate functions and pull off
    gpio_set_function(GPIO_PIN14, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_PIN15, GPIO_FUNC_ALT5);
    gpio_set_pullupdownoff(GPIO_PIN14, 0);
    gpio_set_pullupdownoff(GPIO_PIN15, 0);
    /*unsigned ra;
    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    PUT32(GPFSEL1,ra);

    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) DUMMY();
    PUT32(GPPUDCLK0,(1<<14));
    for(ra=0;ra<150;ra++) DUMMY();
    PUT32(GPPUDCLK0,0);*/

    // enable r/s
    PUT32(AUX_MU_CNTL_REG, 3);
    DSB();
}