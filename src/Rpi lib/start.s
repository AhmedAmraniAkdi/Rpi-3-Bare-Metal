.section ".text.boot"

.globl _start
_start:
    mov sp,#0x2000000
    bl notmain
hang: b hang