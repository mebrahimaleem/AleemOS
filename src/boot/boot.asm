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

;Verify PCI support mechanism #1
mov ax, 0xB101
mov edi, 0
int 0x1A
cmp ah, 0
jne pci_err
and al, 1
cmp al, 1
jne pci_err
jmp pci_ok

pci_err:
mov si, pci_err_msg
call real_print
.hang: jmp .hang
pci_err_msg: db "FATAL: Failed to verify PCI mechanism #1 exists. System Hang.", 0

pci_ok:

;Set the VGA video mode
xor ah, ah
mov al, 0x03
int 0x10 

mov ah, 1
mov ch, 0
mov cl, 15
int 0x10

;Now we need to copy the kernel onto the RAM
mov si, KERNEL_FNAME ;file to copy
mov cx, 0xE000 ;Where to copy kernel
call VBR_READ_FILE
xor dx, dx
mov es, dx

;Copy the default application (sh.elf) onto the RAM
mov si, DEFAPP_FNAME
mov cx, 0x0800
call VBR_READ_FILE
xor dx, dx
mov es, dx

;Now we need the memory map
get_mmap:
xor ebx, ebx
mov di, ARDS
mov ecx, 20
mov si, 16

.loop:
mov eax, 0xe820
mov edx, 0x534D4150
int 0x15

jc .exit ;Check for error

add di, 20 ;Next range
dec si
cmp si, 0
je .exit

cmp ebx, 0
jne .loop

.exit:

;Next, we will store a timestamp on the MBR (on the disk) so that the kernel (in PM) can find the disk
;The only way for this method to fail is for the user to boot the code at time t, then poweroff, then boot again (from another drive) at time t
;This is very unlikely, so our method should be safe to use

xor ah, ah ;Get time
int 0x1a

mov [MBR_SIG], dx ;Save this value to pass to the kernel
mov [MBR_SIG+2], cx
 
mov [0x600+32], dx ;Save this value in the MBR
mov [0x600+34], cx

mov si, 3 ;Attempt 3 writes

write_MBR:
mov al, 1

;CHS (0, 0, 1)
xor ch, ch
mov cl, 1
xor dh, dh
mov dl, [DRIVE_NO]
mov bx, 0x600
mov ah, 3
int 0x13

;Now we need to enter PM
cli ;We will disable interupts until the kernel has correctly set up the IDT
lgdt [GDT_ptr] ;Load the GDT descriptor

mov eax, cr0 ;Set the PM bit
or eax, 0x01
mov cr0, eax
jmp CODE_SEG:enter_pm ;Far jump to 32-bit (to set CS)

;Kernel Data structure

KERNEL_DATA:

;MBR signature
MBR_SIG dd 0

;Address Range Descriptor Structure
ARDS times 16*20 db 0x0

;System TIME
FRACTION_MS dd 0
WHOLE_MS dd 0

PIT_FRCMS dd 0
PIT_WHLMS dd 0

;Keyboard queue pointer
KEYBOARD_QUEUE dd 0

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

;Load the IDT descriptor
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

;Set segments
mov ax, DATA_SEG
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

sti

;Now we need to setup and enable paging
mov eax, PAGE_TABLE_1
or eax, 3
mov [PAGE_DIRECTORY], eax

;Set control register
mov eax, PAGE_DIRECTORY
mov cr3, eax

;Enable paging
mov eax, cr0
or eax, 0x80000000
mov cr0, eax

;Setup PIT

PIT_FREQ equ 100 ;100Hz

;Find the closest PIT reload value
xor edx, edx
mov eax, 3579545
mov ebx, PIT_FREQ
div ebx
cmp edx, 3579545/2
jb .rd1 ;Round down
inc eax ;Round up

.rd1:
xor edx, edx
mov ebx, 3
div ebx
cmp edx, 3/2
jb .rd2 ;Round down
inc eax ;Round up

