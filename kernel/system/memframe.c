
#include <mem.h>
#include <x86_64.h>
#include <interrupt.h>

/*
 * Page frame allocator.
 *
 * Simple implementation.
 *
 */

typedef struct region {
	struct region *next, *prev;
	uintptr_t phys;
	size_t len;
} region_t;

static region_t *free;

void
region_free(uintptr_t phys, size_t len)
{
	uint64_t flags;
	region_t *r;
   
	cli_ifsave(&flags);

	r = kmalloc(sizeof(region_t));
	r->phys = phys;
	r->len = len;

	if (free == NULL) {
		r->next = r->prev = NULL;
		free = r;
	} else if (r->phys < free->phys) {
		r->prev = NULL;
		r->next = free;
		free->prev = r;
		free = r;
	} else {
		region_t *c = free;

		while (c->next != NULL && c->next->phys < r->phys)
			c = c->next;

		r->prev = c;
		r->next = c->next;
		c->next = r;
	}

	ifrestore(flags);
}

uintptr_t
frame_alloc(void)
{
	uint64_t flags;
	uintptr_t phys = 0;
	region_t *c;

	cli_ifsave(&flags);

	c = free;
	while (c != NULL) {
		if (c->len >= FRAME_LEN) {
			phys = c->phys;
			c->len -= FRAME_LEN;
			c->phys += FRAME_LEN;

			if (c->len == 0) {
				if (c == free)
					free = c->next;

				if (c->next != NULL)
					c->next->prev = c->prev;
				if (c->prev != NULL)
					c->prev->next = c->next;

				kfree(c);
			}
			break;
		} else {
			c = c->next;
		}
	}

	ifrestore(flags);
	return phys;
}

void
frame_free(uintptr_t phys)
{
	uint64_t flags;
	cli_ifsave(&flags);

	region_free(phys, FRAME_LEN);

	ifrestore(flags);
}
