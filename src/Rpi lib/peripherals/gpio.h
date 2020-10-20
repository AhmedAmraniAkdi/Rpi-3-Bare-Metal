#ifndef GPIO_H
#define GPIO_H

/*
*
* Functions for controlling Raspberry Pi GPIO.
* Ahmed Amrani Akdi
*
*/


/* 
*
* Pins
*
*/


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


/* 
*
* Pin Functions
*
*/


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


/* 
*
* Registers
*
*/

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

// events
// interrupts will be on interrupts.h
typedef enum{
    GPEDS0  = 0x3F200040,
    GPEDS1  = 0x3F200044,
    GPREN0  = 0x3F20004C,
    GPREN1  = 0x3F200050,
    GPFEN0  = 0x3F200058,
    GPFEN1  = 0x3F20005C,
    GPHEN0  = 0x3F200064,
    GPHEN1  = 0x3F200068,
    GPLEN0  = 0x3F200070, 
    GPLEN1  = 0x3F200074,
    GPAREN0 = 0x3F20007C,
    GPAREN1 = 0x3F200080,
    GPAFEN0 = 0x3F200088,
    GPAFEN1 = 0x3F20008C,
} gpio_events;


/* 
*
* Functions
*
*/


//Initialize the GPIO code module. Does nothing for now
int gpio_init(void);

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


/* 
*
* Pull up/down
*
*/


// the 3 below functions use this one internally
int gpio_set_pullupdownoff(gpio_pin pin, unsigned resistor);

// set <pin> as a pullup
int gpio_set_pullup(gpio_pin pin);

// set <pin> as a pulldown.
int gpio_set_pulldown(gpio_pin pin);

// set <pin> back to the default state: no pull up, no pulldown.
int gpio_pud_off(gpio_pin pin);

/* 
*
* Events
*
*/

// if val == 1, we set, if val == 0 we clear the event enable registers
//set to detect rising edge (0->1) on <pin>. 011
int gpio_event_rising_edge_sync(gpio_pin pin, unsigned val);

// set or clear falling edge (1->0). 100
int gpio_event_falling_edge_sync(gpio_pin pin, unsigned val);

//set or clear rising edge (0->1) on <pin>. asynchronous!
int gpio_event_rising_edge_async(gpio_pin pin, unsigned val);

// set or clear falling edge (1->0). asynchronous
int gpio_event_falling_edge_async(gpio_pin pin, unsigned val);

// set or clear to detect high level
int gpio_event_highlevel(gpio_pin pin, unsigned val);

// set to detect low level
int gpio_event_lowlevel(gpio_pin pin, unsigned val);

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an event.
// 1 if sucess, 0 if no event, -1 if error
int gpio_event_detected(gpio_pin pin);

// have to write a 1 to the pin in register to clear the event.
int gpio_event_clear(gpio_pin pin);


#endif