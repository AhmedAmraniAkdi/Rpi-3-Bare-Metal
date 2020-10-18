// can specify include dir with -I for compiler?
#include "../libpi/rpi.h"
#include "../libpi/gpio.h"
#include "../libpi/timer.h"


int notmain(){
    gpio_set_output(GPIO_PIN16);
    /*unsigned int ra = GET32(GPFSEL1);
    ra&=~(7<<18);
    ra|=1<<18;
    PUT32(GPFSEL1,ra);*/
    //int i = 0;
    while(1){
        gpio_write(GPIO_PIN16, 1);
        //PUT32(GPSET0,1<<16);
        delay_ms(500);
        gpio_write(GPIO_PIN16, 0);
        //PUT32(GPCLEAR0,1<<16);
        delay_ms(500);
        //i++;
    }
    //PUT32(GPSET0,1<<16);
    //while(1){};
    reboot();
    return 0;
}

