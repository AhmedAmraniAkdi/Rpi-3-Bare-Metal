.globl ret_from_fork
ret_from_fork:
    bl preempt_enable
    mov x0, x20
    blr x19         

// also known as brain surgery
.globl cpu_switch_to
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

