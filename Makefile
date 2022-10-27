CWARN := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls \
	-Wnested-externs -Winline -Wno-long-long -Wconversion -Wstrict-prototypes

#CWARN_IGN := -Wcast-align

CFLAGS := $(CWARN) -masm=intel -m32 -fno-pie -ffreestanding -c

CC := gcc

B_NASM := nasm -f bin
E_NASM := nasm -f elf

LD := ld
LDFLAGS := -melf_i386 -T link.ld

BOOT_RECORDS := build/MBR.bin build/VBR.bin

BOOT_ASM := $(shell find boot/ -type f -name "*.asm")
KERNEL_SRC := $(shell find kernel/ -type f -name "*.c")
DRIVERS_SRC := $(shell find drivers/ -type f -name "*.c")

FLAT_BIN := $(patsubst boot/%.asm,build/%.bin,$(BOOT_ASM))
KERNEL_OBJ := $(patsubst kernel/%.c,build/%.elf,$(KERNEL_SRC))
DRIVERS_OBJ := $(patsubst drivers/%.c,build/%.elf,$(DRIVERS_SRC))

.PHONY: all
all: os

.PHONY: os
os: build/os.img
	@echo "Done Building OS!"

build/os.img: build/MBR.bin build/FS.img
	@cat build/MBR.bin build/FS.img > build/os.img
	@truncate -s 1440000 build/os.img

build/FS.img: build/VBR.bin build/boot.bin build/kernel.bin
	@cat build/VBR.bin > build/FS.img
	@truncate -s 1440000 build/FS.img
	@losetup -D
	@losetup -o 0 /dev/loop0 build/FS.img
	@mount /dev/loop0 mnt
	@cp build/boot.bin mnt/BOOT.BIN
	@cp build/kernel.bin mnt/KERNEL.BIN
	@umount mnt
	@dd if=/dev/loop0 seek=512 of=build/FS.img
	@losetup -d /dev/loop0

$(FLAT_BIN): build/%.bin: boot/%.asm
	@$(B_NASM) -o $@ $<

build/kernel.bin: build/kentry.elf $(KERNEL_OBJ) $(DRIVER_OBJ)
	@$(LD) $(LDFLAGS)
	@truncate -s 4096 build/kernel.bin

build/kentry.elf: kernel/kentry.asm
	@$(E_NASM) -o $@ $<

$(KERNEL_OBJ): build/%.elf: kernel/%.c
	@$(CC) $(CFLAGS) $< -o $@

$(DRIVERS_OBJ): build/%.elf: drivers/%.c
	@$(CC) $(CFLAGS) $< -o $@
