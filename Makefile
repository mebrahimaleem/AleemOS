CWARN := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls \
	-Wnested-externs -Winline -Wno-long-long -Wconversion -Wstrict-prototypes

#CWARN_IGN := -Wcast-align

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
INCLUDE := $(shell find src/include/ -type f -name "*.h")
STDC_HEAD := $(shell find src/include/stdc/ -type f -name "*.h")
STDC_SRC := $(shell find src/stdc/ -type f -name "*.c")

FLAT_BIN := $(patsubst src/boot/%.asm,build/%.bin,$(BOOT_ASM))
KERNEL_OBJ := $(patsubst src/kernel/%.c,build/%.elf,$(KERNEL_SRC))
DRIVERS_OBJ := $(patsubst src/drivers/%.c,build/%.elf,$(DRIVERS_SRC))
STDC_OBJ := $(patsubst src/stdc/%.c,build/stdc/%.o,$(STDC_SRC))

CFLAGS := $(CWARN) -masm=intel -O2 -m32 -fno-pie -ffreestanding -c -g -F dwarf -I src/include/

LOOPBACK := $(shell sudo losetup -f)

.PHONY: all
all: os dbl Makefile

.PHONY: os
os: build/os.img Makefile
	@echo "Done Building OS!"

.PHONY: dbl
dbl: build/boot2e.elf build/boot2.elf build/taskSwitch.elf $(KERNEL_OBJ) $(DRIVERS_OBJ) build/shd.elf Makefile linkd.ld
	$(LD) $(LDDFLAGS)

build/FS.img: build/MBR.bin build/VBR.bin build/FAT.bin Makefile
	dd if=/dev/zero of=$@ bs=512 count=1064958
	dd conv=notrunc if=build/MBR.bin of=$@ bs=512 seek=0 count=1
	dd conv=notrunc if=build/VBR.bin of=$@ bs=512 seek=1 count=3
	dd conv=notrunc if=build/VBR.bin of=$@ bs=512 seek=7 count=3
	dd conv=notrunc if=build/FAT.bin of=$@ bs=1 seek=16896 count=12
	dd conv=notrunc if=build/FAT.bin of=$@ bs=1 seek=549376 count=12

build/os.img: build/FS.img build/boot.bin build/kernel.bin build/sh.elf Makefile
	cp $< $@
	sudo losetup -o 512 $(LOOPBACK) build/os.img
	sudo mount -t vfat -o umask=000 $(LOOPBACK) mnt
	dd conv=notrunc if=build/boot.bin of=$@ bs=512 seek=10 count=23
	cp build/kernel.bin mnt/KERNEL.BIN
	cp build/sh.elf mnt/SH.ELF
	cp LICENSE mnt/LICENSE
	sudo fatattr +rhs mnt/KERNEL.BIN
	sudo fatattr +r -hs mnt/SH.ELF
	sudo fatattr +r -hs mnt/LICENSE
	sudo umount mnt
	sudo dd if=$(LOOPBACK) of=$@ bs=4M seek=512
	sudo losetup -d $(LOOPBACK)

$(FLAT_BIN): build/%.bin: src/boot/%.asm Makefile
	$(B_NASM) -o $@ $<

build/kernel.bin: build/boot2e.elf build/boot2.elf build/taskSwitch.elf $(KERNEL_OBJ) $(DRIVERS_OBJ) Makefile link.ld
	$(LD) $(LDFLAGS)

build/boot2e.elf: src/boot2/boot2e.asm Makefile
	$(E_NASM) -o $@ $<

build/boot2.elf: src/boot2/boot2.c $(INCLUDE) link.ld Makefile
	$(CC) $(CFLAGS) $< -o $@

build/taskSwitch.elf: src/kernel/taskSwitch.asm $(INCLUDE) link.ld Makefile
	$(E_NASM) -o $@ $<

$(KERNEL_OBJ): build/%.elf: src/kernel/%.c src/include/%.h Makefile
	$(CC) $(CFLAGS) $< -o $@

$(DRIVERS_OBJ): build/%.elf: src/drivers/%.c src/include/%.h Makefile
	$(CC) $(CFLAGS) $< -o $@

build/sh.elf : src/defapp/* build/stdc.elf userlandl.ld Makefile
	$(CC) $(CFLAGS) -I src/include/stdc/ -I src/include/defapp/ src/defapp/sh.c -o build/sh.o -g
	ld -melf_i386 -T userlandl.ld -o build/sh.elf build/sh.o -s

build/shd.elf : src/defapp/* build/stdc.elf userlandl.ld Makefile
	ld -melf_i386 -T userlandl.ld -o build/shd.elf build/sh.o

build/stdc.elf : build/stdc/crt0.o $(STDC_OBJ) stdcl.ld Makefile
	ld -r -melf_i386 -T stdcl.ld

build/stdc/crt0.o : src/stdc/crt0.asm Makefile
	$(E_NASM) -o $@ $<

$(STDC_OBJ) : build/stdc/%.o: src/stdc/%.c src/include/stdc/%.h Makefile
	$(CC) $(CFLAGS) -I src/include/stdc/ $< -o $@
