#ifndef _X86_64_H
#define _X86_64_H

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define KERNEL_ESP 0x200000
#define HIGH_HALF 0xffffffff80000000

#define VIRTUAL(addr) (addr + HIGH_HALF)
#define ALIGN(addr, nbytes) ((addr + nbytes - 1) & ~(nbytes -1))

#ifndef ASM_FILE

#include "types.h"

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

typedef struct {
	uint64_t entry[512];
} page_t;

extern page_t kernel_pml4;
extern page_t kernel_pdpt;
extern page_t kernel_pd;

void processor_brand(char buf[49]);

static inline void
cpuid(uint64_t *rax, uint64_t *rbx, uint64_t *rcx, uint64_t *rdx)
{
	asm volatile(
		"cpuid"
		: "=a"(*rax), "=b"(*rbx), "=c"(*rcx), "=d"(*rdx)
		: "a"(*rax), "b"(*rbx)
		:
	);
}

#endif
#endif
