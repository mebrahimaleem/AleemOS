;boot.asm

;Bootstrapper for AleemOS

;We are located just after the bootsector
[ORG 0x7e00]

;We have not switched to 32-bit mode yet
[BITS 16]

jmp $

db "Hello, World from AleemOS!"

times 0x800-($-$$) nop
