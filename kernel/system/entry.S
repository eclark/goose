# Entry point when coming from GRUB2

#define ASM_FILE 1

#include <multiboot2.h>
#include <x86_64.h>

.globl entry, multiboot_info

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
	cli

	movl	%eax, multiboot_info
	movl	%ebx, multiboot_info + 4

	# set up the early 32-bit gdt and jump to it
	lgdt	earlygdtr
	ljmp	$KERNEL_CS, $_entry2

_entry2:
	movl	$KERNEL_DS, %eax
	movw	%ax, %ds
	movw	%ax, %ss
	movl	$KERNEL_ESP, %esp

	# now we have a minimal 32-bit protected mode

	# see if this cpu even support x86-64
	call	_x86_64_check

	# set up pd, pdpt, pml4 and link them together
	movl	$0x83, phys_pd
	movl	$(0x83 + 1*(1<<21)), phys_pd + 8

	movl	$(phys_pd + 0x03), phys_pdpt
	movl	$(phys_pd + 0x03), (phys_pdpt + 0xff0)
	movl	$(phys_pdpt + 0x03), phys_pml4
	movl	$(phys_pdpt + 0x03), (phys_pml4 + 0xff8)

	# set pml4
	movl	$phys_pml4, %eax
	movl	%eax, %cr3

	# enable PAE
	movl	%cr4, %eax
	orl 	$0x20, %eax
	movl	%eax, %cr4

	# enable long mode
	movl	$0xc0000080, %ecx
	rdmsr
	orl 	$(1 << 8), %eax
	wrmsr

	# enable paging
	movl	%cr0, %eax
	orl 	$(1<<31), %eax
	movl	%eax, %cr0

	lgdt	gdtr64l

	# jump to long mode
	ljmp	$KERNEL_CS, $_entry3

# CPU Check, never returns if not 64-bit capable.
#  Clobber: eax, ebx, ecx, edx
_x86_64_check:
	movl	$0x80000001, %eax
	cpuid
	testl	$(1<<29), %edx
	jz  	1f

	ret
1:
	cld
	movw	$0x4700, %ax
	movl	$2000, %ecx
	movl	$0xb8000, %edi
	rep stosw

	movl	$(0xb8000 + 80*10), %edi

	movl	$cpu_warning, %esi

3:
	movb	(%esi), %bl
	movb	%bl, (%edi)

	incl	%esi
	addl	$2, %edi
	cmpb	$0, (%esi)
	jnz 	3b

2:
	hlt
	jmp 	2b


.code64
_entry3:
	movl	$KERNEL_DS, %eax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	movq	$(KERNEL_ESP + HIGH_HALF), %rsp
	movq	$0, %rbp

	# move up to high half gdtr
	lgdt	gdtr64h

	# now we have high-half long mode
	jmp 	entry64

.data

multiboot_info:
	.long	0
	.long	0

cpu_warning:
	.asciz "Not a 64-bit cpu."

	.align 8
earlygdt:
	.quad	0
	# EARLY_CS
	.quad	0x00cf9a000000ffff
	# EARLY_DS
	.quad	0x00cf92000000ffff
earlygdt_end:

gdt:
	.quad	0
	# KERNEL_CS
	.quad	0x00a09a000000ffff
	# KERNEL_DS
	.quad	0x00a0920000000000
gdt_end:

earlygdtr:
	.word	earlygdt_end - earlygdt - 1
	.long	earlygdt

gdtr64l:
	.word	gdt_end - gdt - 1
	.long	gdt

gdtr64h:
	.word	gdt_end - gdt - 1
	.quad	gdt + HIGH_HALF 
