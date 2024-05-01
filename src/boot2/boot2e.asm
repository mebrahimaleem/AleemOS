;boot2e.asm

;This is the entry point for boot2. This file will be linked with the remainaing kernel so that we can switch from assembly to C.

;NOTE: Since we are linking this file with ld, there is no need for an ORG directive

;Bootloader has put us in 32-bit mode
[BITS 32]

[global _start]
_start:

;Store important information
[extern k_DRIVE_NO]
[extern k_PARTITION_OFFSET]
[extern k_TRACK_SECTORS]
[extern k_HEADS]
[extern k_KDATA]
mov BYTE [k_DRIVE_NO], dl
mov WORD [k_PARTITION_OFFSET], si
mov WORD [k_TRACK_SECTORS], bx
mov WORD [k_HEADS], cx
mov WORD [k_KDATA], di

;Store system tables
[extern k_idtd]
[extern k_CR3]
[extern k_gdtd]
sidt [IDTR_Buf]
mov DWORD [k_idtd], IDTR_Buf
mov eax, cr3
mov [k_CR3], eax
mov DWORD [k_gdtd], GDT_ptr

sidt [IDT_ptr]

;Install ISR 0x20
cli
mov esi, ISR20
mov edi, [IDT_st]
add edi, (0x20 * 8)
mov ecx, 2
rep movsd

;Install ISR 0x21
mov esi, ISR21
mov edi, [IDT_st]
add edi, (0x21 * 8)
mov ecx, 2
rep movsd

;Install ISR 0x2a
mov esi, ISR2A
mov edi, [IDT_st]
add edi, (0x2a * 8)
mov ecx, 2
rep movsd

;Install ISR 0x2b
mov esi, ISR2B
mov edi, [IDT_st]
add edi, (0x2b * 8)
mov ecx, 2
rep movsd

; Set scheduler status to disabled
[extern schedulerStatus]
mov BYTE [schedulerStatus], 0xff
sti

;Pass idt_ptr to boot2
[extern k_uidtd]
mov eax, UIDT_ptr
mov [k_uidtd], eax
[extern k_uidts]
mov eax, UIDT_start
mov [k_uidts], eax

;Pass end of IDT_ISR size to boot2
[extern k_uisrs]
mov eax, ISR_END
sub eax, UIDT_start
mov [k_uisrs], eax

[extern boot2]
jmp boot2

GDT_ptr:
dw 0x30
dd 0xffc0bfd0

; IDTR Buffer
IDTR_Buf:
times 6 db 0

;IDT pointer
IDT_ptr:
IDT_sz dw 0
IDT_st dd 0

;User IDT pointer
UIDT_ptr:
UIDT_sz dw UIDT_end - UIDT_start  - 1;
UIDT_st dd 0xFFC06200

align 4

%macro ISR_INT 1
dw (%1 - UIDT_start + 0x6200)
dw (1)<<3
db 0
db 0b11101110
dw 0xFFC0
%endmacro

;This macro is like the previous, but creates a trap instead (to be used with IRQ handling)

%macro ISR_TRAP 1
dw (%1 - UIDT_start + 0x6200)
dw (1)<<3
db 0
db 0b11101111
dw 0xFFC0
%endmacro

%macro ISR_TASK 2
dw 0
dw %1
db 0
db ((0b10000101) + (%2<<5))
dw 0
%endmacro

