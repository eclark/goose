#ifndef _X86_64_H
#define _X86_64_H

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define KERNEL_ESP 0x200000 /* Bad news */
#define HIGH_HALF 0xffff800000000000

/* Initial kernel heap to use before the memory subsystem can assign
 * physical pages */
#define KERNEL_HEAP_ADDR 0x200000
#define KERNEL_HEAP_SIZE 0x200000

#define VIRTUAL(addr) (void*)((uint64_t)(addr) | HIGH_HALF)
#define ALIGN(addr, nbytes) (void*)(((uint64_t)(addr) + (nbytes) - 1) & ~((nbytes) -1))

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

extern uint64_t boot_pml4;

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
rdmsr(uint32_t ecx)
{
	uint64_t ret;
	uint32_t edx, eax;

	asm volatile ("rdmsr" : "=d"(edx),"=a"(eax) : "c"(ecx));

	ret = edx;
	ret <<= 32;
	ret |= eax;

	return ret;
}

#endif
#endif
