#ifndef _VC_H
#define _VC_H

#include <stdint.h>

extern uint16_t *vga;

void clear(void);
void putc(const char ch);
void puts(const char *str);

#endif
