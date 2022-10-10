os: build/os.img
	truncate -s 1440000 build/os.img
	@echo Done Building OS

build/os.img: build/MBR.bin build/VBR.bin
	cat build/MBR.bin build/VBR.bin > build/os.img

build/MBR.bin: boot/MBR.asm
	nasm -f bin -o build/MBR.bin boot/MBR.asm

build/VBR.bin: boot/VBR.asm
	nasm -f bin -o build/VBR.bin boot/VBR.asm
