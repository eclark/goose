
#include <stddef.h>
#include <stdint.h>

#include "vc.h"
#include "mem.h"

void
main(void)
{
	unsigned int i;
	char *str = "Simple print  \n";
	clear();

	while (1) {
		str[13] = 'a' + (i % 26);

		puts(str);

		i++;
	}
}
