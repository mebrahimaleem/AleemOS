;VBR.asm

;Volume Boot Records for AleemOS

;Still in 16-bit
[BITS 16]

;MBR loaded VBR to 0x7c00
[ORG 0x7c00]

MBR_READ_SECTOR equ 0x72C ;This is the memory location of the MBR readsector routine, params: bx: LBA of sector to read, es:cx: where to write sector
MBR_ERR equ 0x600 + 226 ; This is the memory location of the MBR print error and hang routine, params: si: pointer to the error message null terminated string

;BPB
jmp short copyboot
nop
OEM db "ALEEMOS " ;See the FAT12 filesystem documentation for more information on this datastructure
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
DRIVE_NO db 0 
NT_FLAGS db 0
SIGNATURE db 41
VOLUME_ID dd 16
VOLUME_LABEL db "ALEEMOS    "
FS_ID db "FAT12   "

copyboot:

;BIOS is supposed to set BPB, but some (from previous experiences) don't
mov BYTE [DRIVE_NO], dl
mov WORD [PARTION_OFFSET], si
mov WORD [TRACK_SECTORS], bx
mov WORD [HEADS], cx

;First we copy FAT1 onto the RAM, we don't need FAT2 since it is rarely used for disk recovery

copyFAT:
	mov bx, 2 ;1 hidden sector + 1 reserved sector = 2
	.loop:
		mov ax, 512 ;First calculate where to copy sector
		mul bx
		add ax, 0x7A00
		mov cx, ax ;
		pusha
		call MBR_READ_SECTOR ;Copy to 0x7A00 + 512 * LBA sector on disk
		popa
		inc bx ;Read next sector
		cmp bx, 11 ;Check if last sector
		jne .loop

;Now we copy the Root Directory from the floppy to RAM
copyRootDir:
	mov bx, 20 ;hidden sector + VBR + FAT1 + FAT2 = 20
	.loop:
		mov ax, 512 ;Calculate where to copy sector
		mul bx
		add ax, 0x6800
		mov cx, ax
		pusha
		call MBR_READ_SECTOR ;Similar logic to before, copy to 0x6800 + 512 * LBA sector on disk
		popa
		inc bx ;Next sector
		cmp bx, 34 ;Check if last sector
		jne .loop

;Some useful constants
FAT1 equ 0x7E00 ;Start of the filesystem on RAM
ROOT_DIR equ 0x9000 ;Start of the RootDirectory on RAM
FSDATA equ 0x20 ;NOTE: not the start of the data area (in sectors), this is set such that cluster 2 is the start of the data area (since 0 and 1 are reserved)

mov si, BOOT_FNAME ;The file to copy
mov cx, 0xAC00 ;Where to copy start of boot
call findBoot

mov dl, BYTE [DRIVE_NO] ;Pass important drive parameters to the remainaing bootloader
mov si, WORD [PARTION_OFFSET]
mov bx, WORD [TRACK_SECTORS]
mov cx, WORD [HEADS]
jmp 0xAC00 ;Address of the bootloader

FILE_NOT_FOUND db "FATAL: FILE NOT FOUND - Get new copy of OS", 0

times 0x100-($-$$) nop ;Pad out so that we know the exact memory location of this function (so that it can later be used)
;We will now search the root directory for a file called BOOT with extension BIN
findBoot: ;Find the file pointed by si and copy it to cx
	mov ax, ROOT_DIR ;Algorithm: check everyroot directory until we find a valid file
	push cx
	push si
	.loop:
		mov di, ax
		pop si
		push si
		mov cx, 11
		rep cmpsb
		je readBoot
		add ax, 32 ;Go to next entry
		cmp ax, ROOT_DIR + (32 * 224)
		mov si, FILE_NOT_FOUND
		je MBR_ERR
		jmp .loop
jmp $
readBoot:
	pop si
	add ax, 26 ;Point to starting cluster
	mov bx, ax
	mov bx, word [bx] ;Get starting cluster
	mov dx, bx
	pop cx
	call readCluster ;Read cluster bx
	
	call findCluster ;Get the next cluster
	.loop:
		cmp bx, 0x0FFF ;Check if last cluster
		je bootjmp ;If so, go to boot
		add cx, 512 ;Copy to next cluster

		mov dx, bx
		call readCluster ;Read the next cluster
		
		call findCluster ;Get the next cluster
		jmp .loop

bootjmp:
	ret
				
readCluster: ;Copies cluster bx to address es:cx
	add bx, FSDATA ;Add data area offset
	pusha
	call MBR_READ_SECTOR ;Read cluster
	popa
	ret

findCluster: ;Gets a cluster from index dx and returns in bx
	mov bx, dx ;save dx
	and bx, 1
	push bx ;If odd, bx is 1
	
	;Multiply dx by 1.5 (floored) and put it in ax
	mov bx, 3
	mov ax, dx
	mul bx
	xor dx, dx
	mov bx, 2
	div bx
		
	;Add FAT offset to ax
	add ax, FAT1
	pop bx
	cmp bx, 1
	je .odd ;If odd, we handle this differently

	;Even
	;This is is the simple situation, all we need is the first 1.5 bytes of the word pointed by ax
	mov bx, ax
	mov dx, [bx]
	and dx, 0x0FFF ;Mask out high 4 bits (since they belong to another entry)
	mov bx, dx
	ret

	.odd:
	;This is more complex, we need to add the values pointed by ax and ax+1 while masking out adjacent bytes (since we only want 1.5 bytes)
	mov bx, ax
	mov ax, [bx] ;First 2 thirds of entry
	mov dx, [bx+1] ;Other third of entry
	shl dx, 4 ;Mask out the other entries
	shr ax, 4
	and dx, 0x0F00
	and ax, 0x00FF
	or dx, ax ;Combine the entry
	mov bx, dx
	ret

PARTION_OFFSET dw 0

BOOT_FNAME db "BOOT    BIN" ;This is the name of the file we are searching for

times 510-($-$$) nop ;Pad out remaining sector
dw 0xaa55
