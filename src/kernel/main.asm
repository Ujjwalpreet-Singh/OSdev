org 0x7C00
bits 16
%define ENDL 0x0D,0x0A

start:
	jmp main

;Prints a string
;Params
;	-d:si points to string


puts:
	;save registers
	push si
	push ax

.loop:
	lodsb  	;loads next char
	or al,al	;verify next char is not null
	jz .done

	mov ah,0x0e
	mov bh,0
	int 0x10

	jmp .loop

.done:
	pop ax
	pop si
	ret


main:
	;setup segement
	mov ax,0
	mov ds,ax
	mov es,ax

	;setup stack
	mov ss,ax
	mov sp,0x7C00	;pops downwards

	;prints message
	mov si,msg_hello
	call puts

	hlt

.halt:
	jmp .halt

msg_hello: db 'Welcome gang',ENDL,0

times 510-($-$$) db 0
dw 0AA55h
