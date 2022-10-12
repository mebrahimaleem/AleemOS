;MBR.asm

;Master Boot Records for AleemOS

[BITS 16] ;We start in 16-bit mode

[ORG 0x600] ;The MBR will relocate itself to 0x600

;Setup segments & stack
cli
mov bp, 0x7a00
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, ax

;Relocate MBR
mov cx, 0x0100 ;Number of WORDs to copy
mov si, 0x7c00 ;copy source
mov di, 0x0600 ;copy destination
rep movsw

jmp 0:MBR ;Long jump to rest of MBR to ensure CS is set to 0

MBR:
sti
mov BYTE [BOOT_DRIVE], dl
.findActive: ;Find first active partition
	mov bx, PARTITION_TABLE
	mov cx, 4
	.findLoop:
		mov al, BYTE [bx] ;Get partition active flag
		cmp al, 0x80
		je .findExit
		add bx, 0x10 ;Check next partition table entry
		dec cx
		cmp cx, 0
		jne .findLoop
		jmp err ;Jump tp MBR Fatal Error if no partition is active
	.findExit:
		mov WORD [PARTITION_OFFSET], bx ;Store partition table entry offset
		add bx, 8
	
;Read VBR from partition
mov ebx, DWORD [bx] ;Get LBA of partition

;Get drive parameters in case OS was copied to another medium (non 1.44M floppy)
pusha
mov ah, 8
xor di, di
int 0x13
jc err ;Fatal error on fail
and cx, 0x3f
mov [TRACK_SECTORS], cx
movzx dx, dh
add dx, 1
mov [HEADS], dx
popa

mov cx, 0x7c00
call read_sector

;Jump to VBR
cmp WORD [0x7dfe], 0xaa55 ;Check if VBR has boot signature
jne err

;Pass Partition table offset and boot drive to VBR
mov si, WORD [PARTITION_OFFSET]
mov dl, BYTE [BOOT_DRIVE]

;Pass drive parameters
mov dx, [TRACK_SECTORS]
mov cx, [HEADS]

;Jump to VBR
jmp 0x7c00


times 218-($-$$) nop

TIMESTAMP dq 0 ;NOTE: Setting the timestamp in assembly is useless since we write to the disk at a later time. This field should be set after writing to a disk.

BOOT_DRIVE db 0
PARTITION_OFFSET dw 0
TRACK_SECTORS dw 18
HEADS dw 2
FATAL_ERR db "MBR: FATAL ERROR - PROGRAM SUSPENDED", 0

err:
mov si, FATAL_ERR
mov ah, 0x0e
mov bh, 0
mov bl, 0xf0
.print:
	mov al, BYTE [si]
	cmp al, 0
	je .hang
	int 0x10
	inc si
	jmp .print
.hang:
jmp .hang

times 300-($-$$) nop

read_sector: ;Reads the LBA sector stores in bx and copies it to cx
xor dx, dx
mov es, dx
push cx

;Now we read the disk
mov ax, bx

xor dx, dx
div WORD [TRACK_SECTORS]
add dl, 1
mov cl, dl
mov ax, bx

xor dx, dx
div WORD [TRACK_SECTORS]
mov dx, 0
div WORD [HEADS]
mov dh, dl
mov ch, al

mov al, 1
mov dl, BYTE [BOOT_DRIVE]
pop bx
mov ah, 0x02

int 0x13

;Fatal error on fail
jc err

cmp al, 1
jne err

ret

times 434-($-$$) nop

UID_1 dq 0x345 ;Unique disk ID (NOTE: We can't know for sure what the other disk IDs are at compile time so our best bet is to set this to some random number)
UID_2 dd 0xf0


PARTITION_TABLE:
;First Entry
db 0x80 ;Set to bootable
db 0x00 ;Starting head
db 0x02 ;Starting sector (and upper two bits for cylinder)
db 0x00 ;Starting cylinder
db 0x01 ;System ID
db 0x01 ;Ending head
db 0x11 ;Ending sector (and upper two bits for cylinder)
db 0x50 ;Ending cylinder
dd 0x01 ;Partion starting LBA
dd 0x0b3e ;Number of sectors

;Second Entry
dq 0 ;Unused partition
dq 0

;Third Entry
dq 0 ;Unused partition
dq 0

;Fourth Entry
dq 0 ;Unused partition
dq 0

dw 0xaa55 ;Boot signature
