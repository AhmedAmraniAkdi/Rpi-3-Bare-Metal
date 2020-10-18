.globl _start
_start:
    b skip
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang

skip:
    mov sp,#0x08000000
    bl notmain
hang: b hang
