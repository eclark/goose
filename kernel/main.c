
unsigned short *vga = (unsigned short*)0xb8000;

void
main(void)
{
	int i;

	for (i = 0; i < 80*25; i++) {
		vga[i] = 0x0700 + ('a' + i % 26);
	}

}
