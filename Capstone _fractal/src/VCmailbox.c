// https://github.com/bztsrc/raspi3-tutorial/tree/master/04_mailboxes
// https://jsandler18.github.io/extra/mailbox.html
#include "VCmailbox.h"
#include "rpi.h"
#include <stdint.h>
#include "assert.h"
#include "mini_uart.h"
/* mailbox message buffer */
static int is_turbo = 0;
volatile uint32_t  __attribute__((aligned(16))) mbox[36];

enum {
    VIDEOCORE_MBOX  = 0x3F00B880,
    MBOX_READ       = VIDEOCORE_MBOX+0x0,
    MBOX_POLL       = VIDEOCORE_MBOX+0x10,
    MBOX_SENDER     = VIDEOCORE_MBOX+0x14,
    MBOX_STATUS     = VIDEOCORE_MBOX+0x18,
    MBOX_CONFIG     = VIDEOCORE_MBOX+0x1C,
    MBOX_WRITE      = VIDEOCORE_MBOX+0x20,
    MBOX_RESPONSE   = 0x80000000,
    MBOX_FULL       = 0x80000000,
    MBOX_EMPTY      = 0x40000000,
};

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(mb_channel_t ch){
    uint32_t r = (((uint32_t)((uintptr_t)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    while(1){
        if(!(GET32(MBOX_STATUS) & MBOX_FULL)) break;
    }
    /* write the address of our message to the mailbox with channel identifier */
    PUT32(MBOX_WRITE, r);
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        while(1){
            if(!(GET32(MBOX_STATUS) & MBOX_EMPTY)) break;
        }
        /* is it a response to our message? */
        if(r == GET32(MBOX_READ))
            /* is it a valid successful response? */
            return mbox[1] == MBOX_RESPONSE;
    }
    return 0;
}

uint32_t vc_mem_start_address(void){
    uart_init();
    
    mbox[0] = 8*4;            
    mbox[1] = MBOX_REQUEST;         

    mbox[2] = MBOX_TAG_GETVCMEM;
    mbox[3] = 8;
    mbox[4] = 0;
    mbox[5] = 0;
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP)) {
        return mbox[5];
    }
    else{
        panic("could not query video core base memory");
    }
    return 0;  // error
}

void print_info_mem_freq(void){
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
        panic("Unable to query serial!\n");
    }
}

void set_max_freq(void){

    uint32_t maxclockrate = 1400000000;
    // now we set the clock rate to max so we go from 0.6GHz to 1.4GHz
    mbox[0] = 9*4;            
    mbox[1] = MBOX_REQUEST;         

    mbox[2] = MBOX_TAG_SETCLOCKRATE;
    mbox[3] = 12;
    mbox[4] = 0;
    mbox[5] = 0x000000003;   // ARM   
    mbox[6] = maxclockrate;
    mbox[7] = 0;

    mbox[8] = MBOX_TAG_LAST;

    if(mbox_call(MBOX_CH_PROP)){
        uint32_t new_rate = mbox[6];
        demand(new_rate == maxclockrate, "could not set max clock rate, new rate = %d\n", new_rate);
        is_turbo = 1;
    } else {
        panic("could not set vmax arm clock rate");
    }
}

uint32_t maxfreq(void){

    mbox[0] = 8*4;            
    mbox[1] = MBOX_REQUEST;         

    mbox[2] = MBOX_TAG_GETMAXCLOCKRATE;
    mbox[3] = 8;
    mbox[4] = 0;
    mbox[5] = 0x000000003;   // ARM   
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP)) {
        return mbox[6];
    }
    else {
        panic("could not query max frequency\n");
    }
    return 0; // error
}


int turbo_status(void){
    return is_turbo;
}