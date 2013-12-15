
#define ASM_FILE 1

#include "x86_64.h"

.globl processor_brand

.text

processor_brand:
	pushq	%rbx

	movq	$0, %r10
	movq	$0x80000002, %r11
1:
	movq	%r11, %rax
	cpuid

	movl	%eax, (%rdi)
	movl	%ebx, 4(%rdi)
	movl	%ecx, 8(%rdi)
	movl	%edx, 12(%rdi)

	addq	$16, %rdi
	incq	%r10
	incq	%r11
	cmpq	$3, %r10
	jne  	1b

	popq	%rbx
	ret

.data

.globl kernel_pml4, kernel_pdpt, kernel_pd,
.globl phys_pml4, phys_pdpt, phys_pd


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

