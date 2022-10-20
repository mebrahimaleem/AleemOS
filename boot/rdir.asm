;rdir.asm

;Root directory for FAT12 filesystem

[BITS 16]

;BOOT.BIN
db "BOOT    " ;NAME
db "BIN" ;EXTENSION
db 0x26 ;ATRIBUTES
db 0 ;TIMESTAMP STUFF (which we can ignore)
db 0
dw 0
dw 1
dw 1
dw 0
dw 1
dw 1
dw 2 ;STARTING CLUSTER
dd 0x800 ;FILE SIZE (BYTES)

;KERNEL.BIN
db "KERNEL  "
db "BIN"
db 0x26
db 0
db 0
dw 0
dw 1
dw 1
dw 0
dw 1
dw 1
dw 6
dd 0x1000


times 0x1c00-($-$$) db 0
