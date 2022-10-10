;VBR.asm

;Volume Boot Records for AleemOS

;Still in 16-bit
[BITS 16]

;MBR loaded VBR to 0x7c00
[ORG 0x7c00]

;BPB
jmp short boot
nop
OEM db "FRDOS5.1"
SECTOR_BYTES dw 512
CLUSTER_SECTORS db 1
RESERVED_SECTORS dw 4
FATS db 2
ROOT_ENTRIES dw 224 ;14 sectors
SECTORS dw 0x0b3a
MEDIA_BYTE db 0xF0
FAT_SECTORS dw 9
TRACK_SECTORS dw 18
HEADS dw 2
HIDDEN_SECTORS dd 1
LARGE_SECTOR dd 0

;Extended Boot Record
DRIVE_NO db 0 ;NOTE: We don't know this at compile time
NT_FLAGS db 0
SIGNATURE db 41
VOLUME_ID dd 16
VOLUME_LABEL db "ALEEMOS    "
FS_ID db "FAT12   "

boot:

mov BYTE [DRIVE_NO], dl
mov WORD [PARTION_OFFSET], si
jmp $

PARTION_OFFSET dw 0

times 510-($-$$) nop
dw 0xaa55
