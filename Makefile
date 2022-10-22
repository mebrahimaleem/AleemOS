os: build/os.img
	@truncate -s 1440000 build/os.img
	@echo Done Building OS

build/os.img: build/boot.img build/fs.img
	@cat build/boot.img build/fs.img > build/os.img

build/boot.img: build/MBR.bin build/VBR.bin
	@cat build/MBR.bin build/VBR.bin > build/boot.img

build/fs.img: build/FAT.bin build/rdir.bin build/boot.bin build/kernel.bin
	@cat build/FAT.bin build/FAT.bin build/rdir.bin build/boot.bin build/kernel.bin > build/fs.img

build/MBR.bin: boot/MBR.asm
	nasm -f bin -o build/MBR.bin boot/MBR.asm

build/VBR.bin: boot/VBR.asm
	nasm -f bin -o build/VBR.bin boot/VBR.asm

build/FAT.bin: boot/FAT.asm
	nasm -f bin -o build/FAT.bin boot/FAT.asm

build/rdir.bin: boot/rdir.asm
	nasm -f bin -o build/rdir.bin boot/rdir.asm

build/boot.bin: boot/boot.asm
	nasm -f bin -o build/boot.bin boot/boot.asm

build/kernel.bin: build/kentry.elf build/kernel.elf build/basicio.elf
	ld -melf_i386 -o build/kernel.bin -Ttext 0xb400 -Tdata 0x15FA00 -Tbss 0x13FA00 build/kentry.elf build/basicio.elf build/interupts.elf build/kernel.elf --oformat binary
	truncate -s 4096 build/kernel.bin

build/kentry.elf: kernel/kentry.asm
	nasm -f elf -o build/kentry.elf kernel/kentry.asm

build/kernel.elf: kernel/kernel.c kernel/basicio.h
	gcc -m32 -fno-pie -ffreestanding -c kernel/kernel.c -o build/kernel.elf

build/basicio.elf: kernel/basicio.c
	gcc -m32 -fno-pie -ffreestanding -c kernel/basicio.c -o build/basicio.elf
