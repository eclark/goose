
#include <system.h>
#include "tar.h"

size_t
tar_size(struct header_gnu_tar *p)
{
	size_t size = 0;
	int i;

	for (i = 0; i < 12 && p->size[i] != 0; i++)
		size = (size << 3) | (p->size[i] - '0');

	return size;
}

void
tar_demo(uintptr_t addr)
{
	int f;
	size_t siz;
	struct header_gnu_tar *p = (void*)addr;
	char *file = (char*)p + 512;

	while (*(char*)p != 0) {
		f = p->typeflag[0];
		siz = tar_size(p);
		kprintf("%s %s %d\n", p->name, &f, siz);
/*		if (siz > 0)
			kprintf("%s\n", file);*/

		p = (void*)(((uintptr_t)p + 1023 + siz) & ~0x1ff);
		file = (char*)p + 512;
	}
}
