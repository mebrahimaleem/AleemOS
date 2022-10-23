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
call real_print

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

;Now we need to remap the PIC (master and slave)
PIC1 equ 0x20
PIC2 equ 0xA0

PIC1_CMD equ 0x20
PIC1_DAT equ 0x21

PIC2_CMD equ 0xA0
PIC2_DAT equ 0xA1

MASTER_OFFSET equ 0x20
SLAVE_OFFSET equ 0x28

ICW1_ICW4 equ 0x1
ICW1_CASCADE equ 0x2
ICW1_INTERVAL equ 0x4
ICW1_LEVEL equ 0x08
ICW1_INIT equ 0x10

ICW4_8086 equ 0x1
ICW4_AUTO equ 0x2
ICW4_BUFS equ 0x8
ICW4_BUFM equ 0xC
ICW4_SFNM equ 0x10

;First, we need to get the current masks for PIC1 and PIC2, and store them in al and ah, respectively
in al, PIC1_DAT
mov bl, al

in al, PIC2_DAT
mov bh, al

;Next we initilize in cascade mode
mov al, (ICW1_INIT | ICW1_ICW4)
out PIC1_CMD, al

xor al, al
out 0x80, al ;wait

mov al, (ICW1_INIT | ICW1_ICW4)
out PIC2_CMD, al

xor al, al
out 0x80, al ;wait

;Set offsets
mov al, MASTER_OFFSET
out PIC1_DAT, al

xor al, al
out 0x80, al ;wait

mov al, SLAVE_OFFSET
out PIC2_DAT, al

xor al, al
out 0x80, al ;wait

;Tell master slave's location and give cascade ID
mov al, 4
out PIC1_DAT, al

xor al, al
out 0x80, al

mov al, 2
out PIC2_DAT, al

xor al, al
out 0x80, al

;Set 8086 mode
mov al, ICW4_8086
out PIC1_DAT, al

xor al, al
out 0x80, al

mov al, ICW4_8086
out PIC2_DAT, al

xor al, al
out 0x80, al

;Restore masks
mov al, bl
out PIC1_DAT, al

mov al, bh
out PIC2_DAT, al

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

;Make sure we correctly set our TSS
mov DWORD [KTRCKPS], ebx
mov DWORD [KHEADS], ecx
mov DWORD [KBOOTNO], edx
mov DWORD [KPARTI], esi

;jump to kernel

;Set the LDT
mov ax, R0LD_SEG
lldt ax

;Set segments
mov ax, L0DT_SEG
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

sti

KTSS_EIP: ;Our EIP for the TSS
nop
mov ax, KTSS_SEG ;Set the TSS
ltr ax

;Far jump to kernel
jmp L0CD_SEG:0xB400

;Bootloader real_print routine - prints a null terminated string pointed by si. Modifies si
real_print:
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

;Bootloader pm_print routine - prints a null terminated string pointed by si without escape codes. Modifies si
pm_print:
push ebx
push cx
mov ebx, [VGA_CURSOR]
.loop:
mov cl, [si]
cmp cl, 0
je .exit
mov [ebx], cl
add ebx, 1
mov BYTE [ebx], 0x0F
add ebx, 1
add si, 1
jmp .loop

.exit:
mov [VGA_CURSOR], ebx
pop cx
pop ebx
ret

;Current cursor position on VGA
VGA_CURSOR dd (0xB8000+160)

DRIVE_NO db 0 ;Our boot drive number
PARTION_OFFSET dw 0 ;Offset of active partion in the partion table
TRACK_SECTORS dw 0 ;Number of sectors per track
HEADS dw 0 ;Number of heads

BOOT_START_STR db "AleemOS - Please Wait", 0x0D, 0x0A, 0x00

KERNEL_FNAME db "KERNEL  BIN"

;IDT
;The IDT will contain 56 entries (each 8 bytes long)
;This IDT is for ring 0 interupts, the kernel will establish an IDT for rings 1 and 2
;We set the IDT in the bootloader so that the kernel can immediatly set up drivers and work with interupts enabled
;Each interupt gate 0-32 will redirect to an ISR that will either (1) print the interupt name if it is fatal and restart the kernel, or (2) ignore th interupt
;The ring 0 IDT ignores certain interupts since the kernel is not meant to be halted in the event of a debug or breakpoint exception
;All IRQ handling will function as a trap

;This macro will create an interupt gate that runs the ISR pointed by the first parameter
;Because all ISR all located within the first 0xFFFF bytes, only the first 16 bits in the offset are used

%macro ISR_INT 1
dw %1
dw (CODE_SEG/8)<<3
db 0
db 0b10001110
dw 0
%endmacro

;This macro is like the previous, but creates a trap instead (to be used with IRQ handling)

%macro ISR_TRAP 1
dw %1
dw (CODE_SEG/8)<<3
db 0
db 0b10001111
dw 0
%endmacro

