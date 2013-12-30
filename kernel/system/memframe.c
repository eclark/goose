
#include <mem.h>
#include <x86_64.h>
#include <interrupt.h>

/*
 * Page frame allocator.
 *
 * Simple implementation, up to 128MB as qemu uses it.
 *
 */

/* 32 byte frame tracking */
typedef struct frame {
	struct frame *next;
	struct frame *prev;
	uintptr_t addr;
	size_t refcnt;
} frame_t;

static frame_t *build(uintptr_t start, size_t len, page_size_t ps);
static inline frame_t *push(frame_t **head, frame_t *f);
static inline frame_t *remove(frame_t *f);

/*static frame_t *free_huge;*/
static frame_t *free_large;
static frame_t *free_standard;

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
	free_large = build(0x400000, 0x7ffe000, LARGE_PAGE);

	return 0;
}

uintptr_t
frame_alloc(page_size_t ps)
{
	uint64_t flags;
	frame_t *frame;
	uintptr_t addr;
	cli_ifsave(&flags);

	switch (ps) {
		case HUGE_PAGE:
			frame = NULL;
			break;
		case LARGE_PAGE:
			frame = remove(free_large);
			break;
		case STANDARD_PAGE:
			if (free_standard == NULL) {
				addr = frame_alloc(LARGE_PAGE);
				if (addr == 0)
					break;
				free_standard = build(addr, page_size(LARGE_PAGE), STANDARD_PAGE);
			}
			frame = remove(free_standard);
			break;
	}

	ifrestore(flags);
	return frame->addr;
}

uintptr_t
frame_reserve(uintptr_t phys_addr, page_size_t ps)
{
	return 0;
}

void
frame_free(uintptr_t phys_addr)
{

}

static frame_t *
build(uintptr_t start, size_t len, page_size_t ps)
{
	frame_t *curr, *frame;

	frame = curr = (void*)start;
	frame->prev = NULL;

	while ((uintptr_t)curr + page_size(ps) <= start + len) {
		curr->next = (frame_t*)((uintptr_t)curr + page_size(ps));
		curr->next->prev = curr;

		curr = curr->next;
	}
	curr->next = NULL;

	return frame;
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
