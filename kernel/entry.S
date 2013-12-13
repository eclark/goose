# Entry point when coming from GRUB2

#define ASM_FILE 1

#include "multiboot2.h"

.globl entry

.text
	.code32

	.align	MULTIBOOT_HEADER_ALIGN
multiboot_header:
	.long	MULTIBOOT2_HEADER_MAGIC
	.long	MULTIBOOT_ARCHITECTURE_I386 
	.long	multiboot_header_end - multiboot_header
	.long	-(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

address_tag_start:
	.short	MULTIBOOT_HEADER_TAG_ADDRESS
	.short	MULTIBOOT_HEADER_TAG_OPTIONAL
	.long	address_tag_end - address_tag_start

	.long	multiboot_header
	.long	_loadStart
	.long	_loadEnd
	.long	_bssEnd
address_tag_end:

entry_address_tag_start:
	.short	MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
	.short	MULTIBOOT_HEADER_TAG_OPTIONAL
	.long	entry_address_tag_end - entry_address_tag_start
	.long	entry
entry_address_tag_end:

	.long	MULTIBOOT_HEADER_TAG_END
	.long	0
	.long	8

multiboot_header_end:

	.align 4
entry:

	hlt
	jmp entry

