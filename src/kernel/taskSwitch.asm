[global taskSwitch]
taskSwitch:
cli
mov esi, [esp + 4] ; State pointer
; trampoline

jmp trampolineEntry + 0xffc00000

trampolineEntry:

; Prepare registers
mov eax, [esi + 16]
mov [r_esi + 0xffc00000], eax
mov eax, [esi + 28]
mov [r_ebp + 0xffc00000], eax
mov eax, [esi + 20]
mov [r_edi + 0xffc00000], eax
mov eax, [esi + 12]
mov [r_edx + 0xffc00000], eax
mov eax, [esi + 8]
mov [r_ecx + 0xffc00000], eax
mov eax, [esi + 4]
mov [r_ebx + 0xffc00000], eax
mov eax, [esi + 0]
mov [r_eax + 0xffc00000], eax

mov eax, [esi + 32]
mov [r_eip + 0xffc00000], eax
mov eax, [esi + 36]
mov [r_efl + 0xffc00000], eax

mov ebx, [esi + 24] ; Save userland esp for later

mov eax, [esi + 40] ; Prepare paging
mov cr3, eax

; Move stack frame to userland
mov esp, ebx

mov eax, esp ; Prepare stack frame
push 0x23
push eax
push DWORD [r_efl + 0xffc00000]
push 0x1b
push DWORD [r_eip + 0xffc00000]

mov ax, 0x23 ; Prepare segment selectors
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax

mov eax, [r_eax + 0xffc00000]
mov ebx, [r_ebx + 0xffc00000]
mov ecx, [r_ecx + 0xffc00000]
mov edx, [r_edx + 0xffc00000]
mov edi, [r_edi + 0xffc00000]
mov ebp, [r_ebp + 0xffc00000]
mov esi, [r_esi + 0xffc00000]

iret

jmp $

r_eax dd 0
r_ebx dd 0
r_ecx dd 0
r_edx dd 0
r_edi dd 0
r_ebp dd 0
r_esi dd 0
r_eip dd 0
r_efl dd 0
