// can specify include dir with -I for compiler?
#include "../libpi-WIP/rpi.h"
#include "../libpi-WIP/gpio.h"
#include "../libpi-WIP/timer.h"


int notmain(){
    gpio_set_output(GPIO_PIN15);
    gpio_set_output(GPIO_PIN14);
    /*int i = 0;
    while(i < 10){
        gpio_write(GPIO_PIN16, 1);
        delay_ms(500);
        gpio_write(GPIO_PIN16, 0);
        delay_ms(500);
        i++;
    }*/
    gpio_write(GPIO_PIN14, 1);
    gpio_write(GPIO_PIN15, 1);
    while(1){};
    reboot();
    return 0;
}

