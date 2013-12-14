#ifndef _VC_H
#define _VC_H

#include <stddef.h>
#include <stdint.h>

extern uint16_t *vga;

void clear(void);
void putc(const char ch);
void puts(const char *str);
void puthex(uint64_t num, size_t n);

#endif
