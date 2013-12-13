
.globl kernel_pml4, kernel_pdpt, kernel_pd

.data



.section .bss

.align 4096
kernel_pml4:
	.rept 512
	.quad 0
	.endr
kernel_pdpt:
	.rept 512
	.quad 0
	.endr
kernel_pd:
	.rept 512
	.quad 0
	.endr
