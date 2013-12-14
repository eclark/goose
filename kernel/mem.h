#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>
#include <stdint.h>

void* memset_quad(void *s, int64_t c, size_t n);
void* memmove_quad(void* dest, const void* src, size_t n);

#endif
