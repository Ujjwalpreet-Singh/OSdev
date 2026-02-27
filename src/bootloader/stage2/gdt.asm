[bits 32]

global GDT_Load
GDT_Load:

    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lgdt [eax]

    mov ax, [ebp + 16]     ; save data selector NOW
    mov dx, ax             ; store in DX temporarily

    mov ax, [ebp + 12]     ; code selector
    push eax
    push .reload_cs
    retf

.reload_cs:

    mov ax, dx             ; restore correct data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, ebp
    pop ebp
    ret