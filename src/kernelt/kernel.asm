[bits 32]
global _start
extern kmain

_start:

    cli

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x200000

    call kmain

.hang:
    hlt
    jmp .hang