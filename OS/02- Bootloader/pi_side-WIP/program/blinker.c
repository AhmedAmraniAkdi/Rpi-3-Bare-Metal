// can specify include dir with -I for compiler?
#include "../libpi-WIP/rpi.h"
#include "../libpi-WIP/gpio.h"
#include "../libpi-WIP/timer.h"


int notmain(){
    gpio_set_output(GPIO_PIN16);
    int i = 0;
    while(i < 20){
        gpio_write(GPIO_PIN16, 1);
        delay_ms(500);
        gpio_write(GPIO_PIN16, 0);
        delay_ms(500);
        i++;
    }
    reboot();
    return 0;
}

