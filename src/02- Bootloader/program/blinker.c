// can specify include dir with -I for compiler?
#include "../libpi/rpi.h"
#include "../libpi/gpio.h"
#include "../libpi/timer.h"


int notmain(){
    gpio_set_output(GPIO_PIN16);
    int i = 0;
    while(i < 10){
        gpio_write(GPIO_PIN16, 1);
        delay_ms(500);
        gpio_write(GPIO_PIN16, 0);
        delay_ms(500);
        i++;
    }
    reboot();
    return 0;
}

