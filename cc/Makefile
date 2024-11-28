# MIT License
# 
# Copyright 2022-2024 Ebrahim Aleem
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

TARGET := i386-elf
KERNEL_PREFIX := $(CURDIR)/cross/kernel-prefix/

GNU_FTP := ftp.gnu.org/gnu

.PHONY: cc
cc: kernel-cc

.PHONY: disclean
distclean: src/%/:
	-$(MAKE) -C $< distclean

.PHONY: clean
clean: distclean
	-rm -rdf src/ build/ cross/

%/:
	-mkdir -p $@

src/gnu-keyring.gpg: | src/
	wget -P src/ $(GNU_FTP)/$(@F)

src/binutils-2.42.tar.xz: | src/gnu-keyring.gpg
	wget -P src/ $(GNU_FTP)/binutils/$(@F)
	wget -P src/ $(GNU_FTP)/binutils/$(@F).sig
	gpgv --keyring $| $@.sig $@

src/gcc-13.2.0.tar.xz: | src/gnu-keyring.gpg
	wget -P src/ $(GNU_FTP)/gcc/gcc-13.2.0/$(@F)
	wget -P src/ $(GNU_FTP)/gcc/gcc-13.2.0/$(@F).sig
	gpgv --keyring $| $@.sig $@

src/binutils-2.42/: | src/binutils-2.42.tar.xz
	-mkdir -p src/ #TODO: find a better solution
	tar xvf $| -C src/

src/gcc-13.2.0/: | src/gcc-13.2.0.tar.xz
	-mkdir -p src/ #TODO: find a better solution
	tar xvf $| -C src/

.PHONY: kernel-bu
kernel-bu: build/kernel-bu/ cross/kernel-prefix/ | src/binutils-2.42/
	-$(MAKE) -C $< distclean
	cd $<; \
		../../src/binutils-2.42/configure \
			--prefix=$(KERNEL_PREFIX) \
			--target=$(TARGET) \
			--with-sysroot \
			--disable-nls \
			--disable-werror \
			--program-prefix=i386-aleemos-
	$(MAKE) -C $<
	$(MAKE) -C $< install

.PHONY: kernel-cc
kernel-cc: build/kernel-cc/ cross/kernel-prefix/ | src/gcc-13.2.0/ kernel-bu
	-$(MAKE) -C $< distclean
	cd $<; \
		../../src/gcc-13.2.0/configure \
			--prefix=$(KERNEL_PREFIX) \
			--target=$(TARGET) \
			--disable-nls \
			--disable-werror \
			--program-prefix=i386-aleemos- \
			--enable-languages=c,c++ \
			--without-headers \
			--with-as=$(KERNEL_PREFIX)/bin/i386-aleemos-as \
			--with-ld=$(KERNEL_PREFIX)/bin/i386-aleemos-ld
	$(MAKE) -C $< all-gcc
	$(MAKE) -C $< install-gcc
