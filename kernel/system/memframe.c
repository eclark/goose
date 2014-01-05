
#include <mem.h>
#include <x86_64.h>
#include <interrupt.h>

/*
 * Page frame allocator.
 *
 * Simple implementation.
 *
 */

static frame_t *build(uintptr_t start, size_t len, page_size_t ps);

/*static frame_t *free_huge;*/
static frame_t *free_large;
static frame_t *free_standard;

static frame_t *used_large;

frame_t *
frame_alloc(page_size_t ps)
{
	uint64_t flags;
	frame_t *frame;
	cli_ifsave(&flags);

	switch (ps) {
		case HUGE_PAGE:
			frame = NULL;
			break;
		case LARGE_PAGE:
			frame = framelist_remove(&free_large, free_large);
			break;
		case STANDARD_PAGE:
			if (free_standard == NULL) {
				frame = frame_alloc(LARGE_PAGE);
				if (frame == 0)
					break;
				framelist_add(&used_large, frame);
				frame->refcnt++;
				free_standard = build(frame->phys,
					page_size(LARGE_PAGE) / page_size(STANDARD_PAGE),
					STANDARD_PAGE);
			}
			frame = framelist_remove(&free_standard, free_standard);
			break;
	}

	ifrestore(flags);
	return frame;
}

frame_t *
frame_reserve(uintptr_t phys_addr, page_size_t ps)
{
	return 0;
}

void
frame_free(frame_t *f)
{
	frame_t **free;
	uint64_t flags;
	cli_ifsave(&flags);

	if (f == NULL)
		return;

	if (f->ps == LARGE_PAGE) {
		free = &free_large;
	} else if (f->ps == STANDARD_PAGE) {
		free = &free_standard;
	} else {
		/* TODO die */
	}

	framelist_add(free, f);

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

static frame_t *
build(uintptr_t start, size_t npages, page_size_t ps)
{
	size_t i;
	frame_t *f, *cp, *pp;

	f = pp = kmalloc(sizeof(frame_t));
	pp->prev = NULL;
	pp->phys = start;
	pp->refcnt = 0;
	pp->ps = ps;
	for (i = 1; i < npages; i++) {
		pp->next = cp = kmalloc(sizeof(frame_t));
		cp->prev = pp;
		cp->phys = start + i*page_size(ps);
		cp->refcnt = 0;
		cp->ps = ps;
		pp = cp;
	}
	pp->next = NULL;

	return f;
}
