;kentry.asm

;This is the entry point for the kernel. This file will be linked with the remainaing kernel so that we can switch from assembly to C.

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
[extern k_gdtd]
[extern k_ldtd]
[extern k_idtd]
[extern k_tssd]
[extern k_CR3]
sgdt [k_gdtd]
sldt [k_ldtd]
sidt [k_idtd]
str [k_tssd]
mov eax, cr3
mov [k_CR3], eax

sidt [IDT_ptr]

;Install ISR 0x21
cli
mov esi, ISR21
mov edi, [IDT_st]
add edi, (0x21 * 8)
mov ecx, 2
rep movsd
sti

;Pass idt_ptr to kernel
[extern k_uidtd]
mov eax, UIDT_ptr
mov [k_uidtd], eax
[extern k_uidts]
mov eax, UIDT_start
mov [k_uidts], eax

;Pass end of IDT_ISR size to kernel
[extern k_uisrs]
mov eax, ISR_END
sub eax, UIDT_start
mov [k_uisrs], eax

;Pass GDT to kernel
[extern k_ugdtd]
mov eax, UGDT_ptr
mov [k_ugdtd], eax

[extern kernel]
jmp kernel

UGDT_ptr:
dw 55
dd 0xFFC06000

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
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	dq 0 ;Reserved
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	times 6 dq 0 ;Reserved
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	ISR_INT ISR_KILL ;Kill process
	dq 0 ;Reserved

;IRQs starting at int 0x20
	ISR_TRAP IRQ0
	ISR_TRAP IRQ1
	ISR_TRAP IRQ2
	ISR_TRAP IRQ3
	ISR_TRAP IRQ4
	ISR_TRAP IRQ5
	ISR_TRAP IRQ6
	ISR_TRAP IRQ7
	ISR_TRAP IRQ8
	ISR_TRAP IRQ9
	ISR_TRAP IRQA
	ISR_TRAP IRQB
	ISR_TRAP IRQC
	ISR_TRAP IRQD
	ISR_TRAP IRQE
	ISR_TRAP IRQF

;Task Gates
	ISR_TASK 40, 0 ;int 0x30
UIDT_end:

ISR_KILL:
cli
mov dword [0xFFC06FFC], 0
xchg bx, bx
int 0x30

IRQ0:
cli
mov dword [0xFFC06FFC], 1
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ1:
cli
mov dword [0xFFC06FFC], 2
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ2:
cli
mov dword [0xFFC06FFC], 3
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ3:
cli
mov dword [0xFFC06FFC], 4
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ4:
cli
mov dword [0xFFC06FFC], 5
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ5:
cli
mov dword [0xFFC06FFC], 6
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ6:
cli
mov dword [0xFFC06FFC], 7
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ7:
cli
mov dword [0xFFC06FFC], 8
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ8:
cli
mov dword [0xFFC06FFC], 9
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQ9:
cli
mov dword [0xFFC06FFC], 10
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQA:
cli
mov dword [0xFFC06FFC], 11
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQB:
cli
mov dword [0xFFC06FFC], 12
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQC:
cli
mov dword [0xFFC06FFC], 13
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQD:
cli
mov dword [0xFFC06FFC], 14
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQE:
cli
mov dword [0xFFC06FFC], 15
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

IRQF:
cli
mov dword [0xFFC06FFC], 16
mov dword [0xFFC06FF8], eax
mov dword [0xFFC06FF4], ebx
mov dword [0xFFC06FF0], ecx
mov dword [0xFFC06FEC], edx
mov dword [0xFFC06FE8], esi
mov dword [0xFFC06FE4], edi
mov dword [0xFFC06FE0], esp
mov dword [0xFFC06FDC], ebp
int 0x30

ISR_END:

;ISRs to install

ISR21:
dw ISR21_asm
dw (8/8)<<3
db 0
db 0b10001111
dw 0

[extern ISR21_handler]

ISR21_asm:
cli
pushad

xor eax, eax ;Get current keystoke byte
in al, 0x60

push eax ;Call function
call ISR21_handler
add esp, 0x4

mov al, 0x20 ;Tell PIC we handled the interupt
out 0x20, al

popad
sti
iret

[global setSysTables]
setSysTables:
push ebp
mov ebp, esp
lgdt [0xFFC06138]
lidt [0xFFC0613E]
push eax
mov ax, 40
ltr ax
pop eax
pop ebp
ret

;NOTE: We can't pad out this file because this file goes to the begining of the kernel (not the end)
