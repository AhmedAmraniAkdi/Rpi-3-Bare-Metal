#include "rpi.h"
#include "assert.h"
#include "helper_macros.h"
#include <stdint.h>

// symbol created by libpi/memmap, placed at the end
// of all the code/data in a pi binary file.
extern unsigned int __heap_start__;

// track if initialized.
static int init_p;

// heap pointer
static void* heap_ptr;

/*
 * Return a memory block of at least size <nbytes>
 * Notes:
 *  - There is no free, so is trivial: should be just 
 *    a few lines of code.
 *  - The returned pointer should always be 8-byte aligned.  
 *    Easiest way is to make sure the heap pointer starts 8-byte
 *    and you always round up the number of bytes.  Make sure
 *    you put an assertion in.  
 * 8 since 64bits
 */
void *kmalloc(unsigned nbytes) {
    if(!init_p)
        kmalloc_init();
    
    nbytes = pi_roundup(nbytes, 8);

    void* new_heap_ptr = heap_ptr;
    heap_ptr = heap_ptr + nbytes;

    demand(is_aligned_ptr(new_heap_ptr, 8), alignment not multiple of 8!);

    return new_heap_ptr;
}

// address of returned pointer should be a multiple of
// alignment.
void *kmalloc_aligned(unsigned nbytes, unsigned alignment) {
    if(!init_p)
        kmalloc_init();

    if(alignment <= 8)
        return kmalloc(nbytes);
    demand(alignment % 8 == 0, weird alignment: not a multiple of 8!);
    
    nbytes = pi_roundup(nbytes, alignment);

    void* new_heap_ptr = heap_ptr;
    heap_ptr = heap_ptr + nbytes;

    demand(is_aligned_ptr(new_heap_ptr, alignment), "alignment not multiple of %d!", alignment);

    return new_heap_ptr;
}

/*
 * One-time initialization, called before kmalloc 
 * to setup heap. 
 *    - should be just a few lines of code.
 *    - sets heap pointer to the location of 
 *      __heap_start__.   print this to make sure
 *      it makes sense!
 */
void kmalloc_init(void) {
    if(init_p)
        return;
    init_p = 1;
    
    heap_ptr = __heap_start__;

    output("Starting Heap\nheap_start: %u -- heap_ptr: %p", __heap_start__, heap_ptr);
}

/* 
 * free all allocated memory: reset the heap 
 * pointer back to the beginning.
 */
void kfree_all(void) {
    if(!init_p)
        kmalloc_init();
    
    heap_ptr = __heap_start__;
}

// return pointer to the first free byte.
// for the current implementation: the address <addr> of any
// allocated block satisfies: 
//    assert(<addr> < kmalloc_heap_ptr());
// 
void *kmalloc_heap_ptr(void) {
    if(!init_p)
        kmalloc_init();
    
    return heap_ptr;
}
