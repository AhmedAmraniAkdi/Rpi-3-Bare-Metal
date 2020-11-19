#include "libpi/mini_uart.h"
#include "libpi/rpi.h"
#include "libpi/VCmailbox.h"

// gets arm and core frequencies
// for rpi 3b+ i get 600MHz for arm and 250MHz for core
// returns 128mb for arm and core mems??

// add turbo later
void notmain(){
    uart_init();
    
    mbox[0] = 23*4;            
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

    mbox[12] = MBOX_TAG_GETVCMEM;
    mbox[13] = 8;
    mbox[14] = 0;
    mbox[15] = 0;
    mbox[16] = 0;

    mbox[17] = MBOX_TAG_GETARMMEM;
    mbox[18] = 8;
    mbox[19] = 0;
    mbox[20] = 0;
    mbox[21] = 0;

    mbox[22] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP)) {

        uint32_t arm = mbox[6];
        uint32_t core = mbox[11];
        uint32_t core_base_mem = mbox[15];
        uint32_t core_tot_mem = mbox[16];
        uint32_t arm_base_mem = mbox[20];
        uint32_t arm_tot_mem = mbox[21];

        printk("ARM freq = %d -- Core freq = %d \n", arm, core);
        printk("core mem starts at 0x%x and it's of size 0x%x\n", core_base_mem, core_tot_mem);
        printk("arm mem starts at 0x%x and it's of size 0x%x\n", arm_base_mem, arm_tot_mem);
    } else {
        printk("Unable to query serial!\n");
    }

    while(1) {};
}