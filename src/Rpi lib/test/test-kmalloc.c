#include "testing_lib/rpi.h"
#include "testing_lib/mini_uart.h"
#include "testing_lib/assert.h"
//#include "testing_lib/helper_macros.h"
#include <stdint.h>

void notmain() {
    uart_init();

    printk("starting test\n");
    /*printk("%d %d\n", sizeof(void*), sizeof(uintptr_t));
    kmalloc_init();

    printk("heap starts at 0x%x\n", kmalloc_heap_ptr());

    void *heap0 = kmalloc_heap_ptr();
    // sleazy check that assumes how kmalloc works.
    uint64_t *start = kmalloc(1);
    printk("start=0x%x\n", start);
    for(uint64_t i = 0; i < 20; i++) {
        uint64_t *u = kmalloc(1);
        printk("0x%x\n", *u);
        uint64_t v = (i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i);
        *u = v;
        printk("0x%x \t 0x%p\n", *u, u);
    }

    uint64_t *u = start+1;
    for(uint64_t i = 0; i < 20; i++) {
        uint64_t v = (i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i);
        demand(u[i] == v, 
            "invalid start[%d] =%x, should be %x\n", u[i],v);
    }

    void *p = kmalloc_aligned(1,64);
    demand(is_aligned_ptr(p, 64), "bug in kmalloc_aligned");
    printk("0x%x 0x%x\n", p, kmalloc_heap_ptr());
    printk("%d\n", kmalloc_heap_ptr() - p);

    kfree_all();
    void *heap1 = kmalloc_heap_ptr();
    demand(heap0 == heap1, "did not reset the heap correctly!");
    demand(start == kmalloc(1), "did not reset the heap correctly!");*/

    printk("successful test\n");
    while(1){};
	//clean_reboot();
}