;User IDT
UIDT_start:
	ISR_TRAP ISR_00	 ;#DE Division by zero
	ISR_TRAP ISR_01	 ;#DB Debug
	ISR_TRAP ISR_02	 ;NMI Non-maskable interrupt
	ISR_TRAP ISR_03	 ;#BP Breakpoint
	ISR_TRAP ISR_04	 ;#OF Overflow
	ISR_TRAP ISR_05	 ;#BR Bound range exceeded
	ISR_TRAP ISR_06	 ;#UD Invalid opcode
	ISR_TRAP ISR_07	 ;#NM Device not available
	ISR_TRAP ISR_08	 ;#DF Double fault
	ISR_TRAP ISR_09	 ;#CO Coprocessor segment overrun
	ISR_TRAP ISR_0A	 ;#TS Invalid TSS
	ISR_TRAP ISR_0B	 ;#NP Segment not present
	ISR_TRAP ISR_0C	 ;#SS Stack segment fault
	ISR_TRAP ISR_0D	 ;#GP General protection fault
	ISR_TRAP ISR_0E	 ;#PF Page fault
	dq 0 ;Reserved
	ISR_TRAP ISR_10	 ;#MF x87 floating-point exception
	ISR_TRAP ISR_11	 ;#AC Alignment check
	ISR_TRAP ISR_12	 ;#MC Machine check
	ISR_TRAP ISR_13	 ;#XM SIMD floating-point exception
	ISR_TRAP ISR_14	 ;#VE Virtualization exception
	ISR_TRAP ISR_15	 ;#CP Control protection exception
	times 6 dq 0 ;Reserved
	ISR_TRAP ISR_1C	 ;#HV Hypervisor injection exception
	ISR_TRAP ISR_1D	 ;#VC VMM communication exception
	ISR_TRAP ISR_1E	 ;#SX Security exception
	dq 0 ;Reserved

;IRQs starting at int 0x20
	ISR_INT IRQ0
	ISR_INT IRQ1
	ISR_INT IRQ2
	ISR_INT IRQ3
	ISR_INT IRQ4
	ISR_INT IRQ5
	ISR_INT IRQ6
	ISR_INT IRQ7
	ISR_INT IRQ8
	ISR_INT IRQ9
	ISR_INT IRQA
	ISR_INT IRQB
	ISR_INT IRQC
	ISR_INT IRQD
	ISR_INT IRQE
	ISR_INT IRQF

;System ISRs
	ISR_INT ISR_SYSCALL ;System call (int 0x30)
UIDT_end:

[extern sysCall]
sysCall_pass:
push edx
push ebx
push eax
call sysCall
add esp, 8
pop edx
retf

ISR_SYSCALL:
mov edx, cr3
mov ecx, 0xc000
mov cr3, ecx
call 0x8:sysCall_pass
mov cr3, edx
iret

ISR_00:
pushad
mov edx, 0x20
jmp ISR_G

ISR_01:
pushad
mov edx, 0x21
jmp ISR_G

ISR_02:
pushad
mov edx, 0x22
jmp ISR_G

ISR_03:
pushad
mov edx, 0x23
jmp ISR_G

ISR_04:
pushad
mov edx, 0x24
jmp ISR_G

ISR_05:
pushad
mov edx, 0x25
jmp ISR_G

ISR_06:
pushad
mov edx, 0x26
jmp ISR_G

ISR_07:
pushad
mov edx, 0x27
jmp ISR_G

ISR_08:
pushad
mov edx, 0x28
jmp ISR_G

ISR_09:
pushad
mov edx, 0x29
jmp ISR_G

ISR_0A:
pushad
mov edx, 0x2A
jmp ISR_G

ISR_0B:
pushad
mov edx, 0x2B
jmp ISR_G

ISR_0C:
pushad
mov edx, 0x2C
jmp ISR_G

ISR_0D:
pushad
mov edx, 0x2D
jmp ISR_G

ISR_0E:
pushad
mov edx, 0x2E
jmp ISR_G

ISR_10:
pushad
mov edx, 0x30
jmp ISR_G

ISR_11:
pushad
mov edx, 0x31
jmp ISR_G

ISR_12:
pushad
mov edx, 0x32
jmp ISR_G

ISR_13:
pushad
mov edx, 0x33
jmp ISR_G

ISR_14:
pushad
mov edx, 0x34
jmp ISR_G

ISR_15:
pushad
mov edx, 0x35
jmp ISR_G

ISR_1C:
pushad
mov edx, 0x3C
jmp ISR_G

ISR_1D:
pushad
mov edx, 0x3D
jmp ISR_G

ISR_1E:
pushad
mov edx, 0x3E
jmp ISR_G

ISR_G:
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
call 0x8:processManager_pass ; we need to do a far call because a near/short jump does to the wrong address (due to how high memory is mapped)
pop eax
mov cr3, eax ;Restore PD
popad
iret


