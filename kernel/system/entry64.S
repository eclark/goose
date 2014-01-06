# Entry point when coming from GRUB2

#define ASM_FILE 1

#include <x86_64.h>
#include <multiboot2.h>

.globl entry64

.text

entry64:
	# unmap low half
	movabs	$kernel_pml4, %rax
	movq	$0, (%rax)

	# flush tlb
	movq	$phys_pml4, %rax
	movq	%rax, %cr3

	# build IDT that sends everything to one C function
	# See Figure 6-7. Intel Volume 3
	movabs	$interrupt_linkage, %rax
	shrq	$32, %rax

	movabs	$interrupt_linkage, %rbx
	shlq	$32, %rbx
	movq	$0xffff000000000000, %rdx
	andq	%rdx, %rbx

	movabs	$interrupt_linkage,	%rdx
	andq	$0x0000ffff, %rdx
	orq 	%rdx, %rbx

	orq 	$(KERNEL_CS << 16), %rbx
	movq	$0x00008e0000000000, %rdx
	orq		%rdx, %rbx

	movq	$256, %rcx
	movabs	$idt, %rdx
1:
	movq	%rbx, (%rdx)
	movq	%rax, 8(%rdx)
	addq	$16, %rdx
	addq	$16, %rbx
	decq	%rcx
	jnz 1b

	movabs	$idtr, %rax
	lidt	(%rax)

	movq	$HIGH_HALF, %rdi
	movq	%rdi, %rsi

	addq	$multiboot_info, %rdi
	addq	$(multiboot_info + 4), %rsi

	movq	(%rdi), %rdi
	movq	(%rsi), %rsi

	# work around gdb mode switch issues when remote debugging

#ifdef GDBSTOP
	movq	$0, %rax
dloop:
	pause
	testq	%rax, %rax
	jz dloop
#endif

	callq	main

_entry64_halt:
	hlt
	jmp _entry64_halt

.macro PUSHALL
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%r11
	pushq	%r10
	pushq	%r9
	pushq	%r8
	# skip %rsp
	pushq	%rbp
	pushq	%rsi
	pushq	%rdi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%rax
.endm

.macro POPALL
	popq	%rax
	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rdi
	popq	%rsi
	popq	%rbp
	# skip %rsp
	popq	%r8
	popq	%r9
	popq	%r10
	popq	%r11
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
.endm

# linkage to get an IDT gate to C
	.align 4096
interrupt_linkage:
	vector=0
	.rept 256
	.align 16

	.ifeq vector == 8 || (-(vector >= 10) && -(vector <= 14)) || vector == 17
	pushq	$-1
	.endif
	pushq	$vector
	jmp		interrupt_cont

	vector=vector+1
	.endr

interrupt_cont:
	PUSHALL

	movq	$0, %rbp

	# save rsp before we align it
	movq	%rsp, %r15
	# align stack
	andq	$~0xf, %rsp

	# interrupt(regs_t*)
	movq	%r15, %rdi
	callq	interrupt

	# remove alignment
	movq	%r15, %rsp

	POPALL
	addq	$16, %rsp
	iretq

idtr:
	.word idt_end - idt - 1
	.quad idt

.bss

	.align 8
idt:
	.fill 512, 8, 0
idt_end:

