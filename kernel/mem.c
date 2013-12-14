
#include "mem.h"

void*
memset_quad(void *s, int64_t c, size_t n)
{
	asm (
		"cld; rep stosq"
		:
		: "a"(c), "D"(s), "c"(n)
		: "memory", "cc"
	);

	return s;
}

void*
memmove_quad(void* dest, const void* src, size_t n)
{
	asm (
		"	cld;"
		"	cmpq %%rdi, %%rsi;"
		"	jae 1f;"
		"	leaq -8(%%rsi, %%rcx), %%rsi;"
		"	leaq -8(%%rdi, %%rcx), %%rdi;"
		"	std;"
		"	1:"
		"	rep movsq"
		:
		: "D"(dest), "S"(src), "c"(n)
		: "rdx", "memory", "cc"
	);

	return dest;
}

