CWARN := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls \
	-Wnested-externs -Winline -Wno-long-long -Wconversion -Wstrict-prototypes

#CWARN_IGN := -Wcast-align

CFLAGS := $(CWARN) -masm=intel -O0 -m32 -fno-pie -ffreestanding -c -g -F dwarf

CC := gcc

B_NASM := nasm -f bin
E_NASM := nasm -f elf

LD := ld
LDFLAGS := -melf_i386 -T link.ld -s
LDDFLAGS := -melf_i386 -T linkd.ld

BOOT_RECORDS := build/MBR.bin build/VBR.bin

BOOT_ASM := $(shell find src/boot/ -type f -name "*.asm")
KERNEL_SRC := $(shell find src/kernel/ -type f -name "*.c")
DRIVERS_SRC := $(shell find src/drivers/ -type f -name "*.c")
KERNEL_HEAD := $(shell find src/kernel/ -type f -name "*.h")
DRIVERS_HEAD := $(shell find src/drivers/ -type f -name "*.h")
STDC_HEAD := $(shell find src/stdc/ -type f -name "*.h")
STDC_SRC := $(shell find src/stdc/ -type f -name "*.c")

FLAT_BIN := $(patsubst src/boot/%.asm,build/%.bin,$(BOOT_ASM))
KERNEL_OBJ := $(patsubst src/kernel/%.c,build/%.elf,$(KERNEL_SRC))
DRIVERS_OBJ := $(patsubst src/drivers/%.c,build/%.elf,$(DRIVERS_SRC))
STDC_OBJ := $(patsubst src/stdc/%.c,build/stdc/%.o,$(STDC_SRC))

.PHONY: all
all: os dbl Makefile

.PHONY: os
os: build/os.img Makefile
	@echo "Done Building OS!"

.PHONY: dbl
dbl: build/kentry.elf $(KERNEL_OBJ) $(DRIVERS_OBJ) Makefile linkd.ld
	@$(LD) $(LDDFLAGS)

build/os.img: build/MBR.bin build/FS.img Makefile
	@cat build/MBR.bin build/FS.img > build/os.img
	@truncate -s 1440000 build/os.img

build/FS.img: build/VBR.bin build/boot.bin build/kernel.bin build/sh.elf Makefile
	@cat build/VBR.bin > build/FS.img
	@truncate -s 1440000 build/FS.img
	@losetup -o 0 /dev/loop15 build/FS.img
	@mount /dev/loop15 mnt
	@cp build/boot.bin mnt/BOOT.BIN
	@cp build/kernel.bin mnt/KERNEL.BIN
	@cp build/sh.elf mnt/SH.ELF
	@cp LICENSE mnt/LICENSE
	@fatattr +rhs mnt/BOOT.BIN
	@fatattr +rhs mnt/KERNEL.BIN
	@fatattr -rhs mnt/SH.ELF
	@fatattr +r -hs mnt/LICENSE
	@umount mnt
	@dd if=/dev/loop15 seek=512 of=build/FS.img
	@losetup -d /dev/loop15

$(FLAT_BIN): build/%.bin: src/boot/%.asm Makefile
	@$(B_NASM) -o $@ $<

build/kernel.bin: build/kentry.elf $(KERNEL_OBJ) $(DRIVERS_OBJ) Makefile link.ld
	@$(LD) $(LDFLAGS)

build/kentry.elf: src/kernel/kentry.asm Makefile
	@$(E_NASM) -o $@ $<

build/kernel.elf: $(KERNEL_HEAD) $(DRIVERS_HEAD) link.ld Makefile
	@$(CC) $(CFLAGS) $< -o $@

$(KERNEL_OBJ): build/%.elf: src/kernel/%.c src/kernel/%.h Makefile
	@$(CC) $(CFLAGS) $< -o $@

$(DRIVERS_OBJ): build/%.elf: src/drivers/%.c src/drivers/%.h Makefile
	@$(CC) $(CFLAGS) $< -o $@

build/sh.elf : src/defapp/* build/stdc.elf userlandl.ld Makefile
	@$(CC) $(CFLAGS) -I src/stdc/ src/defapp/sh.c -o build/sh.o
	@ld -melf_i386 -T userlandl.ld -o build/sh.elf build/sh.o

build/stdc.elf : build/stdc/crt0.o $(STDC_OBJ) stdcl.ld Makefile
	@ld -r -melf_i386 -T stdcl.ld

build/stdc/crt0.o : src/stdc/crt0.asm Makefile
	@$(E_NASM) -o $@ $<

$(STDC_OBJ) : build/stdc/%.o: src/stdc/%.c src/stdc/%.h Makefile
	@$(CC) $(CFLAGS) $< -o $@
