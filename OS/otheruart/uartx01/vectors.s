
.globl _start
_start:
    mov sp,#0x80000
    //bl GETPC
    bl notmain
hang: b hang

.globl PUT32
PUT32:
  str w1,[x0]
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