[extern processManager]
processManager_pass:
push edx
call processManager
add esp, 4
retf

IRQ0_pass:
jmp 0x8:ISR20_asm_R ; we need to far jump because a near/short jump goes to the wrong address (due to how the high memory is mapped)

IRQ0:
push eax
mov eax, cr3
cmp eax, 0xc000
je IRQ0_pass ; conditional jumps cannot be far jumps

mov DWORD [tebp + 0xffc00000], ebp ; for restoring state
mov ebp, [esp+16] ; get userland esp

push ecx
push edx
push ebx
push ebp ; actually esp

mov ebp, DWORD [tebp + 0xffc00000] ; the actual ebp
push ebp

push esi
push edi

push eax

mov ebp, esp

mov eax, 0xc000 ; change to kernel PD
mov cr3, eax

mov al, 0x20 ;Tell PIC we handled the interupt
out 0x20, al

call 0x8:farSchedulerEntry_asm

pop eax ; restore userland PD
mov cr3, eax

popad ; will skip esp
iret

IRQG:
call 0x8:processManager_pass
pop eax
mov cr3, eax ;Restore PD
popad
iret

IRQ1:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 2
jmp IRQG

IRQ2:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 3
jmp IRQG

IRQ3:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 4
jmp IRQG

IRQ4:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 5
jmp IRQG

IRQ5:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 6
jmp IRQG

IRQ6:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 7
jmp IRQG

IRQ7:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 8
jmp IRQG

IRQ8:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 9
jmp IRQG

IRQ9:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 10
jmp IRQG

IRQA:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 11
jmp IRQG

IRQB:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 12
jmp IRQG

IRQC:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 13
jmp IRQG

IRQD:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 14
jmp IRQG

IRQE:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 15
jmp IRQG

IRQF:
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
mov edx, 16
jmp IRQG

ISR_END:

;ISRs to install

ISR20:
dw ISR20_asm
dw (8/8)<<3
db 0
db 0b10001111
dw 0

ISR21:
dw ISR21_asm
dw (8/8)<<3
db 0
db 0b10001111
dw 0

ISR2A:
dw ISR2A_asm
dw (8/8)<<3
db 0
db 0b10001111
dw 0

ISR2B:
dw ISR2B_asm
dw (8/8)<<3
db 0
db 0b10001111
dw 0

[extern ISR20_handler]
ISR20_asm:
push eax

ISR20_asm_R:
mov DWORD [tebp], ebp ; store ebp for saving state
mov ebp, esp ; get kernel esp
add ebp, 16

push ecx
push edx
push ebx
push ebp ; actually esp

mov ebp, DWORD [tebp] ; the actual ebp
push ebp

push esi
push edi

push 0xc000 ; push cr3

mov ebp, esp

mov al, 0x20 ;Tell PIC we handled the interupt
out 0x20, al

push ebp
call ISR20_handler
add esp, 8

popad ; will skip esp
iret

tebp: dd 0

[extern ISR21_handler]

ISR21_asm:
pushad

xor eax, eax ;Get current keystoke byte
in al, 0x60

push eax ;Call function
call ISR21_handler
add esp, 0x4

mov al, 0x20 ;Tell PIC we handled the interupt
out 0x20, al

popad
iret

[extern ISR2AB_handler]

ISR2A_asm:
pushad

mov eax, 0x2a
push eax
call ISR2AB_handler
add esp, 0x4

mov al, 0x20 ;Tell PIC we handled the interupt
out 0xa0, al
out 0x20, al
popad
iret

ISR2B_asm:
pushad

mov eax, 0x2b
push eax
call ISR2AB_handler
add esp, 4

mov al, 0x20
out 0x20, al
popad
iret

[global setSysTables]
setSysTables:
lgdt [0xFFC06138]
lidt [0xFFC0613E]
push ax
mov ax, 40
ltr ax
pop ax
ret

[extern farSchedulerEntry]
farSchedulerEntry_asm:
push ebp
call farSchedulerEntry
add esp, 4
retf
