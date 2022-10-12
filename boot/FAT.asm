;FAT.asm

;File Allocation Table for primary partition

[BITS 16]

dw 0
db 0

db 0b00000011
db 0b01000000
db 0b00000000
db 0b00000101
db 0b11110000
db 0b11111111

times 0x1200-($-$$) db 0
