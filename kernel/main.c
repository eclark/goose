
unsigned short cx, cy;
unsigned short *vga = (unsigned short*)0xb8000;

void
puts(const char *str)
{
	unsigned short c = 0x0700;

	while (*str != 0) {
		if (c == '\n') {
			cy++;
			cx = 0;
		} else {
			c = (c & 0xff00) | *str;

			vga[cy*80 + cx] = c;

			if (++cx == 80)
				cy++;
		}
		cx %= 80;
		cy %= 25;

		str++;
	}
}

void
main(void)
{
	int i;

	puts("Simple print");
}
