# AleemOS

This is a simple in-progress operating system.
The future objective for this operating system is for it to be able to read and execute an ELF files from a FAT12 filesystem.

For more information, check out this repository's [wiki page](https://github.com/mebrahimaleem/AleemOS/wiki).

## Building

### Setup
Before building, ensure the directories build/, build/stdc/, and mnt/ are created. Also make sure that loopback device 0 is available.

### Building the Image
The OS can be built for i386 using the command `sudo make os`. The makefile will generate multiple files including build/os.img which contains the disk image for the OS.

### Building Kernel Debugging Symbols
The command `sudo make dbl` will create the file build/kerneld.elf which will contain kernel debugging symbols.

## Running

To run the OS in Qemu, use the command `qemu-system-i386 -drive if=none,id=stick,format=raw,file=path/to/build/os.img -device nec-usb-xhci,id=xhci -device usb-storage,bus=xhci.0,drive=stick`. You might need to change the access rights of the disk image before running this command.

To create a live bootable USB, use the command `sudo dd if=path/to/build/os.img of=path/to/device`
