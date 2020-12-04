.globl ret_from_fork
ret_from_fork:
    bl preempt_enable
    mov x0, x20 // arg first arg to function
    mov x1, x21 // ret value second arg to function
    blr x19     // jump to function
    b exit_task // finish and zombifie thread

// also known as brain surgery
.globl context_switch
context_switch:
    mov x10, #0 //#THREAD_CPU_CONTEXT
    add x8, x0, x10 // I mean we are not doing anything here ?
    mov x9, sp
    stp x19, x20, [x8], #16        // store callee-saved registers
    stp x21, x22, [x8], #16
    stp x23, x24, [x8], #16
    stp x25, x26, [x8], #16
    stp x27, x28, [x8], #16
    st1 {  v8.1d,  v9.1d, v10.1d, v11.1d }, [x8], #32
    st1 { v12.1d, v13.1d, v14.1d, v15.1d }, [x8], #32
    stp x29, x9, [x8], #16
    str x30, [x8]

    add x8, x1, x10
    ldp x19, x20, [x8], #16        // restore callee-saved registers
    ldp x21, x22, [x8], #16
    ldp x23, x24, [x8], #16
    ldp x25, x26, [x8], #16
    ldp x27, x28, [x8], #16
    ld1 {  v8.1d,  v9.1d, v10.1d, v11.1d }, [x8], #32
    ld1 { v12.1d, v13.1d, v14.1d, v15.1d }, [x8], #32
    ldp x29, x9, [x8], #16
    ldr x30, [x8]
    mov sp, x9

    ret

.globl reset_mains_stack
reset_mains_stack:
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
	mov	    sp, x2
    ret