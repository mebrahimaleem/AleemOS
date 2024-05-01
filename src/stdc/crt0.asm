;crt0.asm

;crt0 assembly for the AleemOS standard library

[SECTION .text]
[global _start]

_start:

[extern __PROCESS_HEAP_BASE]
mov [__PROCESS_HEAP_BASE], ecx
mov DWORD [ecx], 0

push eax
push ebx

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
