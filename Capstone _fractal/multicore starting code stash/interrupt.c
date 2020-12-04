#include "rpi.h"
#include "kassert.h"
#include "interrupt.h"

static interrupt_handler_f handlers[NUM_IRQS] = {0};
static interrupt_clearer_f clearers[NUM_IRQS] = {0};

static int irq_pos_timer = 0;

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
    int core = CORE_ID();
	printk("\nCore: %d, %s, ESR: %x, address: %x\r\n", core, entry_error_messages[type], esr, address);
    printk("ESR decoding: \n");
    // decode exception type
    switch(esr>>26) {
        case 0b000000: printk("Unknown"); break;
        case 0b000001: printk("Trapped WFI/WFE"); break;
        case 0b001110: printk("Illegal execution"); break;
        case 0b010101: printk("System call"); break;
        case 0b100000: printk("Instruction abort, lower EL"); break;
        case 0b100001: printk("Instruction abort, same EL"); break;
        case 0b100010: printk("Instruction alignment fault"); break;
        case 0b100100: printk("Data abort, lower EL"); break;
        case 0b100101: printk("Data abort, same EL"); break;
        case 0b100110: printk("Stack alignment fault"); break;
        case 0b101100: printk("Floating point"); break;
        default: printk("Unknown"); break;
    }
    // decode data abort cause
    if(esr>>26==0b100100 || esr>>26==0b100101) {
        printk(", ");
        switch((esr>>2)&0x3) {
            case 0: printk("Address size fault"); break;
            case 1: printk("Translation fault"); break;
            case 2: printk("Access flag fault"); break;
            case 3: printk("Permission fault"); break;
        }
        switch(esr&0x3) {
            case 0: printk(" at level 0"); break;
            case 1: printk(" at level 1"); break;
            case 2: printk(" at level 2"); break;
            case 3: printk(" at level 3"); break;
        }
    }
    printk("\n");
    panic("\nShutting down");
}

// irqs will be enabled when registering the handlers
void interrupt_init(void){
    PUT32(IRQ_BASIC_DISABLE, 0xFFFFFFFF);
    PUT32(GPU_ROUTING_INTERRUPT, 0x0);
    PUT32(LOCAL_TIMER_ROUTING, 0x0);
    for(int i = 0; i < 4; i++){
        PUT32(CORE_TIMER_INTERRUPT_CTRL+ i * 4, 0x0);
        PUT32(CORE_MAILBOX_INTERRUPT + i * 4, 0x0);
    }
    enable_irq();
    DSB();
}

void disable_core_timer_int(void){
    unsigned core = CORE_ID();
    PUT32(CORE_TIMER_INTERRUPT_CTRL+ core * 4, 0x0);
}

void enable_core_timer_int(void){
    unsigned core = CORE_ID();
    PUT32(CORE_TIMER_INTERRUPT_CTRL+ core * 4, 2);
}

int is_pending(irq_number_t irq_num){
    uint32_t core_id = CORE_ID(); // check in which core we are

    if(core_id != 0){
        unsigned arm_control = 0;
        arm_control = ((1 << (irq_num - (72 + 12 * core_id))) & GET32(CORE_INTERRUPT_IRQ_SOURCE + 4 * core_id));
        return arm_control;
    } else {
        unsigned basic = 0, arm_control = 0;

        basic = IRQ_IS_BASIC(irq_num) && ((1 << (irq_num - 64)) & GET32(IRQ_BASIC_PENDING));
    
        arm_control = IRQ_IS_ARM_CONTROL(irq_num) && ( (1 << (irq_num - 72)) & GET32(CORE_INTERRUPT_IRQ_SOURCE));
        return (basic || arm_control);
    }
}

