# AleemOS

This is a simple in-progress operating system.
The future objective for this operating system is for it to be able to read and execute an ELF files from a FAT32 filesystem.

For more information, check out this repository's [wiki page](https://github.com/mebrahimaleem/AleemOS/wiki).

## Building

### Before Building
Before building, make you need to install the cross compiler. This can be done in two ways: via an automated script or manually. It is highly recommended to build the cross compiler manually. Either way, make sure you have installed the prerequisites (see [here](https://gcc.gnu.org/install/prerequisites.html)). For reproducibility, gcc 13.2.0 and binutils 2.42 were used.

For manual installation, build binutils with `configure --target=i386-elf --prefix/path/to/install --with-sysroot --disable-nls --disable-werror --program-prefix=i386-aleemos-` and `make && make install`. Build gcc with `configure --target-i386-elf --prefix=/path/to/install --disable-nls --enable-languages=c,c++ --without-headers --with-as=/path/to/install/bin/i386-aleemos-as --with-ld=/path/to/install/bin/i386-aleemos-ld`, then `make all-gcc & make install-gcc`. Make sure to follow the gcc building instructions [here](https://gcc.gnu.org/install/index.html).

Alternatively, for automated installation, run `make cc`, then add `cc/cross/kernel-prefix/bin` to your path.

Make sure to add `/path/to/install/bin` to your path after installation.

### Building the Image
The OS can be built for i386 using the command `make os`. The makefile will generate multiple files including build/os.img which contains the disk image for the OS. You may need to run as a superuser to provide loopback mounting privilages.

### Building Kernel Debugging Symbols
The command `sudo make dbl` will create the file build/kerneld.elf which will contain kernel debugging symbols.

## Running

To run the OS in Qemu, use the command `qemu-system-i386 -drive if=none,id=stick,format=raw,file=path/to/build/os.img -device nec-usb-xhci,id=xhci -device usb-storage,bus=xhci.0,drive=stick`. You might need to change the access rights of the disk image before running this command.

To create a live bootable USB, use the command `dd if=path/to/build/os.img of=path/to/device`
