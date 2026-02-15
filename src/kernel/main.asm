org 0x0
bits 16
%define ENDL 0x0D,0x0A

start:

	;prints message
	mov si,msg_hello
	call puts

.halt:
	cli
	hlt


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

msg_hello: db "Welcome gang, we're in kernel now",ENDL,0