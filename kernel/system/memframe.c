
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
static frame_t *used;

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

frame_t *
frame_alloc(void)
{
	uint64_t flags;
	frame_t *frame;
	region_t *c;

	cli_ifsave(&flags);

	c = free;
	frame = NULL;

	while (c != NULL) {
		if (c->len >= FRAME_LEN) {
			frame = kmalloc(sizeof(frame_t));
			frame->phys = c->phys;
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
	return frame;
}

frame_t *
frame_reserve(uintptr_t phys_addr)
{
	return 0;
}

void
frame_free(frame_t *f)
{
	uint64_t flags;
	cli_ifsave(&flags);

	region_free(f->phys, FRAME_LEN);
	kfree(f);

	ifrestore(flags);
}

void
framelist_add(frame_t **head, frame_t *f)
{
	f->next = f->prev = NULL;

	if (*head != NULL) {
		f->prev = NULL;
		f->next = *head;
		(*head)->prev = f;
	}
	*head = f;
}

frame_t *
framelist_remove(frame_t **head, frame_t *f)
{
	if (head == NULL)
		return NULL;

	if (f != NULL) {
		if (f->prev != NULL)
			f->prev->next = f->next;

		if (f->next != NULL)
			f->next->prev = f->prev;

		if (*head == f)
			*head = f->next;

		f->prev = f->next = NULL;
	}

	return f;
}

