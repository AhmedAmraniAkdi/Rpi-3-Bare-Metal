.globl WAIT_UNTIL_EVENT
WAIT_UNTIL_EVENT:
	wfe				           // sleep until sev is called from core 0 (sev) generally and continue execution from next instruction
	mov	x5, #0xC0                
	movk x5, #0x4000, lsl #16  // Load mailbox0 read address = 0x400000C0
	mrs x6, MPIDR_EL1		   // Fetch core Id
	and x6, x6, #0x3		   // Create 2 bit mask of core Id
	mov x6, x6, lsl #4         // core_id *= 16
	ldr w4, [x5, x6]		   // Read the mailbox
	cbz w4, WAIT_UNTIL_EVENT   // If zero spinlock
	str	w4, [x5, x6]		   // Write to Clear the read address	
    blr x4					   // Call address in x4 
	b WAIT_UNTIL_EVENT		   // should not go back here, Loop back to spinlock

.globl WAKE_CORES
WAKE_CORES:
    dmb sy
    sev
    ret