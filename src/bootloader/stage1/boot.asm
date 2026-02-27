org 0x7C00
bits 16
%define ENDL 0x0D,0x0A


;
;	FAT12 header
;
jmp short start
nop

bdb_oem:						db 'MSWIN4.1'	;8bytes
bdb_bytes_per_sector:        dw 512
bdb_sectors_per_cluster:     db 4
bdb_reserved_sectors:        dw 4
bdb_fat_count:               db 2
bdb_dir_entries_count:       dw 512
bdb_total_sectors:           dw 0
bdb_media_descriptor_type:   db 0F8h
bdb_sectors_per_fat:         dw 64
bdb_sectors_per_track:       dw 32
bdb_heads:                   dw 4
bdb_hidden_sectors:          dd 0
bdb_large_sector_count:      dd 65536


; extended boot record

ebr_drive_number:				db 0 				;0x00 floppy,0x80 hdd
								db 0  				;reserved
ebr_signature:					db 29h
ebr_volume_id:					db 12h,34h,56h,78h	;serial no.
ebr_volume_label:				db 'GOOS       ' 	;11 bytes
ebr_system_id:					db 'FAT16  '		;8 bytes


start:

	;setup segement
	mov ax,0
	mov ds,ax
	mov es,ax

	;setup stack
	mov ss,ax
	mov sp,0x7C00	;pops downwards

	push es
	push word .after
	retf

.after:



	;Read from disk
	; BIOS should set drive no. into DL

	mov [ebr_drive_number],dl



	;prints message
	mov si,msg_hello
	call puts

	;read drive params
	push es
	mov ah, 08h
	int 13h
	jc floppy_error
	pop es

	and cl, 0x3F						;remove top 2 bits
	xor ch,ch
	mov [bdb_sectors_per_track],cx		; sector count

	inc dh
	mov [bdb_heads], dh					;head count

	;read FAT root dir. (LBA of root = reserved + fats*sectors per fat)
	mov ax, [bdb_sectors_per_fat]
	mov bl,[bdb_fat_count]
	xor bh,bh
	mul bx								; ax = (fats * sectors_per_fat)
	add ax,[bdb_reserved_sectors]
	push ax

	;compute size of root dir. = (32*no. of entries)/bytes per sector
	mov ax,[bdb_dir_entries_count]
	shl ax,5						; ax *= 32
	xor dx,dx						; dx = 0
	div word [bdb_bytes_per_sector]	;no. of sectors to read

	test dx,dx						;if dx != 0, add 1
	jz .root_dir_after
	inc ax

.root_dir_after:

	; read root dir.
	mov cl,al						;cl = no. of sectors to read = size of root dir.
	pop ax							;ax = LBA of root dir.
	mov dl,[ebr_drive_number]		;dl = drive no.
	mov bx,buffer
	call disk_read

	;search for stage2.bin

	xor bx,bx
	mov di,buffer

.search_stage2:

	mov si,file_stage2_bin
	mov cx,11
	push di
	repe cmpsb
	pop di
	je .found_stage2

	add di,32
	inc bx
	cmp bx,[bdb_dir_entries_count]
	jl .search_stage2

	;stage2 not found
	jmp stage2_not_found_error

.found_stage2:
	; di should have the address to the entry
	mov ax, [di+26]				;first logical cluster field (offset 26)
	mov [stage2_cluster],ax

	; load FAT from fisk into memory
	mov ax,[bdb_reserved_sectors]
	mov bx,buffer
	mov cl ,[bdb_sectors_per_fat]
	mov dl, [ebr_drive_number]
	call disk_read

	; read stage2 and process FAT chain
	mov bx, STAGE2_LOAD_SEGMENT
	mov es,bx
	mov bx,STAGE2_LOAD_OFFSET

.load_stage2_loop:

	;Read next cluster
	mov ax,[stage2_cluster]
	sub ax,2

	mov cl,[bdb_sectors_per_cluster]
	xor ch,ch
	mul cx

	add ax,164					;first cluster = [stage2_cluster-2]*sectors_per_clluster + start_sector
								;start_sector = reserved + fats + root dir size = 1+18+134 = 33
	mov dl,[ebr_drive_number]
	call disk_read

	mov ax,[bdb_sectors_per_cluster]
	mul word [bdb_bytes_per_sector]
	add bx,ax
	
	mov ax,[stage2_cluster]
	shl ax,1
	mov si,buffer
	add si,ax
	mov ax,[si]



.next_cluster_after:
	cmp ax,0x0FF8					;end of chain
	jae .read_finish

	mov [stage2_cluster],ax
	jmp .load_stage2_loop

.read_finish:
	;boot device in dl
	mov dl,[ebr_drive_number]
	;set segment registers
	mov ax,STAGE2_LOAD_SEGMENT
	mov ds,ax
	mov es,ax

	jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

	jmp wait_key_and_reboot

	cli
	hlt


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

	
floppy_error:
	mov si,msg_read_failed
	call puts
	jmp wait_key_and_reboot

stage2_not_found_error:

	mov si,msg_stage2_not_found
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
msg_read_failed: db 'Read from disk failed!',ENDL,0
msg_stage2_not_found: db 'STAGE2.BIN not found',ENDL,0
file_stage2_bin: db 'STAGE2  BIN'
stage2_cluster: dw 0
STAGE2_LOAD_SEGMENT equ 0x0
STAGE2_LOAD_OFFSET	equ 0x500
times 510-($-$$) db 0
dw 0AA55h

buffer: