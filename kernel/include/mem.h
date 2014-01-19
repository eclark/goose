#ifndef _MEM_H
#define _MEM_H

#include <types.h>
#include <kmalloc.h>

#define FRAME_LEN 4096

typedef struct frame {
	struct frame *next;
	struct frame *prev;
	uintptr_t phys;
	size_t refcnt;
} frame_t;

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset_quad(void *s, int64_t c, size_t n);
void *memmove_quad(void *dest, const void *src, size_t n);

void region_free(uintptr_t phys, size_t len);

frame_t *frame_alloc(void);
frame_t *frame_reserve(uintptr_t phys_addr);
void frame_free(frame_t *f);

void framelist_add(frame_t **head, frame_t *f);
frame_t *framelist_remove(frame_t **head, frame_t *f);

extern uintptr_t initrd_phys;
extern size_t initrd_len;

#endif
