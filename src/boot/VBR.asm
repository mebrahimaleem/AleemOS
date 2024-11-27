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

;VBR.asm

;Volume Boot Records for AleemOS

;Still in 16-bit
[BITS 16]

;MBR loaded VBR to 0x7c00
[ORG 0x7c00]

MBR_READ_SECTOR equ 0x72C ;This is the memory location of the MBR readsector routine, params: bx: LBA of sector to read, es:cx: where to write sector
MBR_ERR equ 0x600 + 226 ; This is the memory location of the MBR print error and hang routine, params: si: pointer to the error message null terminated string

;BPB
jmp short copyboot
nop
OEM db "MSWIN4.1"
SECTOR_BYTES dw 512
CLUSTER_SECTORS db 8
RESERVED_SECTORS dw 32
FATS db 2
ROOT_ENTRIES dw 0
SECTORS dw 0
MEDIA_BYTE db 0xF8
FAT_SECTORS dw 0
TRACK_SECTORS dw 0x3f
HEADS dw 0x20
HIDDEN_SECTORS dd 1
LARGE_SECTOR dd 0x103FF8

;Extended Boot Record
FAT_SZ dd 0x410
EXT_FLD dw 0
FS_VER dw 0
FS_ROOT dd 2
FS_INFO dw 1
VB_COP dw 6
RES0 times 12 db 0
DRIVE_NO db 0x80
RES1 db 0
EXT_SIG db 0x29
EXT_VID dd 0x1234e
EXT_VLB db "NO NAME    "
EXT_TP db "FAT32   "

copyboot:
mov BYTE [DRIVE_NO], dl
mov WORD [PARTION_OFFSET], si
mov WORD [TRACK_SECTORS], bx
mov WORD [HEADS], cx

BOOT_SRC equ 10 ;LBA Sector
BOOT_END equ 30
BOOT_DST equ 0xAC00

xor bx, bx
mov es, bx

mov bx, BOOT_SRC
mov cx, BOOT_DST

.loop0:
push bx
push cx
call MBR_READ_SECTOR
pop cx
pop bx

add bx, 1
add cx, 512
cmp bx, BOOT_END
jne .loop0

mov dl, BYTE [DRIVE_NO] ;Pass important drive parameters to the remainaing bootloader
mov si, WORD [PARTION_OFFSET]
mov bx, WORD [TRACK_SECTORS]
mov cx, WORD [HEADS]
jmp BOOT_DST ;Address of the bootloader

PARTION_OFFSET dw 0

times 510-($-$$) nop ;Pad out remaining sector
dw 0xaa55

; FSINFO strucuture 
FSI_LeadSig dd 0x41615252
Res0 times 480 db 0
FSI_StrucSi dd 0x61417272
FSI_FreeCou dd 0xFFFFFFFF
FSI_NxtFree dd 0xFFFFFFFF
Res1 times 12 db 0
FSI_TrailSi dd 0xAA550000

times 1534-($-$$) nop 
dw 0xaa55
