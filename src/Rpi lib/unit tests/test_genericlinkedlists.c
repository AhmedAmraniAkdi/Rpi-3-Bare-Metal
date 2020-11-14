#include "testing_lib/rpi.h"
#include "testing_lib/assert.h"
#include "testing_lib/helper_macros.h"


void notmain(){
    printk("**********TESTING LINKED LIST***************\n");
    Q_t *entry = (Q_t *) kmalloc(sizeof(Q_t), 16);

    int a[5] = {1,2,3,4,5};

    for(int i = 0; i < 5; i++){
        node *temp = (node*) kmalloc(sizeof(node), 16);
        temp->next = NULL;
        temp->ptr = (uintptr_t) &a[i];
        Q_push(entry, temp);
    }

    node *temp = entry->head;
    while(temp){
        int* n = (int*) temp->ptr;
        printk("address 0x%x\t value:%d\n", temp->ptr, *n);
        temp = temp->next;
    }
    
    printk("*********TESTING CIRCULAR LIST**************\n");

    circularQ_t *C_entry =(circularQ_t *) kmalloc(sizeof(circularQ_t), 16);

    node* temp_ptr_array[5];
    for(int i = 0; i < 5; i++){
        temp_ptr_array[i] = (node*) kmalloc(sizeof(node), 16);
        temp_ptr_array[i]->next = NULL;
        temp_ptr_array[i]->ptr = (uintptr_t) &a[i];
        add_circular(C_entry, temp_ptr_array[i]);
    }
    
    temp = C_entry->next;
    for(int i = 0; i < C_entry->ptr * 2; i++, temp = temp->next){
        int* n = (int*) temp->ptr;
        printk("address 0x%x\t value:%d\n", temp->ptr, *n);
    }

    remove_circular(C_entry, temp_ptr_array[0]);
    remove_circular(C_entry, temp_ptr_array[2]);
    remove_circular(C_entry, temp_ptr_array[4]);
    remove_circular(C_entry, temp_ptr_array[1]);

    printk("removed elements\n");
    temp = C_entry->next;
    for(int i = 0; i < C_entry->ptr * 2; i++, temp = temp->next){
        int* n = (int*) temp->ptr;
        printk("address 0x%x\t value:%d\n", temp->ptr, *n);
    }
    printk("trying to remove element not found, should throw error\n");
    remove_circular(C_entry, (node *) kmalloc(sizeof(node), 16));
    
    printk("removing rest\n");
    printk("count:%d\t head:0x%x\n", C_entry->ptr, C_entry->next);
    remove_circular(C_entry, temp_ptr_array[3]);
    printk("count:%d\t head:0x%x\n", C_entry->ptr, C_entry->next);
    // removing when empty
    //remove_circular(C_entry, (node *) kmalloc(sizeof(node), 16));
    printk("sucessful test\n");
    while(1){}
}