IDT_start:
ISR_INT ISR_00 ;int 0x00 - Divide by zero
ISR_INT ISR_01 ;int 0x01 - Debug
ISR_INT ISR_02 ;int 0x02 - NMI
ISR_INT ISR_03 ;int 0x03 - Breakpoint
ISR_INT ISR_04 ;int 0x04 - Overflow
ISR_INT ISR_05 ;int 0x05 - Out of bounds
ISR_INT ISR_06 ;int 0x06 - Bad Op
ISR_INT ISR_07 ;int 0x07 - FPU not available
ISR_INT ISR_08 ;int 0x08 - Double Fault
ISR_INT ISR_09 ;int 0x09 - Segment overrun
ISR_INT ISR_0A ;int 0x0A - Bad TSS
ISR_INT ISR_0B ;int 0x0B - Missing Segment
ISR_INT ISR_0C ;int 0x0C - SS Fault
ISR_INT ISR_0D ;int 0x0D - GP Fault
ISR_INT ISR_0E ;int 0x0E - PF
dq 0 ;int 0x0F - Reserved
ISR_INT ISR_10 ;int 0x10 - x87 Float exception
ISR_INT ISR_11 ;int 0x11 - Alignment check
ISR_INT ISR_12 ;int 0x12 - Machine check
ISR_INT ISR_13 ;int 0x13 - SMID exception
ISR_INT ISR_14 ;int 0x14 - Virtualization exception
ISR_INT ISR_15 ;int 0x15 - Control exception
dq 0 ;int 0x16 - Reserved
dq 0 ;int 0x17 - Reserved
dq 0 ;int 0x18 - Reserved
dq 0 ;int 0x19 - Reserved
dq 0 ;int 0x1A - Reserved
dq 0 ;int 0x1B - Reserved
ISR_INT ISR_1C ;int 0x1C - Hypervisor exception
ISR_INT ISR_1D ;int 0x1D - VMM Comms exception
ISR_INT ISR_1E ;int 0x1E - Security exception
dq 0 ;int 0x1F - Reserved

ISR_TRAP ISR_20 ;IRQ 0x00 - Master Timer
ISR_TRAP ISR_21 ;IRQ 0x01 - Master keyboard
dq 0 ;IRQ 0x02 - Cascade
ISR_TRAP ISR_23 ;IRQ 0x03 - Master COM2
ISR_TRAP ISR_24 ;IRQ 0x04 - Master COM1
ISR_TRAP ISR_25 ;IRQ 0x05 - Master LPT2
ISR_TRAP ISR_26 ;IRQ 0x06 - Master floppy
ISR_TRAP ISR_27 ;IRQ 0x07 - Master LPT1 OR Spurious
ISR_TRAP ISR_28 ;IRQ 0x08 - Slave CMOS
ISR_TRAP ISR_29 ;IRQ 0x09 - Slave free (for devices)
ISR_TRAP ISR_2A ;IRQ 0x0A - Slave free (for devices)
ISR_TRAP ISR_2B ;IRQ 0x0B - Slave free (for devices)
ISR_TRAP ISR_2C ;IRQ 0x0C - Slave PC2 mouse
ISR_TRAP ISR_2D ;IRQ 0x0D - Slave FPU OR Co/Inter-processor
ISR_TRAP ISR_2E ;IRQ 0x0E - Slave primary ATA
ISR_TRAP ISR_2F ;IRQ 0x0F - Slave secondary ATA

times 8 dq 0
IDT_end:

;Interupt Service Routines

;First we need strings for all our interupt names
ISR_S db "FATAL: ", 0
ISR00_S db "#DE", 0
ISR01_S db "#DB", 0
ISR02_S db "NMI", 0
ISR03_S db "#BP", 0
ISR04_S db "#OF", 0
ISR05_S db "#BR", 0
ISR06_S db "#UD", 0
ISR07_S db "#NM", 0
ISR08_S db "#DF", 0
ISR0A_S db "#TS", 0
ISR0B_S db "#NP", 0
ISR0C_S db "#SS", 0
ISR0D_S db "#GP", 0
ISR0E_S db "#PF", 0
ISR10_S db "#MF", 0
ISR11_S db "#AC", 0
ISR12_S db "#MC", 0
ISR13_S db "#XM", 0
ISR14_S db "#VE", 0
ISR15_S db "#CP", 0
ISR1C_S db "#HV", 0
ISR1D_S db "#VC", 0
ISR1E_S db "#SX", 0

ISR_00:
mov si, ISR_S
call pm_print
mov si, ISR00_S
call pm_print
jmp $

ISR_01:
mov si, ISR_S
call pm_print
mov si, ISR01_S
call pm_print
iret

ISR_02:
mov si, ISR_S
call pm_print
mov si, ISR02_S
call pm_print
jmp $

ISR_03:
mov si, ISR_S
call pm_print
mov si, ISR03_S
call pm_print
iret

ISR_04:
mov si, ISR_S
call pm_print
mov si, ISR04_S
call pm_print
jmp $

ISR_05:
mov si, ISR_S
call pm_print
mov si, ISR05_S
call pm_print
jmp $

ISR_06:
mov si, ISR_S
call pm_print
mov si, ISR06_S
call pm_print
jmp $

ISR_07:
mov si, ISR_S
call pm_print
mov si, ISR07_S
call pm_print
jmp $

ISR_08:
mov si, ISR_S
call pm_print
mov si, ISR08_S
call pm_print
jmp $

