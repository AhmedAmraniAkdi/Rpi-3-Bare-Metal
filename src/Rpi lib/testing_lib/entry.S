#include "entry.h"

.macro handle_invalid_entry type
kernel_entry
mov	x0, #\type
mrs	x1, esr_el1
mrs	x2, elr_el1
bl	show_invalid_entry_message
b	err_hang
.endm

.macro	ventry	label
.align	7
b	\label
.endm

.macro	kernel_entry
// since st1 needs post index, everything post index
sub	sp, sp, #784
stp	x0, x1, [sp], #16 //[sp, #16 * 0]
stp	x2, x3, [sp], #16 //[sp, #16 * 1]
stp	x4, x5, [sp], #16//[sp, #16 * 2]
stp	x6, x7, [sp], #16//[sp, #16 * 3]
stp	x8, x9, [sp], #16//[sp, #16 * 4]
stp	x10, x11, [sp], #16//[sp, #16 * 5]
stp	x12, x13, [sp], #16//[sp, #16 * 6]
stp	x14, x15, [sp], #16//[sp, #16 * 7]
stp	x16, x17, [sp], #16//[sp, #16 * 8]
stp	x18, x19, [sp], #16//[sp, #16 * 9]
stp	x20, x21, [sp], #16//[sp, #16 * 10]
stp	x22, x23, [sp], #16//[sp, #16 * 11]
stp	x24, x25, [sp], #16//[sp, #16 * 12]
stp	x26, x27, [sp], #16//[sp, #16 * 13]
stp	x28, x29, [sp], #16//[sp, #16 * 14]

mrs	x22, elr_el1
mrs	x23, spsr_el1
stp	x30, x22, [sp], #16//[sp, #16 * 15] 
str	x23, [sp], #16//[sp, #16 * 16] // we are at sp + 272 // 8 bytes will be empty  //need to be 16 aligned
// st1 needs post-index
st1 {  v0.2d,  v1.2d,  v2.2d,  v3.2d }, [sp], #64//[sp, #(272 + 64 * 0)]
st1 {  v4.2d,  v5.2d,  v6.2d,  v7.2d }, [sp], #64//[sp, #(272 + 64 * 1)]
st1 {  v8.2d,  v9.2d, v10.2d, v11.2d }, [sp], #64//[sp, #(272 + 64 * 2)]
st1 { v12.2d, v13.2d, v14.2d, v15.2d }, [sp], #64//[sp, #(272 + 64 * 3)]
st1 { v16.2d, v17.2d, v18.2d, v19.2d }, [sp], #64//[sp, #(272 + 64 * 4)]
st1 { v20.2d, v21.2d, v22.2d, v23.2d }, [sp], #64//[sp, #(272 + 64 * 5)]
st1 { v24.2d, v25.2d, v26.2d, v27.2d }, [sp], #64//[sp, #(272 + 64 * 6)]
st1 { v28.2d, v29.2d, v30.2d, v31.2d }, [sp], #64//[sp, #(272 + 64 * 7)]
sub	sp, sp, #784
.endm
	
.macro	kernel_exit
ldp	x0, x1, [sp], #16 //[sp, #16 * 0]
ldp	x2, x3, [sp], #16 //[sp, #16 * 1]
ldp	x4, x5, [sp], #16 //[sp, #16 * 2]
ldp	x6, x7, [sp], #16 //[sp, #16 * 3]
ldp	x8, x9, [sp], #16 //[sp, #16 * 4]
ldp	x10, x11, [sp], #16 //[sp, #16 * 5]
ldp	x12, x13, [sp], #16 //[sp, #16 * 6]
ldp	x14, x15, [sp], #16 //[sp, #16 * 7]
ldp	x16, x17, [sp], #16 //[sp, #16 * 8]
ldp	x18, x19, [sp], #16 //[sp, #16 * 9]
ldp	x20, x21, [sp], #16 //[sp, #16 * 10]
ldp	x22, x23, [sp], #16 //[sp, #16 * 11]
ldp	x24, x25, [sp], #16 //[sp, #16 * 12]
ldp	x26, x27, [sp], #16 //[sp, #16 * 13]
ldp	x28, x29, [sp], #16 //[sp, #16 * 14]

ldp	x30, x22, [sp], #16 //[sp, #16 * 15] 
ldr	x23, [sp], #16 //[sp, #16 * 16]
msr	elr_el1, x22			
msr	spsr_el1, x23

ld1 {  v0.2d,  v1.2d,  v2.2d,  v3.2d }, [sp], #64//[sp, #(272 + 64 * 0)]
ld1 {  v4.2d,  v5.2d,  v6.2d,  v7.2d }, [sp], #64//[sp, #(272 + 64 * 1)]
ld1 {  v8.2d,  v9.2d, v10.2d, v11.2d }, [sp], #64//[sp, #(272 + 64 * 2)]
ld1 { v12.2d, v13.2d, v14.2d, v15.2d }, [sp], #64//[sp, #(272 + 64 * 3)]
ld1 { v16.2d, v17.2d, v18.2d, v19.2d }, [sp], #64//[sp, #(272 + 64 * 4)]
ld1 { v20.2d, v21.2d, v22.2d, v23.2d }, [sp], #64//[sp, #(272 + 64 * 5)]
ld1 { v24.2d, v25.2d, v26.2d, v27.2d }, [sp], #64//[sp, #(272 + 64 * 6)]
ld1 { v28.2d, v29.2d, v30.2d, v31.2d }, [sp], #64//[sp, #(272 + 64 * 7)]

//add sp, sp, #784	
eret
.endm


/*
 * Exception vectors.
 */
.align	11
.globl vectors 
vectors:
	ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	ventry	sync_invalid_el1h			// Synchronous EL1h
	ventry	el1_irq					    // IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	ventry	sync_invalid_el0_64			// Synchronous 64-bit EL0
	ventry	irq_invalid_el0_64			// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64	    // Error 64-bit EL0

	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32		// Error 32-bit EL0

sync_invalid_el1t:
	handle_invalid_entry  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry  SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_invalid_entry  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  ERROR_INVALID_EL0_32

el1_irq:
	kernel_entry 
	bl irq_handler
	kernel_exit 

.globl err_hang
err_hang: b err_hang

