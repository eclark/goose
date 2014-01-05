#ifndef _MEM_H
#define _MEM_H

#include <types.h>
#include <kmalloc.h>

typedef enum {
	STANDARD_PAGE = 1,
	LARGE_PAGE = 2,
	HUGE_PAGE = 4
} page_size_t;

typedef struct frame {
	struct frame *next;
	struct frame *prev;
	uintptr_t phys;
	size_t refcnt;
	page_size_t ps;
} frame_t;

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

frame_t *frame_alloc(page_size_t ps);
frame_t *frame_reserve(uintptr_t phys_addr, page_size_t ps);
void frame_free(frame_t *f);

void framelist_add(frame_t **head, frame_t *f);
frame_t *framelist_remove(frame_t **head, frame_t *f);

#endif
