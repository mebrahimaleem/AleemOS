;kentry.asm

;This is the entry point for the kernel. This file will be linked with the remainaing kernel so that we can switch from assembly to C.

;NOTE: Since we are linking this file with ld, there is no need for an ORG directive

;Bootloader has put us in 32-bit mode
[BITS 32]

mov BYTE [0xb8000], 'X'

[extern kernel]
jmp kernel 

;NOTE: We can't pad out this file because this file goes to the begining of the kernel (not the end)