.rd2:
push eax
;Calculate the frequency of IRQ0 firing
xor edx, edx
mov ebx, eax
mov eax, 0xdbb3a062
mul ebx
shrd eax, edx, 10
shr edx, 10

mov [PIT_FRCMS], eax
mov [PIT_WHLMS], edx

;Actually set the PIT
cli
mov al, 0b00110100 ;Use rate generator because it has better accuracy
out 0x43, al ;Set the mode

pop eax
out 0x40, al ;Set the reload value
mov al, ah
out 0x40, al

; Jump to boot2e

mov eax, 0x0
mov ecx, [KHEADS]
mov edx, [KBOOTNO]
mov ebx, [KTRCKPS]
mov esi, [KPARTI]
mov edi, KERNEL_DATA
mov ebp, 0x9fa00
mov esp, ebp

jmp 0xe000

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

KERNEL_FNAME db "KERNEL  BIN"
DEFAPP_FNAME db "SH      ELF"

;NOTE: 	The remaining code defines the various system datastructures that are used in protected mode. Most OS implementations do this in the kernel, but in order
				;to call the kernel in protected mode, we will declare these in the bootloader and the kernel can access them later (using sgdt sidt, Etc.)

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

%macro ISR_TASK 2
dw 0
dw %1
db 0
db ((0b10000101) + (%2<<5))
dw 0
%endmacro

align 4

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
cli
mov si, ISR_S
call pm_print
mov si, ISR00_S
call pm_print
jmp $

ISR_01:
cli
mov si, ISR_S
call pm_print
mov si, ISR01_S
call pm_print
iret

ISR_02:
cli
mov si, ISR_S
call pm_print
mov si, ISR02_S
call pm_print
jmp $

ISR_03:
cli
mov si, ISR_S
call pm_print
mov si, ISR03_S
call pm_print
iret

ISR_04:
cli
mov si, ISR_S
call pm_print
mov si, ISR04_S
call pm_print
jmp $

ISR_05:
cli
mov si, ISR_S
call pm_print
mov si, ISR05_S
call pm_print
jmp $

ISR_06:
cli
mov si, ISR_S
call pm_print
mov si, ISR06_S
call pm_print
jmp $

ISR_07:
cli
mov si, ISR_S
call pm_print
mov si, ISR07_S
call pm_print
jmp $

ISR_08:
cli
mov si, ISR_S
call pm_print
mov si, ISR08_S
call pm_print
jmp $

ISR_09:
iret

ISR_0A:
cli
mov si, ISR_S
call pm_print
mov si, ISR0A_S
call pm_print
jmp $

ISR_0B:
cli
mov si, ISR_S
call pm_print
mov si, ISR0B_S
call pm_print
jmp $

ISR_0C:
cli
mov si, ISR_S
call pm_print
mov si, ISR0C_S
call pm_print
jmp $

ISR_0D:
cli
mov si, ISR_S
call pm_print
mov si, ISR0D_S
call pm_print
jmp $

ISR_0E:
cli
mov si, ISR_S
call pm_print
mov si, ISR0E_S
call pm_print
jmp $

ISR_10:
cli
mov si, ISR_S
call pm_print
mov si, ISR10_S
call pm_print
jmp $

ISR_11:
cli
mov si, ISR_S
call pm_print
mov si, ISR11_S
call pm_print
jmp $

ISR_12:
cli
mov si, ISR_S
call pm_print
mov si, ISR12_S
call pm_print
jmp $

ISR_13:
cli
mov si, ISR_S
call pm_print
mov si, ISR13_S
call pm_print
jmp $

ISR_14:
cli
mov si, ISR_S
call pm_print
mov si, ISR14_S
call pm_print
jmp $

ISR_15:
cli
mov si, ISR_S
call pm_print
mov si, ISR15_S
call pm_print
jmp $

