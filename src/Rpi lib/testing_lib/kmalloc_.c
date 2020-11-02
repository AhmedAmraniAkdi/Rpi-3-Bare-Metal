#include "rpi.h"
#include "assert.h"
#include "helper_macros.h"
#include <stdint.h>

// more sophisticated than cs140e kmalloc
// uses linked lists and no need to reset rpi for freeing allocations
// 1Mb heap

// defined in linked file after bss
extern unsigned int __heap_start;
#define HEAP_SIZE 0x100000 // 1MB

static heap_segment_t * heap_segment_list_head;

typedef struct heap_segment{
    struct heap_segment * next;
    struct heap_segment * prev;
    uint8_t is_allocated;
    uint32_t segment_size;  
} heap_segment_t;

void * kmalloc(uint64_t bytes) {
    heap_segment_t * curr, *best = NULL;
    int diff, best_diff = 0x7fffffff; // Max signed int

    // Add the header to the number of bytes we need and make the size 8 byte aligned
    bytes += sizeof(heap_segment_t);
    bytes = pi_roundup(bytes, 8);
    

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
    // Since our segment headers are rather large, the criterion for splitting the segment is that
    // when split, the segment not being requested should be twice a header size
    if (best_diff > (int)(2 * sizeof(heap_segment_t))) {
        
        // i set the struct below, no need to zero it
        curr = best->next;
        best->next = ((char*)(best)) + bytes;
        best->next->next = curr;
        best->next->prev = best;
        best->next->segment_size = best->segment_size - bytes;
        best->segment_size = bytes;
    }

    best->is_allocated = 1;

    return best + 1; // pointer directly after header
}

void kfree(void *ptr) {
    heap_segment_t * seg = ptr - sizeof(heap_segment_t);
    seg->is_allocated = 0;

    // try to coalesce segments to the left
    while(seg->prev != NULL && !seg->prev->is_allocated) {
        seg->prev->next = seg->next;
        seg->next->prev = seg->prev;
        seg->prev->segment_size += seg->segment_size;
        seg = seg->prev;
    }
    // try to coalesce segments to the right
    while(seg->next != NULL && !seg->next->is_allocated) {
        seg->segment_size += seg->next->segment_size;
        seg->next = seg->next->next;
        seg->next->prev = seg;
    }
}

void heap_init(void) {
    heap_segment_list_head = (heap_segment_t *) &__heap_start;
    heap_segment_list_head->next = NULL;
    heap_segment_list_head->prev = NULL; // before the heap there is the bss
    heap_segment_list_head->segment_size = HEAP_SIZE;
    heap_segment_list_head->is_allocated = 0;
}