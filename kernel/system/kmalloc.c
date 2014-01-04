
#include <mem.h>
#include <x86_64.h>
#include <types.h>

#include <system.h>

/*
 * Simple memory allocator based on K&R section 8.7. It should be replaced
 * when the kernel becomes more advanced.
 */

typedef struct header {
	struct header *next;	/* next free block */
	size_t size;			/* size of block = size * sizeof(header_t) */
} header_t;

static header_t *morecore(size_t);

static header_t sentinel = { &sentinel, 0 };
static header_t *freep = &sentinel;

void *
kmalloc(size_t nbytes)
{
	header_t *p, *prevp;
	header_t *morecore(size_t);
	size_t nunits;

	nunits = (nbytes + sizeof(header_t) - 1) / sizeof(header_t) + 1;

	prevp = freep;
	for (p = prevp->next; ; prevp = p, p = p->next) {
		if (p->size >= nunits) {
			if (p->size == nunits) { /* Exact fit */
				prevp->next = p->next;
			} else { /* Take some units from the larger block */
				p->size -= nunits;
				p += p->size;
				p->size = nunits;
			}
			freep = prevp;
			return (void *)(p + 1);
		}

		if (p == freep && (p = morecore(nunits)) == NULL)
			return NULL;
	}

	return 0;
}

void
kfree(void *ap)
{
	header_t *bp, *p;

	bp = (header_t *)ap - 1;
	for (p = freep; !(bp > p && bp < p->next); p = p->next)
		if (p >= p->next && (bp > p || bp < p->next))
			break;

	if (bp + bp->size == p->next) {
		/* Join to upper block */
		bp->size += p->next->size;
		bp->next = p->next->next;
	} else {
		bp->next = p->next;
	}

	if (p + p->size == bp) {
		/* Join to lower block */
		p->size += bp->size;
		p->next = bp->next;
	} else {
		p->next = bp;
	}

	freep = p;
}

static header_t *
morecore(size_t nu)
{
	kprintf("shit.\n");

	while (1);

	return (header_t*)0;
}

void *
kcalloc(size_t nmemb, size_t size)
{
	return NULL;
}

void *
krealloc(void *ptr, size_t size)
{
	return NULL;
}

void
kbfree(void *p, size_t n)
{
	header_t *hp;

	if (n < 2*sizeof(header_t))
		return;

	hp = p;
	hp->size = n / sizeof(header_t);
	hp->next = freep->next->next;
	freep->next = hp;
}
