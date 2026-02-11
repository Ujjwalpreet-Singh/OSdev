org 0x7C00
bits 16
%define ENDL 0x0D,0x0A


;
;	FAT12 header
;
jmp short start
nop

bdb_oem:						db 'MSWIN4.1'	;8bytes
bdb_bytes_per_sector:			dw 512
bdb_sectors_per_cluster:		db 1
bdb_reserved_sectors:			dw 1
bdb_fat_count:					db 2
bdb_dir_entries_count:			dw 0E0h
bdb_total_sectors:				dw 2880			;2880*512 = 1.44MB
bdb_media_descriptor_type:		db 0F0h
bdb_sectors_per_fat:			dw 9
bdb_sectors_per_track:			dw 18
bdb_heads:						dw 2
bdb_hidden_sectors:				dd 0
bdb_large_sector_count:			dd 0


; extended boot record

ebr_drive_number:				db 0 				;0x00 floppy,0x80 hdd
								db 0  				;reserved
ebr_signature:					db 29h
ebr_volume_id:					db 12h,34h,56h,78h	;serial no.
ebr_volume_label:				db 'GOOS       ' 	;11 bytes
ebr_system_id:					db 'FAT12   '		;8 bytes


start:
	jmp main

;Prints a string
;Params
;	-d:si points to string


puts:
	;save registers
	push si
	push ax
	push bx

.loop:
	lodsb  	;loads next char
	or al,al	;verify next char is not null
	jz .done

	mov ah,0x0e
	mov bh,0
	int 0x10

	jmp .loop

.done:
	pop bx
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


	;Read from disk
	; BIOS should set drive no. into DL

	mov [ebr_drive_number],dl

	mov ax,1
	mov cl,1
	mov bx,0x7E00
	call disk_read

	;prints message
	mov si,msg_hello
	call puts

	cli
	hlt

	
floppy_error:
	mov si,msg_read_failed
	call puts
	jmp wait_key_and_reboot

wait_key_and_reboot:
	mov ah,0
	int 16h
	jmp 0FFFFh:0
	
	
.halt:
	cli
	hlt

;
; Disk stuff
;

;
; Converts an LBA addr. to a CHS addr.
; Parameters:
;	-ax: LBA addr.
; Returns:
;	-cx [bits0-5]: sector no.	
;	-cx [bits 6-15]: cylinder
;	-dh : head
;

lba_to_chs:

	push ax
	push dx
	
	xor dx,dx		;dx = 0
	div word [bdb_sectors_per_track] ;ax = LBA % sectorspertrack
									 ;dx = LBA%sectorspertrack		
	inc dx							 ;dx = dx + 1 = sector
	mov cx,dx
	
	xor dx,dx						;dx = 0
	div word [bdb_heads]			;ax = ax/heads = clyinder
									;dx = ax%heads = head

	mov dh,dl						;dh = head
	mov ch,al						;ch = cylinder (lower 8 bits)
	shl ah, 6
	or cl,ah						; put upper 2 bits of cylinder in cl


	pop ax
	mov dl,al						;Restore DL
	pop ax
	ret


;
;Reads sectors from a disk
;Parameters:
;	-ax: LBA addr.
;	-cl: no. of sectors to read (ipyo 128)
;	-dl: drive no.
;	-es:bx: mem. addr. where to store read data

disk_read:

	push ax					;Save registers
	push bx
	push cx
	push dx
	push di

	
	push cx					; store CL
	call lba_to_chs			;compute CHS
	pop ax					; AL = no. of sectors to read
	mov ah,02h
	mov di,3  				;rety count

.retry:
	pusha					;save all registers
	stc						;set carry flag
	int 13h					;carry flag cleared = success
	jnc .done

	;read failed
	popa
	call disk_reset

	dec di
	test di,di
	jnz .retry

.fail:
	;all attempts are exhausted
	jmp floppy_error

.done:
	popa

	pop di
	pop dx
	pop cx
	pop bx
	pop ax					;Restore registers

	ret


;Disk reset
; Parameters: dl - drive no.
disk_reset:
	pusha
	mov ah,0
	stc
	int 13h
	jc floppy_error
	popa
	ret


msg_hello: db 'Welcome gang',ENDL,0
msg_read_failed: db "Read from disk failed!",ENDL,0
times 510-($-$$) db 0
dw 0AA55h
