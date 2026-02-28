bits 32
section .entry
extern kmain
global _start

_start:
    call kmain

hang:
    cli
    hlt
    jmp hang