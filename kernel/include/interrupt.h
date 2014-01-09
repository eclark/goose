#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <x86_64.h>

#define IRQBASEVEC 0x20

typedef int (*handler_t)(regs_t *regs);

int apic_init(void);
void bind_vector(uint8_t irq, handler_t f);
void clear_vector(uint8_t irq);
void enable_isa_irq(uint8_t);
void disable_isa_irq(uint8_t);

void send_eoi(uint8_t irq);

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
