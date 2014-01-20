/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */
#ifndef _MEM_H
#define _MEM_H

#include <types.h>
#include <kmalloc.h>

#define FRAME_LEN 4096

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset_quad(void *s, int64_t c, size_t n);
void *memmove_quad(void *dest, const void *src, size_t n);

uintptr_t frame_alloc(size_t nframes);
void frame_free(uintptr_t phys, size_t nframes);

extern uintptr_t initrd_phys;
extern size_t initrd_len;

#endif
