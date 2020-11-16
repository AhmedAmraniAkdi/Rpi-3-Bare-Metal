// https://github.com/bztsrc/raspi3-tutorial/tree/master/04_mailboxes
// https://jsandler18.github.io/extra/mailbox.html
#include "VCmailbox.h"
#include "rpi.h"
#include <stdint.h>
/* mailbox message buffer */
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
