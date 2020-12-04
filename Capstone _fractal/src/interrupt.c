#include "rpi.h"
#include "kassert.h"
#include "interrupt.h"

static interrupt_handler_f handlers[4] = {0};
static interrupt_clearer_f clearers[4] = {0};

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
#define CORE_MAILBOX_WRITETOSET 0x40000080
void enable_irq_secondary(void){
    enable_irq();
}

void interrupt_init(void){
    for(int i = 0; i < 4; i++){
        PUT32(CORE_TIMER_INTERRUPT_CTRL+ i * 4, 0x0);
        PUT32(CORE_MAILBOX_INTERRUPT + i * 4, 0x0);
    }
    enable_irq();
    PUT32(CORE_MAILBOX_WRITETOSET + 16, (uintptr_t)&enable_irq_secondary);
	PUT32(CORE_MAILBOX_WRITETOSET + 32, (uintptr_t)&enable_irq_secondary);
	PUT32(CORE_MAILBOX_WRITETOSET + 48, (uintptr_t)&enable_irq_secondary);
	WAKE_CORES();
    DSB();
}

void disable_core_timer_int(void){
    unsigned core = CORE_ID();
    PUT32(CORE_TIMER_INTERRUPT_CTRL+ core * 4, 0x0);
}

void enable_core_timer_int(void){
    unsigned core = CORE_ID();
    PUT32(CORE_TIMER_INTERRUPT_CTRL+ core * 4, 0x2);
}

int is_pending(){
    uint32_t core_id = CORE_ID(); // check in which core we are
    unsigned arm_control = 0;
    arm_control = (0x2 & GET32(CORE_INTERRUPT_IRQ_SOURCE + 4 * core_id));
    return arm_control;
}

void register_irq_handler(irq_number_t irq_num, core_number_t core, interrupt_handler_f handler, interrupt_clearer_f clearer){
    // core 0 will set up these
    handlers[core] = handler;
    clearers[core] = clearer;

    uint32_t irq_ctrl_register = CORE_TIMER_INTERRUPT_CTRL + 4 * core; //qa7 has separate registers
    int irq_pos = irq_num - (72 + 12 * core);
    uint32_t r = GET32(irq_ctrl_register);
    
    PUT32(irq_ctrl_register, r | (1 << irq_pos));
    
    DSB();
}

// processor disables all ints when irq_handler is called , no nested interrupts for now
// loop : that way we eat multiple ints if there are demanded
void irq_handler(void) {
    unsigned core = CORE_ID();
    if (is_pending()  && (handlers[core] != 0)) {
        DSB();
		clearers[core]();
		handlers[core]();
        DSB();
    }
}

