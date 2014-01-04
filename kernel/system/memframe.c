
#include <mem.h>
#include <x86_64.h>
#include <interrupt.h>

/*
 * Page frame allocator.
 *
 * Simple implementation, up to 128MB as qemu uses it.
 *
 */

static frame_t *build(uintptr_t start, size_t len, page_size_t ps);
static inline frame_t *push(frame_t **head, frame_t *f);
static inline frame_t *pop(frame_t **head);
static inline frame_t *remove(frame_t *f);

/*static frame_t *free_huge;*/
static frame_t *free_large;
static frame_t *free_standard;

static frame_t *used_large;

int
frame_init(void)
{
	/* TODO fix hard-coded qemu memory map 
	 * 0000000000000000 000000000009fc00 1
	 * 000000000009fc00 0000000000000400 2
	 * 00000000000f0000 0000000000010000 2
	 * 0000000000100000 0000000007efe000 1
	 * 0000000007ffe000 0000000000002000 2
	 * 00000000fffc0000 0000000000040000 2
	 */

	/* We reserved the 0 to 4MB in entry.S, so the first free is at 4MB */
	free_large = build(0x400000, (0x7ffe000 - 0x400000) / page_size(LARGE_PAGE), LARGE_PAGE);

	free_standard = NULL;
	used_large = NULL;

	return 0;
}

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
			frame = pop(&free_large);
			break;
		case STANDARD_PAGE:
			if (free_standard == NULL) {
				frame = frame_alloc(LARGE_PAGE);
				if (frame == 0)
					break;
				push(&used_large, frame);
				frame->refcnt++;
				free_standard = build(frame->phys,
					page_size(LARGE_PAGE) / page_size(STANDARD_PAGE),
					STANDARD_PAGE);
			}
			frame = pop(&free_standard);
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
	frame_t **free, *curr;
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

	if (*free == NULL) {
		push(free, f);
	} else {
		/* walk forward */
		for (curr = *free; curr->next != NULL && curr->phys < f->phys; curr = curr->next);

		/* insert before curr */
		f->prev = curr->prev;
		f->next = curr;
		if (curr->prev != NULL)
			curr->prev->next = f;
		curr->prev = f;

		/* Move the head if necessary */
		if (f->prev == NULL)
			*free = (*free)->prev;
	}

	ifrestore(flags);
}

void
framelist_add(frame_t **head, frame_t *f)
{


}

frame_t *
framelist_remove(frame_t **head, frame_t *f)
{


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

static inline frame_t *
push(frame_t **head, frame_t *f)
{
	f->next = f->prev = NULL;

	if (*head != NULL) {
		f->prev = NULL;
		f->next = *head;
		(*head)->prev = f;
	}
	*head = f;

	return f;
}

static inline frame_t *
pop(frame_t **head)
{
	frame_t *ret;

	if (head == NULL || *head == NULL)
		return NULL;

	ret = *head;
	*head = (*head)->next;
	if (*head != NULL)
		(*head)->prev = ret->prev;

	ret->prev = ret->next = NULL;

	return ret;
}

static inline frame_t *
remove(frame_t *f)
{
	if (f != NULL) {
		if (f->prev != NULL)
			f->prev->next = f->next;

		if (f->next != NULL)
			f->next->prev = f->prev;

		f->prev = f->next = NULL;
	}

	return f;
}
