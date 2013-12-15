
#include <vc.h>
#include <mem.h>

uint16_t *vga = (uint16_t*)0xffffffff800b8000;

static uint16_t cx, cy;
static uint16_t cattr = 0x3000;
static uint64_t cattrq = 0x3000300030003000;

void
clear(void)
{
	memset_quad(vga, cattrq, 20*25);
	cx = 0;
	cy = 0;
}

void
putc(const char ch)
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
}

void
puts(const char *str)
{
	while (*str != 0) {
		putc(*str);

		str++;
	}
}

void
puthex(uint64_t num, size_t n)
{
	size_t m = 2 * n;
	char nib;
	while (m--) {
		nib = (num >> (4 * m)) & 0xf;

		putc(nib < 10 ? nib + '0' : nib + 'a' - 10);
	}
}

