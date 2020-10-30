#include "rpi.h"
#include "interrupt.h"
#include "assert.h"

static interrupt_handler_f handlers[NUM_IRQS] = {0};
static interrupt_clearer_f clearers[NUM_IRQS] = {0};

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32"	
};

void show_invalid_entry_message(int type, uint64_t esr, uint64_t address){
	panic("\n%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr, address);
}

// irqs will be enabled when registering the handlers
void interrupt_init(void){
    PUT32(IRQ_BASIC_DISABLE, 0xFFFFFFFF);
    PUT32(IRQ_GPU_DISABLE1, 0xFFFFFFFF);
    PUT32(IRQ_GPU_DISABLE2, 0xFFFFFFFF);
    irq_vector_init();
    enable_irq();
    DSB();
}

int is_pending(irq_number_t irq_num){
    return ((IRQ_IS_BASIC(irq_num) && ((1 << (irq_num - 64)) & GET32(IRQ_BASIC_PENDING))) 
                || (IRQ_IS_GPU2(irq_num) && ((1 << (irq_num - 32)) & GET32(IRQ_GPU_PENDING2))) 
                || (IRQ_IS_GPU1(irq_num) && ((1 << (irq_num)) & GET32(IRQ_GPU_PENDING1))));
}

void register_irq_handler(irq_number_t irq_num, interrupt_handler_f handler, interrupt_clearer_f clearer){
    uint32_t irq_pos;
    if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = handler;
	    clearers[irq_num] = clearer;
        uint32_t r = GET32(IRQ_BASIC_ENABLE);
        PUT32(IRQ_BASIC_ENABLE, r | (1 << irq_pos));
    }
    else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = handler;
	    clearers[irq_num] = clearer;
        uint32_t r = GET32(IRQ_GPU_ENABLE2);
        PUT32(IRQ_GPU_ENABLE2, r | (1 << irq_pos));
    }
    else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = handler;
	    clearers[irq_num] = clearer;
        uint32_t r = GET32(IRQ_GPU_ENABLE1);
        PUT32(IRQ_GPU_ENABLE1, r | (1 << irq_pos));
    }
    else {
        panic("ERROR: CANNOT REGISTER IRQ HANDLER: INVALID IRQ NUMBER: %d\n", irq_num);
    }
    DSB();
}
void unregister_irq_handler(irq_number_t irq_num){
    uint32_t irq_pos;
    if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        uint32_t r = GET32(IRQ_BASIC_DISABLE);
        PUT32(IRQ_BASIC_DISABLE, r | (1 << irq_pos));
    }
    else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        uint32_t r = GET32(IRQ_GPU_DISABLE2);
        PUT32(IRQ_GPU_DISABLE2, r | (1 << irq_pos));
    }
    else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        uint32_t r = GET32(IRQ_GPU_DISABLE1);
        PUT32(IRQ_GPU_DISABLE1, r | (1 << irq_pos));
    }
    else {
        panic("ERROR: CANNOT UNREGISTER IRQ HANDLER: INVALID IRQ NUMBER: %d\n", irq_num);
    }
    DSB();
}

// processor disables all ints when irq_handler is called , no nested interrupts for now
// loop : that way we eat multiple ints if there are demanded
void irq_handler(void) {
	for (int j = 0; j < NUM_IRQS; j++) {
        if (IRQ_IS_PENDING(j)  && (handlers[j] != 0)) {
            DSB();
		    clearers[j]();
		    handlers[j]();
            DSB();
        }
    }
}