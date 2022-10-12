;rdir.asm

;Root directory for FAT12 filesystem

[BITS 16]

db "BOOT    "
db "BIN"
db 0x20
db 0
db 0
dw 0
dw 1
dw 1
dw 0
dw 1
dw 1
dw 2
dd 0x800

times 0x1c00-($-$$) db 0
