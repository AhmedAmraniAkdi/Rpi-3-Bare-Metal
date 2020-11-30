.section ".text.boot"

.global _start
_start:
    // set up stacks
    ldr     x2, =__stack_end_core0__			
	mrs     x6, mpidr_el1						
	ands    x6, x6, #0x3
    cbz     x6, 1f						
    ldr     x2, =__stack_end_core1__		
	cmp     x6, #1								
	beq     1f							
    ldr     x2, = __stack_end_core2__			
	cmp     x6, #2								
	beq     1f							
    ldr     x2, =__stack_end_core3__			
1:
	msr	    sp_el1, x2		

2:
    // set up EL1
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // running at EL3?
    cmp     x0, #12
    bne     5f
    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, 5f
    msr     elr_el3, x2
    eret

5:
    // running at EL2?
    cmp     x0, #4
    beq     5f
    // enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    // enable AArch64 in EL1
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x0
    mrs     x0, hcr_el2
    // Setup SCTLR access
    mov     x2, #0x0800
    movk    x2, #0x30d0, lsl #16
    msr     sctlr_el1, x2
    // set up exception handlers done in irq.s
    ldr     x2, =vectors
    msr     vbar_el1, x2
    // set up neon and fpu for EL1
    //i'm proud of this 3 lines of code
    mov     x2, #0x0000
    movk    x2, #0x0030, lsl #16 // FPEN disables trapping to EL1.
    msr     cpacr_el1, x2
    // change execution level to EL1
    mov     x2, #0x3c5 // EL1h
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret

5:   
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 6f
    // cpu id > 0, stop
    bl WAIT_UNTIL_EVENT

6:
    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  
    cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  bl      notmain
    // for failsafe, halt this core too
    b       1b