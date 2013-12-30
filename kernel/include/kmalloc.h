#ifndef _MALLOC_H
#define _MALLOC_H

#include <stddef.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t size);

void kbfree(void *p, size_t n);

#endif
