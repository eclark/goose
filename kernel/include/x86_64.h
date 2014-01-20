/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */
#ifndef _X86_64_H
#define _X86_64_H

#define CURRENT_PML4 0xfffffffffffff000
#define HIGH_HALF 0xffff800000000000

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define KERNEL_EARLY_ESP 0x0009f000
#define KERNEL_ESP 0x40002000

/* Initial kernel heap to use before the memory subsystem can assign
 * physical pages */
#define KERNEL_HEAP_ADDR 0x300000
#define KERNEL_HEAP_SIZE 0x100000

#define VIRTUAL(addr) (void*)((uint64_t)(addr) | HIGH_HALF)
#define ALIGN(addr, nbytes) (void*)(((uint64_t)(addr) + (nbytes) - 1) & ~((nbytes) -1))

/* Model specific registers */
#define IA32_APIC_BASE 0x1b
#define IA32_PAT 0x277
#define IA32_EFER 0xc0000080
#define IA32_STAR 0xc0000081
#define IA32_LSTAR 0xc0000082
#define IA32_FMASK 0xc0000084

#ifndef ASM_FILE

#include <types.h>

typedef struct {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;

	uint64_t vector;
	uint64_t error_code;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} __attribute__((packed)) regs_t;

void processor_brand(char buf[49]);

static inline void
cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
	asm volatile(
		"cpuid"
		: "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
		: "a"(*eax), "b"(*ebx)
		:
	);
}

static inline uint64_t
rdmsr(uint32_t msr)
{
	uint64_t ret;
	uint32_t edx, eax;

	asm volatile("rdmsr" : "=d"(edx),"=a"(eax) : "c"(msr));

	ret = edx;
	ret <<= 32;
	ret |= eax;

	return ret;
}

static inline void
wrmsr(uint32_t msr, uint64_t val)
{
	uint32_t edx, eax;
	edx = val >> 32;
	eax = val & 0xffffffff;

	asm volatile("wrmsr" : : "c"(msr), "d"(edx), "a"(eax));
}

#endif
#endif
