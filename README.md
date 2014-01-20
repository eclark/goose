What is this?
=============

Goose is a hobbyist operating system I am developing to learn more about
x86-64 systems.

What does it do?
================

Almost nothing. It configures paging and memory management, then accepts
keyboard and COM1 interrupts.

Why?
====

After building a simple x86 operating system in the Computer Systems
Engineering course at UIUC, I want to understand how to implement support
for modern features of x86 systems, such as ACPI, APIC, HPET, and Long
Mode.

License
=======

The project uses the MIT license. See LICENSE.

Build Environment
=================

Incomplete notes on the toolchain.

	../binutils-2.24/configure --prefix="$HOME/opt" --disable-nls --enable-targets="i586-elf,x86_64-elf,i386-efi-pe" --enable-64-bit-bfd --program-prefix=cross-

	make
	make install

	../gcc-4.8.2/configure --prefix="$HOME/opt" --disable-nls --enable-targets="i586-elf,x86_64-elf,i386-efi-pe" --enable-languages="c,c++" --with-newlib --without-headers --program-prefix=cross-

	make all-gcc
	make install-gcc

Install grub2. Install xorriso.

Running
=======

	qemu-system-x86_64 -cdrom os.iso -serial stdio -display none -s
