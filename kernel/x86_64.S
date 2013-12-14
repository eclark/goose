
#define ASM_FILE 1

#include "x86_64.h"

.globl kernel_pml4, kernel_pdpt, kernel_pd,
.globl phys_pml4, phys_pdpt, phys_pd

.data

phys_pml4 = kernel_pml4 - HIGH_HALF 
phys_pdpt = kernel_pdpt - HIGH_HALF 
phys_pd   = kernel_pd   - HIGH_HALF 

.section .bss

.align 4096
kernel_pml4:
	.fill 512, 8, 0
kernel_pdpt:
	.fill 512, 8, 0
kernel_pd:
	.fill 512, 8, 0

