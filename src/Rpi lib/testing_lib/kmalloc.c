#include "assert.h"
#include "rpi.h"
#include "helper_macros.h"
#include <stdint.h>

// more sophisticated than cs140e kmalloc
// uses linked lists and no need to reset rpi for freeing allocations
// 1Mb heap

// defined in linked file after bss
extern unsigned int __heap_start;
#define HEAP_SIZE 0x100000 // 1MB

static heap_segment_t * heap_segment_list_head;

/*typedef struct heap_segment{
    struct heap_segment * next;
    struct heap_segment * prev;
    uintptr_t alloc_ptr;
    uint8_t is_allocated;
    uint32_t segment_size;  
} heap_segment_t;*/

// address of returned pointer should be a multiple of alignment.
void * kmalloc(uint64_t bytes, uint16_t alignement) {
    heap_segment_t * curr, *best = NULL;
    int diff, best_diff = 0x7fffffff; // Max signed int

    // ok so, if we want the pointer to be aligned, we will have to add to the header size up to <alignment> bytes
    // otherwise the pointer to the data wont be aligned
    // we can't round up now because we don't know the start of the allocation (the best one)
    bytes = sizeof(heap_segment_t) + alignement + bytes;
    

    // Find the allocation that is closest in size to this request
    for (curr = heap_segment_list_head; curr != NULL; curr = curr->next) {
        diff = curr->segment_size - bytes;
        if (!curr->is_allocated && diff < best_diff && diff >= 0) {
            best = curr;
            best_diff = diff;
        }
    }

    if (best == NULL)
        return NULL;

    // If the best difference we could come up with was large, split up this segment into two.
    if (best_diff > (int)(2 * sizeof(heap_segment_t))) {
        curr = best->next;
        best->next = (struct heap_segment*) pi_roundup((((uintptr_t)(best) + bytes)), 8); // aligned to 8 bytes
        best->next->next = curr;
        curr->prev = best->next;
        best->next->prev = best;
        best->next->segment_size = best->segment_size - bytes;
        best->next->alloc_ptr = 0;
        best->next->is_allocated = 0;
        best->segment_size = bytes;
    }

    best->is_allocated = 1;
    // address of returned pointer should be a multiple of alignment.
    best->alloc_ptr = pi_roundup((uintptr_t) best + sizeof(heap_segment_t), alignement);
    demand(is_aligned(best->alloc_ptr, alignement), "pointer allocated not aligned <%d>", alignement);
    return (void*) best->alloc_ptr;
}

void kfree(void *ptr) {
    heap_segment_t* temp = heap_segment_list_head;
    //printk("\n1");
    while(temp){
        if(temp->alloc_ptr == (uintptr_t) ptr){
            break;
        }
        temp = temp->next;
    }
    demand(temp != NULL, "header of pointer not found when freeing?");
    temp->is_allocated = 0;
    temp->alloc_ptr = 0;
    // try to grp segments to the left
    //printk("2");
    while(temp->prev != NULL && !temp->prev->is_allocated) {
        temp->prev->next = temp->next;
        temp->next->prev = temp->prev;
        temp->prev->segment_size += temp->segment_size;
        temp->prev->alloc_ptr = 0;
        temp = temp->prev;
    }
    // try to grp segments to the right
    while(temp->next != NULL && !temp->next->is_allocated) {
        temp->segment_size += temp->next->segment_size;
        temp->next = temp->next->next;
        if(temp->next != NULL)
            temp->next->prev = temp;
    }
}

void heap_init(void) {
    heap_segment_list_head = (heap_segment_t *) &__heap_start;
    heap_segment_list_head->next = NULL;
    heap_segment_list_head->prev = NULL; // before the heap there is the bss
    heap_segment_list_head->alloc_ptr = 0;
    heap_segment_list_head->segment_size = HEAP_SIZE;
    heap_segment_list_head->is_allocated = 0;
}

void * heap_segment_list_head_ptr(void){
    return (void*) heap_segment_list_head;
}

void kfree_all(void){
    heap_init();
}