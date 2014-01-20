/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */
#ifndef _IOPORT_H
#define _IOPORT_H

#include "types.h"

static inline uint8_t
inb(uint16_t port)
{
	uint8_t val;
	asm volatile(
		"inb (%w1), %b0"
		: "=a"(val)
		: "d"(port)
		: "memory", "cc"
	);

	return val;
}

static inline uint16_t
inw(uint16_t port)
{
	uint16_t val;
	asm volatile(
		"inw (%w1), %b0"
		: "=a"(val)
		: "d"(port)
		: "memory", "cc"
	);

	return val;
}

static inline uint32_t
inl(uint16_t port)
{
	uint16_t val;
	asm volatile(
		"inl (%w1), %b0"
		: "=a"(val)
		: "d"(port)
		: "memory", "cc"
	);

	return val;
}

static inline void
outb(uint16_t port, uint8_t data)
{
	asm volatile(
		"outb %b1, (%w0)"
		:
		: "d"(port), "a"(data)
		: "memory", "cc"
	);
}

static inline void
outw(uint16_t port, uint16_t data)
{
	asm volatile(
		"outw %b1, (%w0)"
		:
		: "d"(port), "a"(data)
		: "memory", "cc"
	);
}

static inline void
outl(uint16_t port, uint32_t data)
{
	asm volatile(
		"outl %b1, (%w0)"
		:
		: "d"(port), "a"(data)
		: "memory", "cc"
	);
}

#endif
