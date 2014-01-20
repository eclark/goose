/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */

#include "vc.h"
#include <x86_64.h>
#include <mem.h>

static ssize_t vc_write(chardev_t *dev, const char *buf, size_t nbyte);
static int vc_putchar(int ch);

chardev_t vcdev = { NULL, vc_write };

uint16_t *vga = VIRTUAL(0xb8000);

static uint16_t cx, cy;
static uint16_t cattr = 0x3000;
static uint64_t cattrq = 0x3000300030003000;

void
vc_clear(void)
{
	memset_quad(vga, cattrq, 20*25);
	cx = 0;
	cy = 0;
}

static ssize_t
vc_write(chardev_t *dev, const char *buf, size_t nbyte)
{
	size_t i;
	for (i=0; i < nbyte; i++)
		vc_putchar(*(buf++));

	return i;
}

static int
vc_putchar(int ch)
{
	if (ch == '\n') {
		cy++;
		cx = 0;
	} else {
		vga[cy*80 + cx] = (cattr & 0xff00) | ch;

		if (++cx == 80)
			cy++;
	}
	if (cy == 25) {
		memmove_quad(vga, vga + 80, 20*24);
		memset_quad(vga + 80*24, cattrq, 20);
		cy--;
	}
	cx %= 80;
	cy %= 25;

	return ch;
}