ISR_1C:
cli
mov si, ISR_S
call pm_print
mov si, ISR1C_S
call pm_print
jmp $

ISR_1D:
cli
mov si, ISR_S
call pm_print
mov si, ISR1D_S
call pm_print
jmp $

ISR_1E:
cli
mov si, ISR_S
call pm_print
mov si, ISR1E_S
call pm_print
jmp $

;For now, we will ignore all IRQs NOTE:	Several IRQs can be ignored until we switch to rings 1 and 2 (ex: the kernel does not need to know what keys the user
																			;	presses, it only needs to give it to other applications, which will be implemented in the ring 1/2 IDT's ISRs.

ISR_20:
cli

;Save registers
push eax
push ebx

;Get fractional and while ms delay between IRQ0s
mov eax, [PIT_FRCMS]
mov ebx, [PIT_WHLMS]

;Update system timer
add eax, DWORD [FRACTION_MS]
add ebx, DWORD [WHOLE_MS]

mov DWORD [FRACTION_MS], eax
mov DWORD [WHOLE_MS], ebx

mov al, 0x20
out PIC1_CMD, al

pop ebx
pop eax

sti
iret

ISR_21:
cli
push eax

in al, 0x60 ;Get keyscan to keep keyboard buffer clean

mov al, 0x20
out PIC1_CMD, al

pop eax
sti
iret

ISR_22:
cli
push eax
mov al, 0x20
out PIC1_CMD, al
pop eax
sti
iret

ISR_23:
cli
push eax
mov al, 0x20
out PIC1_CMD, al
pop eax
sti
iret

ISR_24:
cli
push eax
mov al, 0x20
out PIC1_CMD, al
pop eax
sti
iret

ISR_25:
cli
push eax
mov al, 0x20
out PIC1_CMD, al
pop eax
sti
iret

ISR_26:
cli
push eax
mov al, 0x20
out PIC1_CMD, al
pop eax
sti
iret

ISR_27:
cli
push eax
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
pop eax
sti
iret

ISR_28:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_29:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_2A:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_2B:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_2C:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_2D:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_2E:
cli
push eax
mov al, 0x20
out PIC2_CMD, al
out PIC1_CMD, al
pop eax
sti
iret

ISR_2F:
cli
push eax
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
pop eax
sti
iret

;GDT
align 4

KHEADS	dd 0x0	;ECX
KBOOTNO	dd 0x0	;EDX
KTRCKPS	dd 0x0	;EBX
KPARTI	dd 0x0	;ESI

align 4
IDT_ptr: ;This is the descriptor for the IDT
	dw IDT_end - IDT_start -1 ;Size - 1
	dd IDT_start ;Starting address

align 4
GDT_ptr: ;This is the descriptor for the GDT
	dw GDT_end - GDT_start - 1 ;Size - 1
	dd GDT_start ;Starting address

;Some useful constants
CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

times 0x1400-($-$$) - 8 * 6 db 0;We need to be 4KiB aligned

GDT_start:
	GDT_null: ;NULL descriptor
		dq 0
	GDT_code: ;Ring 0 code descriptor
		dw 0xFFFF			;Limit 0-16
		dw 0x0000			;Base 0-16
		db 0x00				;Base 16-24
		db 10011010b	;Flags + Type
		db 11001111b	;Flags + Limit 16-20
		db 0x00				;Base 24-32
	GDT_data: ;Ring 0 data descriptor
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 10010010b
		db 11001111b
		db 0x00
	dq 0 ; User code (set by boot2)
	dq 0 ; User data (set by boot2)
	dq 0 ; TSS
	GDT_end:

;Page directory
PAGE_DIRECTORY:
times 1024 dd 2 ;Create non present pages

;Page table 1
PAGE_TABLE_1:
%assign i 0
%rep 1024
	dd ((i * 0x1000) | 3) + 0x0000
%assign i i+1
%endrep

;No need to pad out file since this file is stored in the filesystem
