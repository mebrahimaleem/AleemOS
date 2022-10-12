os: build/os.img
	@truncate -s 1440000 build/os.img
	@echo Done Building OS

build/os.img: build/boot.img build/fs.img
	@cat build/boot.img build/fs.img > build/os.img

build/boot.img: build/MBR.bin build/VBR.bin
	@cat build/MBR.bin build/VBR.bin > build/boot.img

build/fs.img: build/FAT.bin build/rdir.bin build/boot.bin
	@cat build/FAT.bin build/FAT.bin build/rdir.bin build/boot.bin > build/fs.img

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
