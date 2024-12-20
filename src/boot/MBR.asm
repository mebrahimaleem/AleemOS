;MBR.asm

;Master Boot Records for AleemOS

[BITS 16] ;We start in 16-bit mode

[ORG 0x600] ;The MBR will relocate itself to 0x600

;Setup segments & stack
cli ;Disable interupts while we modify the stack
mov bp, 0x7a00 ;Stack base pointer
xor ax, ax
mov ds, ax ;Data segment
mov es, ax ;Extra segment
mov ss, ax ;Stack segment
mov sp, bp ;Stack pointer

;Relocate MBR
mov cx, 0x0100 ;Number of WORDs to copy
mov si, 0x7c00 ;copy source
mov di, 0x0600 ;copy destination
rep movsw ;Copy MBR

jmp 0:MBR ;Long jump to rest of MBR to ensure CS is set to 0

times 32-($-$$) nop
dd 0 ;This is the signature (later set by boot.bin) that the kernel looks for

MBR:
sti ;Restore interupts
mov BYTE [BOOT_DRIVE], dl ;Store boot drove
.findActive: ;Find first active partition
	;The algorithm: Store the current parition table entry in bx, then see if the parition active flag is set, if so this is the paritition to use
	mov bx, PARTITION_TABLE ;First partition table entry
	mov cx, 4 ;Number of partition table entries
	.findLoop:
		mov al, BYTE [bx] ;Get partition active flag
		cmp al, 0x80
		je .findExit ;If we found the partition then move on
		add bx, 0x10 ;Check next partition table entry
		dec cx ;Decrease entries left
		cmp cx, 0 ;Check if no more entries
		jne .findLoop
		mov si, BAD_DISK_ERR
		jmp err ;Jump to MBR, Fatal Error if no partition is active
	.findExit:
		mov WORD [PARTITION_OFFSET], bx ;Store partition table entry offset
		add bx, 8
	
;Read VBR from partition
mov ebx, DWORD [bx] ;Get LBA of partition

push bx

mov ah, 8
xor di, di
mov dl, [BOOT_DRIVE]
clc
int 0x13
mov si, DISK_ERR ;If we can't get drive parameters then fatal error
jc err ;Fatal error on fail
and cx, 0x3f
mov [TRACK_SECTORS], cx ;Store sectors per track
movzx dx, dh
add dx, 1
mov [HEADS], dx ;Store heads

xor ebx, ebx
pop bx ;LBA to read
mov ecx, 0x7c00 ;Destination
xor dx, dx
mov es, dx
call read_sector

;Jump to VBR
cmp WORD [0x7dfe], 0xaa55 ;Check if VBR has boot signature
mov si, BAD_DISK_ERR
jne err

;Pass Partition table offset and boot drive to VBR
mov si, WORD [PARTITION_OFFSET]
mov dl, BYTE [BOOT_DRIVE]

;Pass drive parameters
mov bx, [TRACK_SECTORS]
mov cx, [HEADS]

;Jump to VBR
jmp 0x7c00

DISK_ERR db "FATAL: DISK FAIL - Please Restart Your Device", 0
times 218-($-$$) nop

TIMESTAMP dq 0 ;NOTE: Setting the timestamp in assembly is useless since we write to the disk at a later time. This field should be set after writing to a disk.

;Memory address 218 + 8 + 0x600 = 0x600 + 226
err: ;Handle MBR error
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

BOOT_DRIVE db 0
PARTITION_OFFSET dw 0
TRACK_SECTORS dw 18
HEADS dw 2

BAD_DISK_ERR db "FATAL: BAD DISK - Get new copy of OS", 0

times 300-($-$$) nop

read_sector: ;Reads the LBA sector stored in ebx and copies it to es:cx
mov dl, BYTE [BOOT_DRIVE]
mov si, SIZE
mov ah, 0x42
mov DWORD [BUF], ecx
mov DWORD [BLN], ebx

clc
int 0x13

cmp ah, 0
jne derr

ret

derr:
mov si, DISK_ERR
jmp err


; Disk Address Packet
SIZE db 0x10
RES0 db 0
TRN dw 1
BUF dd 0
BLN dq 0

push di
mov di, 3


times 434-($-$$) nop

UID_1 dq 0x345
UID_2 dd 0xf0


PARTITION_TABLE:
;First Entry
db 0x80 ;Set to bootable
db 0x00 ;Starting head
db 0x02 ;Starting sector (and upper two bits for cylinder)
db 0x00 ;Starting cylinder
db 0x0c ;System ID
db 0x03 ;Ending head
db 0xd0 ;Ending track (and upper two bits for cylinder)
db 0xff ;Ending cylinder
dd 0x01 ;Partion starting LBA
dd 0x103FFE ;Number of sectors

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
