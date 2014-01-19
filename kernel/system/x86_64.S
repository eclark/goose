
#define ASM_FILE 1

#include <x86_64.h>

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

.globl phys_pml4, phys_pdpt, phys_pd0, phys_pd1, phys_pd2, phys_pd3, phys_pt0, phys_pt1, phys_ptstack, phys_ptheap

phys_pml4 = kernel_pml4 - HIGH_HALF
phys_pdpt = kernel_pdpt - HIGH_HALF
phys_pd0  = kernel_pd0  - HIGH_HALF
phys_pd1  = kernel_pd1  - HIGH_HALF
phys_pd2  = kernel_pd2  - HIGH_HALF
phys_pd3  = kernel_pd3  - HIGH_HALF
phys_pt0  = kernel_pt0  - HIGH_HALF
phys_pt1  = kernel_pt1  - HIGH_HALF
phys_ptstack = kernel_ptstack - HIGH_HALF
phys_ptheap = kernel_ptheap - HIGH_HALF

.section .bss

.align 4096
kernel_pml4:
	.fill 512, 8, 0
kernel_pdpt:
	.fill 512, 8, 0
kernel_pd0:
	.fill 512, 8, 0
kernel_pd1:
	.fill 512, 8, 0
kernel_pd2:
	.fill 512, 8, 0
kernel_pd3:
	.fill 512, 8, 0
kernel_pt0:
	.fill 512, 8, 0
kernel_pt1:
	.fill 512, 8, 0
kernel_ptstack:
	.fill 512, 8, 0
kernel_ptheap:
	.fill 512, 8, 0
