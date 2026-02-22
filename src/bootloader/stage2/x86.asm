bits 32
section .text

global x86_Video_WriteCharTeletype
global x86_Video_ClearScreen


; ---------------------------------------
; void x86_Video_WriteCharTeletype(char c, uint8_t page)
; ---------------------------------------
x86_Video_WriteCharTeletype:

    push ebp
    mov ebp, esp
    push eax
    push edi
    push edx

    ; [ebp + 8]  = character
    ; [ebp + 12] = page (ignored)

    mov al, byte [ebp + 8]

    ; Load current cursor position
    mov edi, [cursor_pos]

    ; VGA base
    mov edx, 0xB8000
    add edx, edi

    ; Attribute = white on blue
    mov ah, 0x1F

    ; Write character + attribute
    mov [edx], ax

    ; Advance cursor (2 bytes per cell)
    add edi, 2
    mov [cursor_pos], edi

    pop edx
    pop edi
    pop eax
    pop ebp
    ret


; ---------------------------------------
; void x86_Video_ClearScreen()
; ---------------------------------------
x86_Video_ClearScreen:

    push eax
    push ecx
    push edi

    mov edi, 0xB8000        ; VGA base
    mov ecx, 80 * 25        ; 2000 cells
    mov ax, 0x1F20          ; 0x20 = space, 0x1F = white on blue

    cld
    rep stosw               ; fill screen

    mov dword [cursor_pos], 0  ; reset cursor

    pop edi
    pop ecx
    pop eax
    ret


section .data
cursor_pos dd 0