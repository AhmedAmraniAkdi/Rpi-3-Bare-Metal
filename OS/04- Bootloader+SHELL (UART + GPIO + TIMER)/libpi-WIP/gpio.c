#include "gpio.h"
#include "rpi.h"

void gpio_init(void){} // nothing for now


// each gpio has a distinct set of function, the user needs
// to use the correct ones
// return 1 if sucess, -1 if error

int gpio_set_function(gpio_pin pin, gpio_func_t function){

    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    if(function < 0 || function > 7){
        return GPIO_INVALID_REQUEST;
    }

    unsigned int value;
    gpio_func_select r = GPFSEL0 + 4 * pin/10;
    value =GET32(r);            
    value &=~(7<<((pin%10)*3));
    value |=function<<((pin%10)*3);
    PUT32(r,value);             

    return 1;
}

int gpio_set_input(gpio_pin pin){
    return gpio_set_function(pin, GPIO_FUNC_INPUT);
}

int gpio_set_output(gpio_pin pin){
    return gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

int gpio_read(gpio_pin pin){

    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    gpio_level r = GPLEV0 + 4 * pin/32;
    unsigned int value = GET32(r);

    return (value>>(pin%32)) & 1;
}

int gpio_write(gpio_pin pin, unsigned val){

    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    unsigned int value;
    gpio_func_select r = GPFSEL0 + 4 * pin/10;
    value =GET32(r);            
    value =(value>>((pin%10)*3)) & 7; // 3 bits for each pin
    if(value != GPIO_FUNC_OUTPUT){
        return GPIO_INVALID_REQUEST;
    }

    if(val == 1){
        return gpio_set_on(pin);
    } else{
        return gpio_set_off(pin);
    }
}

int gpio_set_on(gpio_pin pin){

    unsigned int value;
    gpio_set_clear r = GPSET0 + 4 * pin/32;
    value =GET32(r);
    value |=1<<(pin%32);
    PUT32(r,value); 

    return 1;
}

int gpio_set_off(gpio_pin pin){

    unsigned int value;
    gpio_set_clear r = GPCLEAR0 + 4 * pin/32;
    value =GET32(r);
    value |=1<<(pin%32);
    PUT32(r,value); 

    return 1;
}

int gpio_set_pullupdownoff(gpio_pin pin, unsigned resistor){
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    PUT32(GPPUD, resistor);
    CYCLE_DELAY(175); // it asks for 150, 175 to be sure
    gpio_pullupdown r = GPPUDCLK0 + 4 * pin/32;
    PUT32(r, 1<<(pin%32));
    CYCLE_DELAY(175);
    PUT32(GPPUD, 0);
    PUT32(r, 0);

    return 1;
}

int gpio_set_pullup(gpio_pin pin){
    return gpio_set_pullupdownoff(pin, 2);
}

int gpio_set_pulldown(gpio_pin pin){
    return gpio_set_pullupdownoff(pin, 1);
}

int gpio_pud_off(gpio_pin pin){
    return gpio_set_pullupdownoff(pin, 0);
}