void register_irq_handler(irq_number_t irq_num, core_number_t core, interrupt_handler_f handler, interrupt_clearer_f clearer){
    int irq_pos = -1;
    if (IRQ_IS_ARM_CONTROL(irq_num)){
        // core 0 will set up these
        handlers[irq_num] = handler;
        clearers[irq_num] = clearer;

        uint32_t irq_ctrl_register = 0xFFFFFFFF; //qa7 has separate registers
        irq_pos = irq_num - (72 + 12 * core);
        if(irq_pos < 4){
            irq_pos_timer = irq_pos;
            irq_ctrl_register = CORE_TIMER_INTERRUPT_CTRL + 4 * core;
        }
        else if (irq_pos == 11){
            irq_ctrl_register = LOCAL_TIMER_CONTROL;
            irq_pos = 29;
        }
        else if(irq_pos > 3 && irq_pos < 8){
            irq_ctrl_register = CORE_MAILBOX_INTERRUPT + 4 * core;
            // example: mailbox3 of core 2: irq_num = 103
            // irq_pos = 3 which is correct
            irq_pos = irq_num - (72 + 12 * core) - 4;
        }
        demand(irq_pos != -1 || irq_ctrl_register == 0xFFFFFFFF, "could not unregister irq handler");
        uint32_t r = GET32(irq_ctrl_register);
        PUT32(irq_ctrl_register, r | (1 << irq_pos));
    } 
    else if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = handler;
	    clearers[irq_num] = clearer;
        uint32_t r = GET32(IRQ_BASIC_ENABLE);
        PUT32(IRQ_BASIC_ENABLE, r | (1 << irq_pos));
    }
    else {
        panic("ERROR: CANNOT REGISTER IRQ HANDLER: INVALID IRQ NUMBER: %d\n", irq_num);
    }
    demand(handlers[irq_num] != 0, "handler wasnt set correctly");
    demand(clearers[irq_num] != 0, "clearer wasnt set correctly");
    DSB();
}

void unregister_irq_handler(irq_number_t irq_num, core_number_t core){
    int irq_pos = -1;
    if (IRQ_IS_ARM_CONTROL(irq_num)){
        // core 0 will clear up these
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;

        uint32_t irq_ctrl_register = 0xFFFFFFFF; //qa7 has separate registers
        irq_pos = irq_num - (72 + 12 * core);
        if(irq_pos < 4){
            irq_ctrl_register = CORE_TIMER_INTERRUPT_CTRL + 4 * core;   
        }
        else if (irq_pos == 11){
            irq_ctrl_register = LOCAL_TIMER_CONTROL;
            irq_pos = 29;
        }
        else if(irq_pos > 3 && irq_pos < 8){
            irq_ctrl_register = CORE_MAILBOX_INTERRUPT + 4 * core;
            // example: mailbox3 of core 2: irq_num = 103
            // irq_pos = 3 which is correct
            irq_pos = irq_num - (72 + 12 * core) - 4;
        }
        uint32_t r = GET32(irq_ctrl_register);
        demand(irq_pos != -1 || irq_ctrl_register == 0xFFFFFFFF, "could not unregister irq handler");
        PUT32(irq_ctrl_register, r & ~(1 << irq_pos));
    }
    else if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        uint32_t r = GET32(IRQ_BASIC_DISABLE);
        PUT32(IRQ_BASIC_DISABLE, r | (1 << irq_pos));
    }
    else {
        panic("ERROR: CANNOT UNREGISTER IRQ HANDLER: INVALID IRQ NUMBER: %d\n", irq_num);
    }
    demand(handlers[irq_num] == 0, "handler wasnt cleared correctly");
    demand(clearers[irq_num] == 0, "clearer wasnt cleared correctly");
    DSB();
}

// processor disables all ints when irq_handler is called , no nested interrupts for now
// loop : that way we eat multiple ints if there are demanded
void irq_handler(void) {
    // we don't want a core eating irqs of other cores
    uint32_t core_id = CORE_ID();
    int start, end;
    if(core_id != 0){
        start = 72 + 12 * core_id;
        end   = start + 12;
    } else {
        start = 0;
        end   = 84;
    }
	for (int j = start; j < end; j++) {
        if (is_pending(j)  && (handlers[j] != 0)) {
            DSB();
		    clearers[j]();
		    handlers[j]();
            DSB();
        }
    }
}
