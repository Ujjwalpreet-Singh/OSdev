[bits 32]
global jump_to_kernel

jump_to_kernel:

    cli

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, [esp+8]    ; stack argument
    mov eax, [esp+4]    ; entry argument

    jmp eax