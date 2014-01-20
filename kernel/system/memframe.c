
#include <mem.h>
#include <x86_64.h>

/*
 * Page frame allocator.
 *
 * Similar to K&R malloc but fixed sizes.
 *
 */

typedef struct region {
	struct region *next;
	uintptr_t phys;
	size_t nframes;
} region_t;

static region_t sentinel = { &sentinel, 0, 0 };
static region_t *freep = &sentinel;

uintptr_t
frame_alloc(size_t nframes)
{
	uintptr_t phys;
	region_t *p, *prevp;

	if (nframes == 0)
		return 0;

	prevp = freep;
	for (p = prevp->next; ; prevp = p, p = p->next) {
		if (p->nframes >= nframes) { /* perfect */
			if (p->nframes == nframes) {
				prevp->next = p->next;
				phys = p->phys;
				kfree(p);
			} else { /* Take part of a larger region */
				p->nframes -= nframes;
				phys = p->phys;
				p->phys += nframes * FRAME_LEN;
			}
			freep = prevp;
			return phys;
		}

		if (p == freep)
			return 0;
	}

	return 0;
}

void
frame_free(uintptr_t phys, size_t nframes)
{
	region_t *bp, *p;

	for (p = freep; !(phys > p->phys && phys < p->next->phys); p = p->next)
		if (p->phys >= p->next->phys && (phys > p->phys || phys < p->next->phys))
			break;

	if (phys + nframes * FRAME_LEN == p->next->phys) {
		/* Expand upper region */

		p->next->phys -= nframes * FRAME_LEN;
		p->next->nframes += nframes;
	} else if (p->phys + p->nframes * FRAME_LEN == phys) {
		/* Expand lower region */

		p->nframes += nframes;
	} else {
		/* Insert a new region */
		bp = kmalloc(sizeof(region_t));
		bp->phys = phys;
		bp->nframes = nframes;

		bp->next = p->next;
		p->next = bp;
	}

	freep = p;
}
