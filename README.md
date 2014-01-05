Notes on building the operating system.

../binutils-2.24/configure --prefix="$HOME/opt" --disable-nls --enable-targets="i586-elf,x86_64-elf,i386-efi-pe" --enable-64-bit-bfd --program-prefix=cross-

make
make install

../gcc-4.8.2/configure --prefix="$HOME/opt" --disable-nls --enable-targets="i586-elf,x86_64-elf,i386-efi-pe" --enable-languages="c,c++" --with-newlib --without-headers --program-prefix=cross-

make all-gcc
make install-gcc

Install grub2
Install xorriso

Notes on running.

qemu-system-x86_64 -cdrom os.iso -serial stdio -display none -s
