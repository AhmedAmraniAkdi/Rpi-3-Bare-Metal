#include "testing_lib/rpi.h"
#include "testing_lib/mini_uart.h"
#include "testing_lib/assert.h"
#include "testing_lib/helper_macros.h"
#include <stdint.h>

struct f{
    uint8_t g;
    uint8_t h;
    uint32_t i[20];
};

struct testing_struct{
    uint64_t a;
    uint64_t b;
    char c;
    uint64_t d[10];
    int k;
    uint64_t* e;
    struct f j;
};

void notmain() {
    uart_init();

    printk("*************TESTING ALLOCATIONS*******************\n");
    heap_init();
    heap_segment_t * h = heap_segment_list_head_ptr();
    printk("heap starts at 0x%x\n", h);
    printk("next:%x \t prev:%x \t is_alocatted %u \t segment_size:%u\t alloc_ptr:%x\n",
        h->next, h->prev, h->is_allocated, h->segment_size, h->alloc_ptr);

    uint64_t *start = kmalloc(1, 8);

    printk("next:%x \t prev:%x \t is_alocatted %u \t segment_size:%u\t alloc_ptr:%x\n",
        h->next, h->prev, h->is_allocated, h->segment_size, h->alloc_ptr);

    printk("start=0x%x\n", start);
    for(uint64_t i = 0; i < 20; i++) {
        uint64_t *u = kmalloc(8, 8);
        demand(is_aligned_ptr(u, 8), "allocation not aligned at 8");
        printk("0x%x\n", *u);
        uint64_t v = (i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i);
        *u = v;
        printk("0x%x \t 0x%p\n", *u, u);
    }

    printk("alignement 16\n");
    for(uint64_t i = 0; i < 20; i++) {
        uint64_t *z = kmalloc(8, 16);
        demand(is_aligned_ptr(z, 16), "allocation not aligned at 8");
        uint64_t v = (i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i);
        *z = v;
        printk("0x%x \t 0x%p\t 0x%x\t 0x%x\n", *z, z, *(z+1), z+1);
    }

    void *p = kmalloc(1,64);
    demand(is_aligned_ptr(p, 64), "bug in kmalloc_aligned");
    printk("alignement 64 0x%x \t 0bx%b\n", p, p);

    kfree_all();
    void *heap1 = heap_segment_list_head_ptr();
    demand(h == heap1, "did not reset the heap correctly!");
    demand(start == kmalloc(1, 8), "did not reset the heap correctly!");

    printk("allocating the testing struct of size %u\n", sizeof(struct testing_struct));
    kfree_all();
    struct testing_struct* k = (struct testing_struct*) kmalloc(sizeof(struct testing_struct), 16);
    demand(is_aligned_ptr(k, 16), "testing struct not aligned");
    printk("next:%x \t prev:%x \t is_alocatted %u \t segment_size:%u\n",
        h->next, h->prev, h->is_allocated, h->segment_size);

    printk("**************TESTING FREEING MEMORY****************\n");
    kfree_all();
    uint64_t *u = (uint64_t*) kmalloc(sizeof(uint64_t) * 20, 8);
    for(uint64_t i = 0; i < 20; i++) {
        demand(is_aligned_ptr((u + i), 8), "allocation not aligned at 8");
        uint64_t v = (i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i);
        u[i] = v;
        printk("0x%x \t 0x%p\n", u[i], u+i);
    }
    kfree_all();
    uint64_t *w = (uint64_t*) kmalloc(sizeof(uint64_t) * 20, 8);
    for(uint64_t i = 0; i < 20; i++) {
        demand(is_aligned_ptr((w + i), 8), "allocation not aligned at 8");
        demand((u+i)== (w+i), "allocation not repeatable?");
        uint64_t v = (i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i);
        w[i] = v;
        printk("0x%x \t 0x%p\n", w[i], w+i);
    }

    kfree_all();
    
    struct testing_struct *a, *b, *c, *d, *e, *f;
    a = (struct testing_struct*) kmalloc(sizeof(struct testing_struct)* 5, 16);
    b = (struct testing_struct*) kmalloc(sizeof(struct testing_struct)* 4, 16);
    c = (struct testing_struct*) kmalloc(sizeof(struct testing_struct)* 3, 16);
    d = (struct testing_struct*) kmalloc(sizeof(struct testing_struct)* 2, 16);
    e = (struct testing_struct*) kmalloc(sizeof(struct testing_struct)* 1, 16);
    f = (struct testing_struct*) kmalloc(sizeof(struct testing_struct)* 1, 16);

    heap_segment_t* temp = heap_segment_list_head_ptr();
    while(temp){
        printk("next:%x \t prev:%x \t is_alocatted %u \t segment_size:%u\t alloc_ptr:%x\n",
            temp->next, temp->prev, temp->is_allocated, temp->segment_size, temp->alloc_ptr);
        temp = temp->next;
    }

    kfree(a); kfree(f); kfree(d); kfree(e); 

    temp = heap_segment_list_head_ptr();
    while(temp){
        printk("next:%x \t prev:%x \t is_alocatted %u \t segment_size:%u\t alloc_ptr:%x\n",
            temp->next, temp->prev, temp->is_allocated, temp->segment_size, temp->alloc_ptr);
        temp = temp->next;
    }

    kfree(b); kfree(c);

    printk("heap should be empty\nheap starts at 0x%x\n", h);
    printk("next:%x \t prev:%x \t is_alocatted %u \t segment_size:%u\t alloc_ptr:%x\n",
        h->next, h->prev, h->is_allocated, h->segment_size, h->alloc_ptr);

    printk("successful test\n");
    while(1){};
}
