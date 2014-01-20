#ifndef _MALLOC_H
#define _MALLOC_H

#include <stddef.h>

typedef struct header {
	struct header *next;	/* next free block */
	size_t size;			/* size of block = size * sizeof(header_t) */
} header_t;

typedef struct {
	header_t *freep;
	header_t *(*morecore)(size_t);
	header_t sentinel;
} pool_t;

void pool_init(pool_t *pool, header_t *(*morecore)(size_t));
void *kpmalloc(pool_t *pool, size_t size);
void kpfree(pool_t *pool, void *ptr);
void kpbfree(pool_t *pool, void *p, size_t n);

void kmalloc_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t size);

void kbfree(void *p, size_t n);

#endif
