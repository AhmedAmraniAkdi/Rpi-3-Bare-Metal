#include "testing_lib/mini_uart.h"
#include "testing_lib/rpi.h"
#include "testing_lib/VCmailbox.h"

void notmain()
{
    // set up serial console
    uart_init();
    
    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 0;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    uint64_t result = (((uint64_t) mbox[6]) << 32 ) | mbox[5];
    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        printk("My serial number is: ");
        printk("0x%x\n",result);
    } else {
        printk("Unable to query serial!\n");
    }

    while(1) {};
}