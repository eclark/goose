#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "types.h"

int apic_init(void);
void enable_irq(int8_t);
void disable_irq(int8_t);

static inline void
cli(void)
{
	asm volatile("cli":::"memory", "cc");
}

static inline void
cli_ifsave(uint64_t *flags)
{
	asm volatile(
		"pushfq;"
		"popq %0;"
		"cli"
		: "=r"(*flags)
		:
		: "memory", "cc"
	);
}

static inline void
sti(void)
{
	asm volatile("sti":::"memory", "cc");
}

static inline void
ifrestore(uint64_t flags)
{
	asm volatile(
		"pushq %0;"
		"popfq;"
		:
		: "r"(flags)
		: "memory", "cc"
	);
}

#endif
