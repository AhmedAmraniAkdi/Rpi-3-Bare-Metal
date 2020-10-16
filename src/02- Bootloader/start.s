//.section ".text.boot"


.globl _start
_start:
    b skip
.space 0x280004-0x80004,0 
skip:
    mov sp,#0x10000000
    bl notmain
hang: b hang