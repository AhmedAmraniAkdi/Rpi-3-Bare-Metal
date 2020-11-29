// page 197 of programmer guide armv8
// I don't understand fully so i'll keep it the more general possible

// full system any-any

// data syncronisation barrier: all memory accesses before the barrier are done before the ones after the barrier
// also stops execution of instruction until the memory accesses before the barrier are done
// superset of dmb
// used on example in page 8090 (yes eight thousand and ninety) of ARM Architecture Reference Manual - ARMv8
// for mailboxes (I saw we need these)
.globl DSB
DSB:
    DSB SY
    ret

// just ensures the correct ordering of memory accesses before and after the barrier
.globl DMB
DMB:
    DMB SY
    ret

// for when you need to synchronise the new configuration of system registers and context changing stuff
// invalidates all fetched instructions after barrier and refeteches when the instructions before the barrier are done
.globl ISB
ISB:
    ISB SY
    ret
