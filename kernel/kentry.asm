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
UIDT_end:

[extern processManager]
ISR_KILL:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 0
push 0 
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ0:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 1
push 1
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ1:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 2
push 2
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ2:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 3
push 3
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ3:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 4
push 4
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ4:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 5
push 5
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ5:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 6
push 6
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ6:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 7
push 7
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ7:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 8
push 8
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ8:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 9
push 9
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQ9:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 10
push 10
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQA:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 11
push 11
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQB:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 12
push 12
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQC:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 13
push 13
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQD:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 14
push 14
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQE:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 15
push 15
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

IRQF:
cli
pushad
mov ebp, esp
mov eax, cr3
push eax
mov eax, 0xc000
mov cr3, eax ;Change to kernel PD
push 16
push 16
add esp, 4
call 0x8:processManager
add esp, 4
pop eax
mov cr3, eax ;Restore PD
popad
sti
iret

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
cli
lgdt [0xFFC06138]
lidt [0xFFC0613E]
push eax
mov ax, 40
ltr ax
pop eax
pop ebp
ret

;NOTE: We can't pad out this file because this file goes to the begining of the kernel (not the end)
