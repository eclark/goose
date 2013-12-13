# Entry point when coming from GRUB2

#define ASM_FILE 1

#include "multiboot2.h"

.globl entry64

entry64:

	callq	main

_entry64_halt:
	hlt
	jmp _entry64_halt


