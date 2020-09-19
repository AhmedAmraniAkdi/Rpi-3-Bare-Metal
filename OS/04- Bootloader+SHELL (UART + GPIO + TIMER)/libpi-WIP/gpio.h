#ifndef GPIO_H
#define GPIO_H

/*
 * Functions for controlling Raspberry Pi GPIO.
 * Ahmed Amrani Akdi
 */


//symbolic names for the GPIO pins
typedef enum {
    GPIO_PIN0 = 0,
    GPIO_PIN1 = 1,
    GPIO_PIN2 = 2,
    GPIO_PIN3 = 3,
    GPIO_PIN4 = 4,
    GPIO_PIN5 = 5,
    GPIO_PIN6 = 6,
    GPIO_PIN7 = 7,
    GPIO_PIN8 = 8,
    GPIO_PIN9 = 9,
    GPIO_PIN10 = 10,
    GPIO_PIN11 = 11,
    GPIO_PIN12 = 12,
    GPIO_PIN13 = 13,
    GPIO_PIN14 = 14,
    GPIO_PIN15 = 15,
    GPIO_PIN16 = 16,
    GPIO_PIN17 = 17,
    GPIO_PIN18 = 18,
    GPIO_PIN19 = 19,
    GPIO_PIN20 = 20,
    GPIO_PIN21 = 21,
    GPIO_PIN22 = 22,
    GPIO_PIN23 = 23,
    GPIO_PIN24 = 24,
    GPIO_PIN25 = 25,
    GPIO_PIN26 = 26,
    GPIO_PIN27 = 27,
    GPIO_PIN28 = 28,
    GPIO_PIN29 = 29,
    GPIO_PIN30 = 30,
    GPIO_PIN31 = 31,
    GPIO_PIN32 = 32,
    GPIO_PIN33 = 33,
    GPIO_PIN34 = 34,
    GPIO_PIN35 = 35,
    GPIO_PIN36 = 36,
    GPIO_PIN37 = 37,
    GPIO_PIN38 = 38,
    GPIO_PIN39 = 39,
    GPIO_PIN40 = 40,
    GPIO_PIN41 = 41,
    GPIO_PIN42 = 42,
    GPIO_PIN43 = 43,
    GPIO_PIN44 = 44,
    GPIO_PIN45 = 45,
    GPIO_PIN46 = 46,
    GPIO_PIN47 = 47,
    GPIO_PIN48 = 48,
    GPIO_PIN49 = 49,
    GPIO_PIN50 = 50,
    GPIO_PIN51 = 51,
    GPIO_PIN52 = 52,
    GPIO_PIN53 = 53,
} gpio_pin;

// GPIO pin mappings for UART
#define GPIO_TX GPIO_PIN14
#define GPIO_RX GPIO_PIN15

 //These enumerated values establish symbolic names for each of the
 //available GPIO pin functions.
typedef enum {
    GPIO_FUNC_INPUT   = 0,
    GPIO_FUNC_OUTPUT  = 1,
    GPIO_FUNC_ALT0    = 4,
    GPIO_FUNC_ALT1    = 5,
    GPIO_FUNC_ALT2    = 6,
    GPIO_FUNC_ALT3    = 7,
    GPIO_FUNC_ALT4    = 3,
    GPIO_FUNC_ALT5    = 2,
} gpio_func_t;

#define GPIO_INVALID_REQUEST  -1  // return value for invalid request


//Base adresses of registers

// Function select registers
typedef enum {
    GPFSEL0 = 0x3F200000,
    GPFSEL1 = 0x3F200004,
    GPFSEL2 = 0x3F200008,
    GPFSEL3 = 0x3F20000C,
    GPFSEL4 = 0x3F200010,
    GPFSEL5 = 0x3F200014,
} gpio_func_select;

// Set/clear registers
typedef enum {
    GPSET0   = 0x3F20001C,
    GPSET1   = 0x3F200020,
    GPCLEAR0 = 0x3F200028,
    GPCLEAR1 = 0x3F20002C,
} gpio_set_clear;

// read registers
typedef enum {
    GPLEV0 = 0x3F200034,
    GPLEV1 = 0x3F200038,
} gpio_level;

// pull up/down registers
typedef enum {
    GPPUD     = 0x3F200094,
    GPPUDCLK0 = 0x3F200098,
    GPPUDCLK1 = 0x3F20009C,
} gpio_pullupdown;

// events...
/*
*
*
*
*
*
*/

//Initialize the GPIO code module. Does nothing for now
void gpio_init(void);

// set GPIO function for <pin> (input, output, alt...).
// settings for other pins should be unchanged.
int gpio_set_function(gpio_pin pin, gpio_func_t function);

// set <pin> to input or output.
int gpio_set_input(gpio_pin pin);
int gpio_set_output(gpio_pin pin);

// write <pin> = <val>.  1 = high, 0 = low.
// <pin> must be in output mode, other pins will be unchanged.
int gpio_write(gpio_pin pin, unsigned val);

// read <pin>: 1 = high, 0 = low.
int gpio_read(gpio_pin pin);

// turn <pin> on.
int gpio_set_on(gpio_pin pin);

// turn <pin> off.
int gpio_set_off(gpio_pin pin);

// the 3 below functions use this one internally
int gpio_set_pullupdownoff(gpio_pin pin, unsigned resistor);

// set <pin> as a pullup
int gpio_set_pullup(gpio_pin pin);

// set <pin> as a pulldown.
int gpio_set_pulldown(gpio_pin pin);

// set <pin> back to the default state: no pull up, no pulldown.
int gpio_pud_off(gpio_pin pin);


//int gpio_get_pud(unsigned pin); we can't get pud status of gpio

/*****************************************************************
 * use the following to configure interrupts on pins.
 */


// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the 
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
enum { GPIO_INT0 = 49, GPIO_INT1, GPIO_INT2, GPIO_INT3 };
int is_gpio_int(unsigned gpio_int);

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin);

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin);

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin);

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin);



#endif