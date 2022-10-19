;boot.asm

;Bootstrapper for AleemOS

;We are located just after the bootsector
[ORG 0x7e00]

;We have not switched to 32-bit mode yet
[BITS 16]

mov ah, 0x0E
mov al, 'X'
mov bh, 0
mov bl, 0xF0
int 0x10

jmp $

times 0x800-($-$$) nop
