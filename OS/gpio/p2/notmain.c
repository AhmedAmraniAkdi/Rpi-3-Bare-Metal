
extern void PUT32 ( unsigned int, unsigned int );
extern void PUT16 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ();

typedef unsigned gpio_pin;

#define GPIO_INVALID_REQUEST -1

typedef enum {
    GPIO_FUNC_INPUT   = 0,
    GPIO_FUNC_OUTPUT  = 1,
} gpio_func_t;

typedef enum {
    GPFSEL0 = 0x3F200000,
    GPFSEL1 = 0x3F200004,
} gpio_func_select;

// Set/clear registers
typedef enum {
    GPSET0   = 0x3F20001C,
    GPCLEAR0 = 0x3F200028,
} gpio_set_clear;


void delay_ms(unsigned ms) {
	    unsigned rb = GET32(0x3F003004);
    while (1) {
        unsigned ra = GET32(0x3F003004);;
        if ((ra - rb) >= (1000*ms)) {
            break;
        }
    }
}

void put_in_register(unsigned r, unsigned pin, unsigned val){
    unsigned value;
    unsigned r_ = r + 4 * (pin/32);
    value = GET32(r_);
    PUT32(r_, value | (val << (pin%32)));  
}

int gpio_set_function(gpio_pin pin, gpio_func_t function){

    unsigned value;
    gpio_func_select r = GPFSEL0 + 4 * (pin/10);
    value = GET32(r);            
    value &= ~(7<<((pin%10)*3));
    value |= function<<((pin%10)*3);
    PUT32(r, value);             

    return 1;
}


int gpio_set_output(gpio_pin pin){
    return gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

int gpio_set_on(gpio_pin pin){
    put_in_register(GPSET0, pin, 1);
    return 1;
}

int gpio_set_off(gpio_pin pin){
    put_in_register(GPCLEAR0, pin, 1);
    return 1;
}

int notmain(){

    unsigned val = GET32(GPFSEL1);
    val &= ~(7<<18);
    PUT32(GPFSEL1, val | 1<<18);
    while(1){
        //PUT32(GPSET0, 1<<16);
        gpio_set_on(16);
        delay_ms(1000);
        gpio_set_off(16);
        //PUT32(GPCLEAR0, 1<<16);
        delay_ms(1000);
    }
    return 0;
}

