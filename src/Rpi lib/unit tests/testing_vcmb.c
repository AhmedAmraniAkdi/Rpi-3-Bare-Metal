#include "libpi/mini_uart.h"
#include "libpi/rpi.h"
#include "libpi/VCmailbox.h"

// gets arm and core frequencies
// for rpi 3b+ i get 600MHz for arm and 250MHz for core
// maybe change it to 1400 later?
void notmain(){
    uart_init();
    
    mbox[0] = 13*4;            
    mbox[1] = MBOX_REQUEST;         
    
    mbox[2] = MBOX_TAG_GETCLOCK; 
    mbox[3] = 8;                    
    mbox[4] = 0;
    mbox[5] = 0x000000003;   // ARM        
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_GETCLOCK; 
    mbox[8] = 8;                    
    mbox[9] = 0;
    mbox[10] = 0x000000004;   // CORE      
    mbox[11] = 0;

    mbox[12] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP)) {
        uint32_t arm = mbox[6];
        uint32_t core = mbox[11];
        printk("ARM freq = %d -- Core freq = %d \n", arm, core);
    } else {
        printk("Unable to query serial!\n");
    }

    while(1) {};
}