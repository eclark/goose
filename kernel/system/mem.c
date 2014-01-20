/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */

#include <mem.h>

void *
memcpy(void *dest, const void *src, size_t n)
{
	/* TODO Make faster */
	asm (
		"cld; rep movsb"
		:
		: "D"(dest), "S"(src), "c"(n)
		: "memory", "cc"
	);
	return dest;
}

void *
memmove(void *dest, const void *src, size_t n)
{
	asm (
		"	cld;"
		"	cmpq %%rdi, %%rsi;"
		"	jae 1f;"
		"	leaq -8(%%rsi, %%rcx), %%rsi;"
		"	leaq -8(%%rdi, %%rcx), %%rdi;"
		"	std;"
		"1:"
		"	rep movsb"
		:
		: "D"(dest), "S"(src), "c"(n)
		: "rdx", "memory", "cc"
	);

	return dest;
}

void *
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

void *
memmove_quad(void* dest, const void* src, size_t n)
{
	asm (
		"	cld;"
		"	cmpq %%rdi, %%rsi;"
		"	jae 1f;"
		"	leaq -8(%%rsi, %%rcx), %%rsi;"
		"	leaq -8(%%rdi, %%rcx), %%rdi;"
		"	std;"
		"1:"
		"	rep movsq"
		:
		: "D"(dest), "S"(src), "c"(n)
		: "rdx", "memory", "cc"
	);

	return dest;
}

