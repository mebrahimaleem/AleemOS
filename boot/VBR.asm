;VBR.asm

;Volume Boot Records for AleemOS

;Still in 16-bit
[BITS 16]

;MBR loaded VBR to 0x7c00
[ORG 0x7c00]

MBR_READ_SECTOR equ 0x72C ;This is the memory location of the MBR readsector routine, params: bx: LBA of sector to read, es:cx: where to write sector

;BPB
jmp short copyboot
nop
OEM db "ALEEMOS "
SECTOR_BYTES dw 512
CLUSTER_SECTORS db 1
RESERVED_SECTORS dw 1
FATS db 2
ROOT_ENTRIES dw 224 ;14 sectors
SECTORS dw 0x0b3a
MEDIA_BYTE db 0xF0
FAT_SECTORS dw 9
TRACK_SECTORS dw 18
HEADS dw 2
HIDDEN_SECTORS dd 1
LARGE_SECTOR dd 0

;Extended Boot Record
DRIVE_NO db 0 ;NOTE: We don't know this at compile time
NT_FLAGS db 0
SIGNATURE db 41
VOLUME_ID dd 16
VOLUME_LABEL db "ALEEMOS    "
FS_ID db "FAT12   "

copyboot:

mov BYTE [DRIVE_NO], dl
mov WORD [PARTION_OFFSET], si
mov WORD [TRACK_SECTORS], dx
mov WORD [HEADS], cx

;First we copy FAT1 onto the RAM

copyFAT:
	mov bx, 2 ;1 hidden sector + 1 reserved sector = 2
	.loop:
		mov ax, 512
		mul bx
		add ax, 0x7A00
		mov cx, ax
		pusha
		call MBR_READ_SECTOR
		popa
		inc bx
		cmp bx, 11
		jne .loop

;Now we copy the Root Directory from the floppy to RAM
copyRootDir:
	mov bx, 20 ;hidden sector + VBR + FAT1 + FAT2 = 20
	.loop:
		mov ax, 512
		mul bx
		add ax, 0x6800
		mov cx, ax
		pusha
		call MBR_READ_SECTOR
		popa
		inc bx
		cmp bx, 34
		jne .loop

;Some useful constants
FAT1 equ 0x7E00
ROOT_DIR equ 0x9000
FSDATA equ 0x20 ;NOTE: not the start of the data area (in sectors), this is set such that cluster 2 is the start of the data area (since 0 and 1 are reserved)

;We will now search the root directory for a file called BOOT with extension BIN
findBoot:
	mov ax, ROOT_DIR
	.loop:
		mov di, ax
		mov si, BOOT_FNAME
		mov cx, 11
		rep cmpsb
		je readBoot
		add ax, 32 ;Go to next entry
		jmp .loop
jmp $
readBoot:
	add ax, 26 ;Point to starting cluster
	mov bx, ax
	mov bx, word [bx] ;Get starting cluster
	mov dx, bx
	mov cx, 0xB000 ;Where to copy start of boot
	call readCluster ;Read cluster bx
	
	call findCluster ;Get the next cluster
	.loop:
		cmp bx, 0x0FFF ;Check if last cluster
		je 0xB000 ;If so, go to boot
		add cx, 512 ;Copy to next cluster

		mov dx, bx
		call readCluster ;Read the next cluster
		
		call findCluster ;Get the next cluster
		jmp .loop
		
		
readCluster: ;Copies cluster bx to address es:cx
	add bx, FSDATA
	pusha
	call MBR_READ_SECTOR
	popa
	ret

findCluster: ;Gets a cluster from index dx and returns in bx
	mov bx, dx
	and bx, 1
	push bx ;If odd, bx is 1
	
	mov bx, 3
	mov ax, dx
	mul bx
	xor dx, dx
	mov bx, 2
	div bx
		
	add ax, FAT1
	pop bx
	cmp bx, 1
	je .odd

	;Even
	mov bx, ax
	mov dx, [bx]
	and dx, 0x0FFF
	mov bx, dx
	ret

	.odd:
	mov bx, ax
	mov ax, [bx]
	mov dx, [bx+1]
	shl dx, 4
	shr ax, 4
	and dx, 0x0F00
	and ax, 0x00FF
	add dx, ax
	mov bx, dx
	ret

PARTION_OFFSET dw 0

BOOT_FNAME db "BOOT    BIN"

times 510-($-$$) nop
dw 0xaa55
