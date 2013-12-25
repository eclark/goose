#ifndef _MEM_H
#define _MEM_H

#include <types.h>

typedef enum {
	STANDARD_PAGE = 0,
	LARGE_PAGE = 1,
	HUGE_PAGE = 2
} page_size_t;

static inline size_t
page_size(page_size_t ps)
{
	/* Possible page sizes for IA-32e */
	switch (ps) {
		case STANDARD_PAGE:
			return 1 << 12;
		case LARGE_PAGE:
			return 1 << 21;
		case HUGE_PAGE:
			return 1 << 30;
	}
	return 0;
}

static inline void
flush_tlb(void)
{
	asm volatile (
		"movq %%cr3, %%rax; movq %%rax, %%cr3"
		::: "rax", "memory", "cc"
	);
}

void *map_page(unsigned long phys, page_size_t ps);

void *memcpy(void *dest, const void *src, size_t n);
void *memset_quad(void *s, int64_t c, size_t n);
void *memmove_quad(void* dest, const void* src, size_t n);

#endif
