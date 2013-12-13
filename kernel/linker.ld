/*
 * Based on the example at:
 * http://wiki.osdev.org/Creating_a_64-bit_kernel
 *
 */

ENTRY(entry)

KERNEL_LMA = 0x100000;
KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
	. = KERNEL_LMA;

	.bootstrap :
	{
		_loadStart = .;
		entry.o (.text)
		entry.o (.data)

	}

	. += KERNEL_VMA;

	.text : AT(ADDR(.text) - KERNEL_VMA)
	{
		_code = .;
		*(.text)
		*(.rodata*)
		. = ALIGN(4096);
	}

	.data : AT(ADDR(.data) - KERNEL_VMA)
	{
		_data = .;
		*(.data)
		. = ALIGN(4096);
	}

	.eh_frame : AT(ADDR(.eh_frame) - KERNEL_VMA)
	{
		_ehframe = .;
		*(.eh_frame)
		. = ALIGN(4096);
	}

	_loadEnd = . - KERNEL_VMA;

	.bss : AT(ADDR(.bss) - KERNEL_VMA)
	{
		_bss = .;
		*(.bss)

		/*
		 * You usually need to include generated COMMON symbols
		 * under kernel BSS section or use gcc's -fno-common
		 */

		*(COMMON)
		. = ALIGN(4096);
	}

	_bssEnd = . - KERNEL_VMA;

	_end = .;

	/DISCARD/ :
	{
		*(.comment)
	}
}
