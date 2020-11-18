// https://github.com/dddrrreee/cs140e-20win/blob/master/labs/6-threads/2-threads.new/Q.h
//
#include "rpi.h"
#include "assert.h"
#include "helper_macros.h"

/*typedef struct node{
    struct node *next;
    uintptr_t ptr;
} node;

typedef struct Q {
    node *head, *tail;
    unsigned cnt;
} Q_t;*/

/**********normal linked lists****************/
// used for iteration.
node *Q_start(Q_t *q)   { return q->head; }
node *Q_next(node *e)      { return e->next; }
unsigned Q_nelem(Q_t *q) { return q->cnt; }

int Q_empty(Q_t *q)  { 
    if(q->head)
        return 0;
    assert(Q_nelem(q) == 0);
    demand(!q->tail, invalid Q);
    return 1;
}

// remove from front of list.
node *Q_pop(Q_t *q) {
    demand(q, bad input);

    node *e = q->head;
    if(!e) {
        assert(Q_empty(q));
        return 0;
    }
    q->cnt--;
    q->head = e->next;
    if(!q->head)
        q->tail = 0;
    return e;
}

// insert at tail. (for FIFO)
void Q_append(Q_t *q, node *e) {
    e->next = 0;
    q->cnt++;
    if(!q->tail) 
        q->head = q->tail = e;
    else {
        q->tail->next = e;
        q->tail = e;
    }
}

// insert at head (for LIFO)
void Q_push(Q_t *q, node *e) {
    q->cnt++;
    e->next = q->head;
    q->head = e;
    if(!q->tail)
        q->tail = e;
}

// insert <e_new> after <e>: <e>=<null> means put at head.
void Q_insert_after(Q_t *q, node *e, node *e_new) {
    if(!e)
        Q_push(q,e_new);
    else if(q->tail == e)
        Q_append(q,e_new);
    else {
        q->cnt++;
        e_new->next = e->next;
        e->next = e_new;
    }
}

/**********circular linked lists*************/

void add_circular(circularQ_t* entry, node* e){
    if(entry->ptr == 0){
        e->next = e;
        entry->next = e;
        entry->ptr++;
        return;
    }
    // insert after first element, easiest
    e->next = entry->next->next;
    entry->next->next = e;
    entry->ptr++;

}
void remove_circular(circularQ_t* entry, node* e){
    if(entry->ptr == 0)
        panic("empty list, nothing to remove");

    node *temp = entry->next;
    int found = 0;
    for(int i = 0; i < entry->ptr; i++, temp = temp->next){
        if(temp->next == e) {
            found = 1;
            break;
        }
    }
    if(!found)
        panic("element not found when deleting from circular linked list");

    if(entry->ptr == 1){
        entry->next = NULL;
        entry->ptr = 0;
        return;
    }

    if(temp->next == entry->next) // in case we removing the entry point
        entry->next = temp->next->next;
    temp->next = temp->next->next;
    entry->ptr--;
    // caller frees the node if it was allocated
}