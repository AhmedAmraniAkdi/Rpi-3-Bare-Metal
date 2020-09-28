// the way i understand it is that this is equivalent to volatile,
// no optimization, but better because we might forget the volatile word

.globl PUT32
PUT32:
    str w1,[x0]
    ret

.globl PUT8
PUT8:
    strb w1,[x0]
    ret

.globl GET32
GET32:
    ldr w0,[x0]
    ret

.globl GETPC
GETPC:
    mov x0,x30
    ret

.globl BRANCHTO
BRANCHTO:
    mov w30,w0
    ret

.globl dummy
dummy:
    ret

.globl CYCLE_DELAY
CYCLE_DELAY:
    subs x0, x0, #1
    bne CYCLE_DELAY
    ret

.globl BRANCHTO
BRANCHTO:
    mov w30,w0
    ret
