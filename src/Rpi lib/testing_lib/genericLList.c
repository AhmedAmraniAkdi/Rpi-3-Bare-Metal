#include "rpi.h"
#include "assert.h"
#include "helper_macros.h"
// generic linked lists
// a normal one and a circular one

//typedef struct node{
//    node* next;    // next node
//    uintptr_t ptr; // pointer to data
//} node;

void add_circular(node* entry, void* ptr_data){
    // empty
    if(entry->next == NULL && entry->ptr == 0){
        entry->ptr = (uintptr_t) ptr_data;
        entry->next = entry;
        return;
    }

    node* n = (node*) kmalloc(sizeof(node), 16);
    n->ptr = (uintptr_t) ptr_data;
    n->next = entry->next;
    entry->next = n;
    return;
}

void remove_circular(node* entry, void* ptr_data){
    node* temp = entry;
    while(temp->next->ptr != (uintptr_t) ptr_data){
        temp = temp->next;
        if(temp->next == entry)
            panic("element not found when removing from circular");
    }
    node* found = temp->next;
    temp->next = found->next;
    // careful with this, what happens when we are context switched?
    // nothing? heap is shared betwenn all threads?
    kfree(found);
}

// remove from start
node* pop(node* entry){
    if(entry->next == NULL && entry->ptr == 0)
        panic("nothing to pop, list is empty");
    node* temp = entry;
    entry = entry->next;
    return temp;
}

// add at start
void push(node* entry, void* ptr_data){
    if(entry->next == NULL && entry->ptr == 0){
        entry->ptr = (uintptr_t) ptr_data;
        return;
    }
    node* n = (node*) kmalloc(sizeof(node), 16);
    n->ptr = (uintptr_t) ptr_data;
    n->next = entry;
    entry = n;
}

void remove_element(node* entry, void* ptr_data){
    node* temp = entry;
    while(temp->next->ptr != (uintptr_t) ptr_data){
        temp = temp->next;
        if(temp->next == NULL)
            panic("element not found when removing from circular");
    }
    node* found = temp->next;
    temp->next = found->next;
    // careful with this, what happens when we are context switched?
    // nothing? heap is shared betwenn all threads?
    kfree(found);
}

// insert at end
// void append(node* entry){}
// remove from end
// void prepend(node* entry){}
// element in list
// int is_present(node* entry, void* ptr_data){}
// is empty
// int is_empty(node* entry){}