ISR_09:
iret

ISR_0A:
mov si, ISR_S
call pm_print
mov si, ISR0A_S
call pm_print
jmp $

ISR_0B:
mov si, ISR_S
call pm_print
mov si, ISR0B_S
call pm_print
jmp $

ISR_0C:
mov si, ISR_S
call pm_print
mov si, ISR0C_S
call pm_print
jmp $

ISR_0D:
mov si, ISR_S
call pm_print
mov si, ISR0D_S
call pm_print
jmp $

ISR_0E:
mov si, ISR_S
call pm_print
mov si, ISR0E_S
call pm_print
jmp $

ISR_10:
mov si, ISR_S
call pm_print
mov si, ISR10_S
call pm_print
jmp $

ISR_11:
mov si, ISR_S
call pm_print
mov si, ISR11_S
call pm_print
jmp $

ISR_12:
mov si, ISR_S
call pm_print
mov si, ISR12_S
call pm_print
jmp $

ISR_13:
mov si, ISR_S
call pm_print
mov si, ISR13_S
call pm_print
jmp $

ISR_14:
mov si, ISR_S
call pm_print
mov si, ISR14_S
call pm_print
jmp $

ISR_15:
mov si, ISR_S
call pm_print
mov si, ISR15_S
call pm_print
jmp $

ISR_1C:
mov si, ISR_S
call pm_print
mov si, ISR1C_S
call pm_print
jmp $

ISR_1D:
mov si, ISR_S
call pm_print
mov si, ISR1D_S
call pm_print
jmp $

ISR_1E:
mov si, ISR_S
call pm_print
mov si, ISR1E_S
call pm_print
jmp $

;For now, we will ignore all IRQs TODO:Implement proper handling of IRQs

ISR_20:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_21:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_22:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_23:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_24:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_25:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_26:
mov al, 0x20
out PIC1_CMD, al
iret

ISR_27:
mov al, 0x0b
out PIC1_CMD, al
xor al, al
out 0x80, al
in al, PIC1_CMD
cmp al, 7
jne .spurious 
mov al, 0x20
out PIC1_CMD, al
.spurious:
iret

ISR_28:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_29:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_2A:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_2B:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_2C:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_2D:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_2E:
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
iret

ISR_2F:
mov al, 0x0b
out PIC2_CMD, al
xor al, al
out 0x80, al
in al, PIC2_CMD
cmp al, 15
jne .spurious 
mov al, 0x20
out PIC2_CMD, al
.spurious:
mov al, 0x20
out PIC1_CMD, al
iret

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
		dw KTSS_end - KTSS_start - 1
		dw KTSS_start ;NOTE: The TSS is located within the first 0xFFFF bytes, hence all base bits above 15 should be 0
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
		dw R0LDT_end - R0LDT_start - 1
		dw R0LDT_start
		db 0x00
		db 0b10000010
		db 0b00000000
		db 0x00		
	GDT_r1LD:
		dq 0
	GDT_r2LD:
		dq 0
	GDT_end:

R0LDT_start: ;Our kernels LDT, has 3 entries
	R0LDT_null:
		dq 0
	R0LDT_code:
		dw 0xFFFF			;Limit 0-16
		dw 0x0000			;Base 0-16
		db 0x00				;Base 16-24
		db 10011010b	;Flags + Type
		db 11001111b	;Flags + Limit 16-20
		db 0x00				;Base 24-32
	R0LDT_data:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 10010010b
		db 11001111b
		db 0x00
R0LDT_end:


KTSS_start:
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
	dd 0x06				;EFLAGS 
	dd 0x0000			;EAX
KHEADS	dd 0x0	;ECX NOTE:Must be set at runtime
KBOOTNO	dd 0x0	;EDX NOTE:Must be set at runtine
KTRCKPS	dd 0x0	;EBX NOTE:Must be set at runtime
	dd 0x9fa00		;ESP
	dd 0x9fa00		;EBP
KPARTI	dd 0x0	;ESI NOTE:Must be set at runtime
	dd 0x0				;EDI
	dw L0DT_SEG		;ES
	dw 0x0				;Reserved
	dw L0CD_SEG		;CS
	dw 0x0				;Reserved
	dw L0DT_SEG		;SS
	dw 0x0				;Reserved
	dw L0DT_SEG		;DS
	dw 0x0				;Reserved
	dw L0DT_SEG		;FS
	dw 0x0				;Reserved
	dw L0DT_SEG		;GS
	dw 0x0				;Reserved
	dw (R0LD_SEG/8)<<3;LDT Segment Descriptor
	dw 0x0				;Reserved
	dw 0x0				;Reserved + T
	dw IDT_end - IDT_start ;NO IOPB
KTSS_end:

IDT_ptr: ;This is the descriptor for the IDT
	dw IDT_end - IDT_start -1 ;Size - 1
	dd IDT_start ;Starting address

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

L0CD_SEG equ R0LDT_code - R0LDT_start
L0DT_SEG equ R0LDT_data - R0LDT_start

;Pad out so that BOOT.BIN takes up 4 sectors
times 0x800-($-$$) nop
