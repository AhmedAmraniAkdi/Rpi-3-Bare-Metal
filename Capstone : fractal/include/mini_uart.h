#ifndef MINI_UART_H
#define MINI_UART_H

/*
*
* Functions for using Raspberry Pi mini uart
* Ahmed Amrani Akdi
*
*/

/*
*
* Registers of the mini uart
*
*/

typedef enum{
    AUX_ENABLES     = 0x3F215004,
    AUX_MU_IO_REG   = 0x3F215040,
    AUX_MU_IER_REG  = 0x3F215044,
    AUX_MU_IIR_REG  = 0x3F215048,
    AUX_MU_LCR_REG  = 0x3F21504C,
    AUX_MU_MCR_REG  = 0x3F215050,
    AUX_MU_LSR_REG  = 0x3F215054,
    AUX_MU_MSR_REG  = 0x3F215058,
    AUX_MU_SCRATCH  = 0x3F21505C,
    AUX_MU_CNTL_REG = 0x3F215060,
    AUX_MU_STAT_REG = 0x3F215064,
    AUX_MU_BAUD_REG = 0x3F215068,
} mu_registers;

typedef enum {
    GPIO_TX = 14,
    GPIO_RX = 15,
} uart_gpios;

#define INVALID_REQUEST  -1 

/*
*
* Uart functions
*
*/

// no interrupts still

// initilize the mini uart
void uart_init(void);

// 1 = at least one byte on rx queue, 0 otherwise
int uart_can_getc(void);

// returns one byte from the rx queue,
int uart_getc(void);

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void);

// put one byte/string on the tx queue
void uart_putc(unsigned c);
void uart_puts(char* s);

// returns if uart is enabled
int is_enabled_uart(void);

#endif