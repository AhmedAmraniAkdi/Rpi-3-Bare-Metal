#include "gpio.h"
#include "rpi.h"

int gpio_init(void){} // nothing for now


// each gpio has a distinct set of function, the user needs
// to use the correct ones
// return 1 if sucess, -1 if error

/*
*
* helper function
*
*/

// many times we need to set a bit in a register and that's it
// works for the cases where: 53 pins, 2x32bit registers are needed for X thing
// we just need the address of the first register and the value to put ( 1 or 0)

void put_in_register(unsigned r, unsigned pin, unsigned val){
    unsigned value;
    unsigned r_ = r + 4 * pin/32;
    value = GET32(r_);
    PUT32(r_, value | (val << (pin%32)));  
}

/*
*
* Functions definitions
*
*/


int gpio_set_function(gpio_pin pin, gpio_func_t function){

    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    if(function < 0 || function > 7){
        return GPIO_INVALID_REQUEST;
    }

    unsigned value;
    gpio_func_select r = GPFSEL0 + 4 * pin/10;
    value = GET32(r);            
    value &= ~(7<<((pin%10)*3));
    value |= function<<((pin%10)*3);
    PUT32(r, value);             

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

    unsigned value;
    gpio_func_select r = GPFSEL0 + 4 * pin/10;
    value = GET32(r);            
    value = (value>>((pin%10)*3)) & 7; // 3 bits for each pin
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
    put_in_register(GPSET0, pin, 1);
    return 1;
}

int gpio_set_off(gpio_pin pin){
    put_in_register(GPCLEAR0, pin, 1);
    return 1;
}


/* 
*
* Pull up/down
*
*/


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

/* 
*
* Events
*
*/

int gpio_event_rising_edge_sync(gpio_pin pin, unsigned val){

    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPREN0, pin, val);  

    return 1;

}

int gpio_event_falling_edge_sync(gpio_pin pin, unsigned val){
        
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPFEN0, pin, val);   

    return 1;
}

int gpio_event_rising_edge_async(gpio_pin pin, unsigned val){
        
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPAREN0, pin, val);  
    return 1;
}

int gpio_event_falling_edge_async(gpio_pin pin, unsigned val){
    
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPAFEN0, pin, val);    

    return 1;
}

int gpio_event_highlevel(gpio_pin pin, unsigned val){
    
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPHEN0, pin, val);   

    return 1;
}

// set to detect low level
int gpio_event_lowlevel(gpio_pin pin, unsigned val){
    
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPREN0, pin, val);    

    return 1;
}


int gpio_event_detected(gpio_pin pin){
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    gpio_events r = GPEDS0 + 4 * pin/32;
    unsigned value = GET32(r);

    return (value>>(pin%32)) & 1;   
}


int gpio_event_clear(gpio_pin pin){
    if(pin < 0 || pin > 53){
        return GPIO_INVALID_REQUEST;
    }

    put_in_register(GPEDS0, pin, 1);    

    return 1;
}


