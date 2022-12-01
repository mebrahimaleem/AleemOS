;crt0.asm

;crt0 assembly for the AleemOS standard library

[SECTION .text]
[global _start]

_start:

push eax
push ebx

[extern main]
call main

add esp, 8

push 0 ;Syscall kill
push eax ;Exit code
int 0x30
