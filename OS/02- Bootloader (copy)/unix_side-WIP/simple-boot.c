#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "libunix.h"
#include "send-recv.h"

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"

// check that the expected value <exp> is equal the the value we <got>.
// on mismatch, // drains the tty and echos (to help debugging) and then 
// dies.
static void ck_eq32(int fd, const char *msg, unsigned exp, unsigned got) {
	if(exp == got)
        return;

    // i think there can be garbage in the initial /tty.
    output("%s: expected %x, got %x\n", msg, exp, got);

    // after error: just echo the pi output.
    unsigned char b;
    while(read(fd, &b, 1) == 1) 
        fputc(b, stderr);

    panic("pi-boot failed\n");
}

// hack to handle unexpected PRINT_STRINGS: call <get_op> for the
// first uint32 in a message.  the code checks if it received a 
// PRINT_STRING and emits if so, otherwise returns the value.
uint32_t get_op(int fd) {
    uint32_t op = get_uint32(fd);
    if(op != PRINT_STRING)
        return op;

    unsigned nbytes = get_uint32(fd);
    demand(nbytes < 512, pi sent a suspiciously long string);
    output("pi sent print: <");
    for(int i = 0; i < nbytes - 1; i++)
        output("%c", get_byte(fd));
    output(">\n");
    return 1;
}


void simple_boot(int fd, const uint8_t *buf, unsigned n) { 
    //uint32_t op;

    // if n is not a multiple of 4, use roundup() in libunix.h
    n = roundup(n,4);
    demand(n % 4 == 0, boot code size must be a multiple of 4!);

    output("******************sending %d bytes\n", n);

    while(1){
        output("expected initial 48 got <%d>: discarding.\n", get_byte(fd));
    }
    
    /*put_byte(fd, 111);

    while((op = get_byte(fd)) != 159){
        output("expected initial 159 got <%d>: discarding.\n", op);
    }

    get_op(fd);
    put_byte(fd, 123);
    put_byte(fd, 124);
    while(1){};*/
}
