
#include <mem.h>
#include <log.h>
#include "mmu.h"

/*
 * Simple memory allocator based on K&R section 8.7. It should be replaced
 * when the kernel becomes more advanced.
 */

static pool_t kmalloc_pool;

void
pool_init(pool_t *pool, header_t *(*morecore)(size_t))
{
	assert(pool != NULL);

	pool->sentinel.next = &pool->sentinel;
	pool->sentinel.size = 0;
	pool->freep = &pool->sentinel;
	pool->morecore = morecore;
}

/* Don't call any allocator functions in an interrupt handler */
void *
kpmalloc(pool_t *pool, size_t nbytes)
{
	header_t *p, *prevp;
	size_t nunits;

	assert(pool != NULL);

	nunits = (nbytes + sizeof(header_t) - 1) / sizeof(header_t) + 1;

	prevp = pool->freep;
	for (p = prevp->next; ; prevp = p, p = p->next) {
		if (p->size >= nunits) {
			if (p->size == nunits) { /* Exact fit */
				prevp->next = p->next;
			} else { /* Take some units from the larger block */
				p->size -= nunits;
				p += p->size;
				p->size = nunits;
			}
			pool->freep = prevp;
			return (void *)(p + 1);
		}

		if (p == pool->freep && (p = pool->morecore(nunits)) == NULL)
			return NULL;
	}

	return 0;
}

void
kpfree(pool_t *pool, void *ap)
{
	header_t *bp, *p;

	assert(pool != NULL);

	bp = (header_t *)ap - 1;
	for (p = pool->freep; !(bp > p && bp < p->next); p = p->next)
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

	pool->freep = p;
}

void
kpbfree(pool_t *pool, void *p, size_t n)
{
	header_t *hp;

	assert(pool != NULL);

	if (n < 2*sizeof(header_t))
		return;

	hp = p;
	hp->size = n / sizeof(header_t);
	hp->next = pool->freep->next->next;
	pool->freep->next = hp;
}

static header_t *
kmalloc_morecore(size_t nu)
{
	uintptr_t fp;
	void *mem;
	size_t i, npages;

	klogf(LOG_INFO, "kmalloc_morecore, %d bytes.\n", nu * sizeof(header_t));

	//nunits = (nbytes + sizeof(header_t) - 1) / sizeof(header_t) + 1;
	npages = (nu * sizeof(header_t) + FRAME_LEN - 1) / FRAME_LEN;

	/* kmalloc doesn't guarantee physically contiguous blocks, so we want
	 * to ask for one page at a time from the frame allocator so we can
	 * take up the small regions in the free frame list. */
	fp = frame_alloc(1);
	if (fp == 0)
		return NULL;
	mem = mmu_mapregion(fp, HEAP);
	for (i = 1; i < npages && fp != 0; i++) {
		if ((fp = frame_alloc(1)) == 0)
			break;
		mmu_mapregion(fp, HEAP);
	}

	kbfree(mem, i * FRAME_LEN);

	if (i == npages)
		return kmalloc_pool.freep;
	else
		return NULL;
}

void
kmalloc_init(void)
{
	pool_init(&kmalloc_pool, &kmalloc_morecore);
}

void *
kmalloc(size_t nbytes)
{
	return kpmalloc(&kmalloc_pool, nbytes);
}

void
kfree(void *ap)
{
	kpfree(&kmalloc_pool, ap);
}

void
kbfree(void *p, size_t n)
{
	kpbfree(&kmalloc_pool, p, n);
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
