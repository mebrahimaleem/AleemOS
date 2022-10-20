;boot.asm

;Bootstrapper for AleemOS

[ORG 0xAC00]

VBR_READ_FILE equ 0x7d00 ;This is the memory location of the VBR readBoot routine, params: bx: where to copy file, si: pointer to name of file

;We have not switched to 32-bit mode yet
[BITS 16]

;Save drive parameters, boot drive, and partion offset
mov BYTE [DRIVE_NO], dl
mov WORD [PARTION_OFFSET], si
mov WORD [TRACK_SECTORS], dx
mov WORD [HEADS], cx

;Set the VGA video mode
mov ah, 0x00
mov al, 0x03
int 0x10 

;Hide Cursor
mov ah, 0x01
mov cx, 0x2607
int 0x10

;Notify user to please wait
mov si, BOOT_START_STR
call print

;Now we need to copy the kernel onto the RAM
mov si, KERNEL_FNAME ;file to copy
mov cx, 0xB400 ;Where to copy kernel
call VBR_READ_FILE

;Now we need to enter PM
cli ;We will disable interupts until the kernel has correctly set up the IDT
lgdt [GDT_ptr] ;Load the GDT descriptor

mov eax, cr0 ;Set the PM bit
or eax, 0x01
mov cr0, eax
jmp CODE_SEG:enter_pm ;Far jump to 32-bit (to set CS)

;We are now in 32-bit mode (but we stil need to set the segments registers)
[BITS 32]
enter_pm:
;Set the segment registers
mov ax, DATA_SEG
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

;Set the stack
mov ebp, 0x9fa00
mov esp, ebp

;NOTE: We can't set the LDT just yet because we need an IDT first (otherwise CPU will triple fault)

;Load the IDT descripter
lidt [IDT_ptr]


;Pass information to kernel
mov dl, BYTE [DRIVE_NO]
mov si, WORD [PARTION_OFFSET]
mov dx, WORD [TRACK_SECTORS]
mov cx, WORD [HEADS]

;jump to kernel
jmp 0xB400

;Bootloader print routine - prints a null terminated string pointed by si. Modifies si
print:
push ax
push bx
mov bh, 0
mov bl, 0xF0
mov ah, 0x0E
.loop:
lodsb
cmp al, 0
je .exit
int 0x10
jmp .loop

.exit:
pop bx
pop ax
ret


DRIVE_NO db 0 ;Our boot drive number
PARTION_OFFSET dw 0 ;Offset of active partion in the partion table
TRACK_SECTORS dw 0 ;Number of sectors per track
HEADS dw 0 ;Number of heads

BOOT_START_STR db "AleemOS - Please Wait", 0x0D, 0x0A, 0x00

KERNEL_FNAME db "KERNEL  BIN"

;We will put the GDT & IDT in the 4th sector of the bootloader
times 0x600-($-$$) nop

;IDT
;All IDT entires need to be tied to an interupt handler (which will be implemented by the kernel) so we will just leave the IDT blank for now
;The IDT will contain 34 entries (each 8 bytes long)
IDT_start:
times 40 dq 0
IDT_end:

;GDT
;The GDT will contain 3 entries (1 null, 1 code, 1 data)
GDT_start:
	GDT_null:
		dq 0
	GDT_code:
		dw 0xFFFF			;Limit 0-16
		dw 0x0000			;Base 0-16
		db 0x00				;Base 16-24
		db 10011010b	;Flags + Type
		db 11001111b	;Flags + Limit 16-20
		db 0x00				;Base 24-32
	GDT_data:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 10010010b
		db 11001111b
		db 0x00
	GDT_end:

IDT_ptr: ;This is the descriptor for the IDT
	dw IDT_end - IDT_start -1 ;Size - 1
	dw IDT_start ;Starting address

GDT_ptr: ;This is the descriptor for the GDT
	dw GDT_end - GDT_start - 1 ;Size - 1
	dd GDT_start ;Starting address

;Some useful constants
CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

;Pad out so that BOOT.BIN takes up 4 sectors
times 0x800-($-$$) nop
