
#include <stdarg.h>
#include <system.h>

static const char * repr = "0123456789abcdefghijklmnopqrstuvwxyz"
						   "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char * ltox(unsigned long v, char *s);

chardev_t *kconsole;

size_t
strlen(const char *s)
{
	size_t i = 0;
	while (*(s++) != 0)
		i++;

	return i;
}

char *
strnrev(char *s, size_t nbytes)
{
	register int i;
	register char tmp;

	for (i = 0; i < (nbytes / 2); i++) {
		tmp = s[i];
		s[i] = s[nbytes-i-1];
		s[nbytes-i-1] = tmp;
	}

	return s;
}

int
kprintf(const char *format, ...)
{
	va_list ap;
	const char *pos = format;
	char buf[65];

	int char_mode, short_mode, long_mode, alt_mode;
	const char * args;
	unsigned long arg;
	
	va_start(ap, format);
	for (; *pos; pos++) {
		switch (*pos) {
			case '%':
				alt_mode = 0;
				char_mode = 0;
				short_mode = 0;
				long_mode = 0;
				next_spec:
				pos++;
				if (*pos != '%') {
					/* Handle format */
					switch (*pos) {
						case 's':
							args = va_arg(ap, const char *);
							kconsole->write(kconsole, args, strlen(args));
							break;
						case 'c': /* TODO broken */
							buf[0] = va_arg(ap, int);
							kconsole->write(kconsole, buf, 1);
						case 'd':
							if (long_mode) {
								arg = va_arg(ap, long);
							} else {
								arg = va_arg(ap, int);
							}
							ltoa(arg, buf, 10);
							kconsole->write(kconsole, buf, strlen(buf));
							break;
						case 'z':
							arg = va_arg(ap, size_t);
							ltoa(arg, buf, 10);
							kconsole->write(kconsole, buf, strlen(buf));
							break;
						case 'x':
							arg = va_arg(ap, long);
							ltox(arg, buf);
							args = buf;

							if (!long_mode)
								args += 8;

							if (short_mode)
								args += 4;

							if (char_mode)
								args += 6;

							if (!alt_mode)
								while (*args == '0' && *(args + 1) != 0)
									args++;

							kconsole->write(kconsole, args, strlen(args));
							break;
						case 'h':
							if (short_mode) {
								char_mode = 1;
								short_mode = 0;
							} else {
								short_mode = 1;
							}
							goto next_spec;
						case 'l':
							long_mode = 1;
							goto next_spec;
						case '#':
							alt_mode = 1;
							goto next_spec;
					}
					break;
				}
			default:
				kconsole->write(kconsole, pos, 1);
		}
	}
	va_end(ap);

	return -1;
}

static char *
ltox(unsigned long v, char *s)
{
	char *b = s;
	size_t i;

	for (i = 0; i < 2*sizeof(long); i++, b++) {
		*b = repr[v & 0xf];
		v >>= 4;
	}
	*b = 0;

	return strnrev(s, 2*sizeof(long));
}

char *
ltoa(long v, char *s, int radix)
{
	bool neg = false;
	register long uv = v;
	register int d;
	register int i;

	if (radix == 10 && v < 0) {
		neg = true;
		uv = -v;
	}

	i = 0;
	do {
		d = uv % radix;
		uv /= radix;

		s[i++] = repr[d];
	} while (uv != 0);

	if (neg)
		s[i++] = '-';

	s[i] = 0;
	return strnrev(s, i);
}
