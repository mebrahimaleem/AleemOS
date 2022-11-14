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

sidt [IDT_ptr]

;Install ISR 0x21
cli
mov esi, ISR21
mov edi, [IDT_st]
add edi, (0x21 * 8)
mov ecx, 2
rep movsd
sti

[extern kernel]
jmp kernel

;IDT pointer
IDT_ptr:
IDT_sz dw 0
IDT_st dd 0

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


;NOTE: We can't pad out this file because this file goes to the begining of the kernel (not the end)
