;FAT.asm

;File Allocation Table for primary partition

[BITS 16]

;(cluster 0)

;Reserved
dw 0
db 0

;(cluster 2)

;BOOT.BIN (4 sectors)
db 0b00000011
db 0b01000000
db 0b00000000
db 0b00000101 ;4
db 0b11110000
db 0b11111111

;(cluster 6)

;KERNEL.BIN (8 sectors)
db 0b00000111
db 0b10000000
db 0b00000000
db 0b00001001 ;8
db 0b10100000
db 0b00000000
db 0b00001011 ;10
db 0b11000000
db 0b00000000
db 0b00001101 ;12
db 0b11110000
db 0b11111111

;(cluster 14)

times 0x1200-($-$$) db 0
