#ifndef __RPI_H__
#define __RPI_H__

#include <stdint.h>
/***********HELPERS************/

// https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64)
extern void PUT32(unsigned, unsigned);
extern void PUT8(unsigned, unsigned);
extern uint32_t GET32(unsigned);
extern uint8_t GET8(unsigned);
extern void CYCLE_DELAY(unsigned);
extern void DUMMY();
extern void BRANCHTO(unsigned);

extern uint64_t GETPC(void);
extern uint64_t GETEL(void);

extern void DSB(void);
extern void DMB(void);
extern void ISB(void);
extern int CORE_ID(void);

void reboot(void);
void clean_reboot(void);

/************DEBUGGING***********/
// Took these from https://github.com/dddrrreee/cs140e-20win/tree/master/libpi
// will be helpful

// change these two function pointers to control where pi output goes.
extern int (*rpi_putchar)(int c);
void rpi_reset_putc(void);
void rpi_set_putc(int (*fp)(int));

// int putk(const char *msg);
extern int (*putk)(const char *p);

// call to change output function pointers.
void rpi_set_output(int (*putc_fp)(int), int (*puts_fp)(const char *));

int printk(const char *format, ...);

/*********MEMORY ALLOCATION USING LINKED LISTS*************************/
typedef struct heap_segment{
    struct heap_segment * next;
    struct heap_segment * prev;
    uintptr_t alloc_ptr;
    uint8_t is_allocated;
    uint32_t segment_size;  
} heap_segment_t;

void* heap_segment_list_head_ptr(void);
void* kmalloc(uint64_t nbytes, uint16_t alignement);
void heap_init(void);
void kfree(void *ptr);
void kfree_all(void);

/***********GENERIC LINKED LISTS************************/

// normal linked lists
typedef struct node{
    struct node *next;
    uintptr_t ptr;
} node;

typedef struct Q {
    node *head, *tail;
    unsigned cnt;
} Q_t;

int Q_empty(Q_t *q);
node *Q_pop(Q_t *q);
void Q_append(Q_t *q, node *e);
void Q_push(Q_t *q, node *e);
void Q_insert_after(Q_t *q, node *e, node *e_new);
node *Q_start(Q_t *q);
node *Q_next(node *e);
unsigned Q_nelem(Q_t *q);

// circular linked lists
// we can use the same struct node as header
// uintptr would be num of elemets
// and *next the nodes
//                b->c|
//    header->   a|<-d
//
//
typedef node circularQ_t;
void add_circular(circularQ_t* entry, node* e);
void remove_circular(circularQ_t* entry, node* e);

#endif