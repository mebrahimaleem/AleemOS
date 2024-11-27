;crt0.asm
; MIT License
; 
; Copyright 2022-2024 Ebrahim Aleem
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.

;crt0 assembly for the AleemOS standard library

[SECTION .text]
[global _start]

_start:

[extern __PROCESS_HEAP_BASE]
mov [__PROCESS_HEAP_BASE], ecx
add ecx, 4

push ecx
push eax

[extern main]
call main

add esp, 8

push 0 ;Syscall kill
push eax ;Exit code
int 0x30

[global _syscall]
_syscall: ; _syscall(call, params)

pushad
mov eax, [esp+36] ; call
mov ebx, [esp+40] ; params
int 0x30
pop edi
pop esi
pop ebp
add esp, 4 ; skip esp
pop ebx
pop edx
pop ecx
add esp, 4 ; skip eax because it has the return value
ret
