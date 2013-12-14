
.globl kernel_pml4, kernel_pdpt, kernel_pd,
.globl phys_pml4, phys_pdpt, phys_pd

.data

phys_pml4 = kernel_pml4 - 0xffffffff80000000
phys_pdpt = kernel_pdpt - 0xffffffff80000000
phys_pd   = kernel_pd   - 0xffffffff80000000

.section .bss

.align 4096
kernel_pml4:
	.fill 512, 8, 0
kernel_pdpt:
	.fill 512, 8, 0
kernel_pd:
	.fill 512, 8, 0

