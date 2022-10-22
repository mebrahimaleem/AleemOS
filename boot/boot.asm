;boot.asm

;Bootstrapper for AleemOS

[ORG 0xAC00]

VBR_READ_FILE equ 0x7d00 ;This is the memory location of the VBR readBoot routine, params: bx: where to copy file, si: pointer to name of file

;We have not switched to 32-bit mode yet
[BITS 16]

;Save drive parameters, boot drive, and partion offset
mov BYTE [DRIVE_NO], dl
mov WORD [PARTION_OFFSET], si
mov WORD [TRACK_SECTORS], bx
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

;Load the IDT descripter
lidt [IDT_ptr]

xor ebx, ebx
xor ecx, ecx
xor edx, edx
xor esi, esi

;Pass information to kernel
mov dl, BYTE [DRIVE_NO]
mov si, WORD [PARTION_OFFSET]
mov bx, WORD [TRACK_SECTORS]
mov cx, WORD [HEADS]

mov DWORD [KTRCKPS], ebx
mov DWORD [KHEADS], ecx
mov DWORD [KBOOTNO], edx
mov DWORD [KPARTI], esi

;jump to kernel

KTSS_EIP:
nop
mov ax, KTSS_SEG
ltr ax

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

;IDT
;The IDT will contain 56 entries (each 8 bytes long)
IDT_start:
times 56 dq 0
IDT_end:

;GDT
;The GDT will contain 10 entries
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
	GDT_KTSS:
		dw 104
		dw TSS_start ;NOTE: The TSS is located within the first 0xFFFF bytes, hence all base bits above 15 should be 0
		db 0x00
		db 0b10001001
		db 0b01000000
		db 0x00
	GDT_UTSS:
		dq 0
	GDT_UCod:
		dq 0
	GDT_UDat:
		dq 0
	GDT_r0LD:
		dq 0
	GDT_r1LD:
		dq 0
	GDT_r2LD:
		dq 0
	GDT_end:

TSS_start:
	dw KTSS_SEG 	;Link
	dw 0x0000			;Reserved
	dd 0x9fa00		;ESP 0
	dw DATA_SEG		;SS 0
	dw 0x0000			;Reserved
	dd 0x7c00			;ESP 1
	dw DATA_SEG		;SS 1
	dw 0x0000			;Reserved
	dd 0x7c00			;ESP 2
	dw UDAT_SEG		;SS 2
	dw 0x0000			;Reserved
	dd 0x0000			;CR3
	dd KTSS_EIP		;EIP
	dd 0x46				;EFLAGS
	dd 0x0000			;EAX
KHEADS	dd 0x0	;ECX NOTE:Must be set at runtime
KBOOTNO	dd 0x0	;EDX NOTE:Must be set at runtine
KTRCKPS	dd 0x0	;EBX NOTE:Must be set at runtime
	dd 0x9fa00		;ESP
	dd 0x9fa00		;EBP
KPARTI	dd 0x0	;ESI NOTE:Must be set at runtime
	dd 0x0				;EDI
	dw DATA_SEG		;ES
	dw 0x0				;Reserved
	dw CODE_SEG		;CS
	dw 0x0				;Reserved
	dw DATA_SEG		;SS
	dw 0x0				;Reserved
	dw DATA_SEG		;DS
	dw 0x0				;Reserved
	dw DATA_SEG		;FS
	dw 0x0				;Reserved
	dw DATA_SEG		;GS
	dw 0x0				;Reserved
	dw R0LD_SEG		;LDT Segment Descriptor
	dw 0x0				;Reserved
	dw 0x0				;Reserved + T
	dw IDT_end - IDT_start ;NO IOPB

IDT_ptr: ;This is the descriptor for the IDT
	dw IDT_end - IDT_start -1 ;Size - 1
	dw IDT_start ;Starting address

GDT_ptr: ;This is the descriptor for the GDT
	dw GDT_end - GDT_start - 1 ;Size - 1
	dd GDT_start ;Starting address

;Some useful constants
CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start
KTSS_SEG equ GDT_KTSS - GDT_start
UTSS_SEG equ GDT_UTSS - GDT_start
UCOD_SEG equ GDT_UCod - GDT_start
UDAT_SEG equ GDT_UDat - GDT_start
R0LD_SEG equ GDT_r0LD - GDT_start
R1LD_SEG equ GDT_r1LD - GDT_start
R2LD_SEG equ GDT_r2LD - GDT_start

;Pad out so that BOOT.BIN takes up 4 sectors
times 0x800-($-$$) nop
