
#include <mem.h>
#include <x86_64.h>
#include <interrupt.h>

#define MASK_ADDR 0x000ffffffffff000
#define MASK_1GB  0x000fffffc0000000
#define MASK_2MB  0x000fffffffe00000
#define MASK_4KB  MASK_ADDR
#define MASK_RSVD 0xfff0000000000000

#define PML4_IDX(x) (((x) >> 39) & 0x1ff)
#define PDPT_IDX(x) (((x) >> 30) & 0x1ff)
#define   PD_IDX(x) (((x) >> 21) & 0x1ff)
#define   PT_IDX(x) (((x) >> 12) & 0x1ff)

#define PHYSICAL(ptr) (lookup_phys((void*)(ptr)))

/* Aligned down */
#define SET_ADDR(pe, addr, mask) (((pe)->v) = (((pe)->v & ~(mask)) | ((addr) & (mask))))

/* See Intel manual pg. 1965 */
typedef struct {
	union {
		uint64_t v;
		struct {
			uint64_t p : 1;		/* Present */
			uint64_t rw : 1;	/* Writable */
			uint64_t us : 1;	/* User */
			uint64_t pwt : 1;	/* Write-through */
			uint64_t pcd : 1;	/* Cache-disable */
			uint64_t a : 1;		/* Accessed */
			uint64_t d : 1;		/* Dirty */
			uint64_t ps : 1; 	/* Page Size or PAT for 4kb pages */
			uint64_t g : 1;		/* Global */
			uint64_t ign0 : 3;
			uint64_t pat : 1;	/* See section 4.9.2 */
			uint64_t rsvd : 50;
			uint64_t xd : 1;	/* Execute Disable */
		} __attribute__((packed));
	};
} page_entry_t;

typedef struct page_struct {
	page_entry_t entry[512];
} page_table_t;

static uint64_t lookup_phys(void *object);
static page_table_t *page_table_alloc(void);
static void page_table_free(page_table_t *p);

static int next_free();
#define POOL_SIZE 512
#define POOLMAP_SIZE (POOL_SIZE / (sizeof(unsigned long) * 8))
#define MARK_FREE(n) (poolmap[(n) >> 3] |= 1 << ((n) & 0x7))
#define MARK_USED(n) (poolmap[(n) >> 3] &= ~(1 << ((n) & 0x7)))

static unsigned long poolmap[POOLMAP_SIZE] = {
	0xffffffffffffffff,
	0xffffffffffffffff,
	0xffffffffffffffff,
	0xffffffffffffffff,
	0xffffffffffffffff,
	0xffffffffffffffff,
	0xffffffffffffffff,
	0xffffffffffffffff
};
static page_table_t pool[POOL_SIZE] __attribute__((aligned(4096)));

void *
map_page(unsigned long phys, page_size_t ps)
{
	page_table_t *pml4, *pdpt, *pd, *pt;
	page_entry_t *pml4e, *pdpte, *pde, *pte;
	uint64_t flags;
	void *ret = VIRTUAL(phys);
	uint64_t virt = (uint64_t)ret;

	cli_ifsave(&flags);

	pml4 = VIRTUAL(boot_pml4);
	pml4e = pml4->entry + PML4_IDX(virt);

	if (!pml4e->p) {
		pdpt = page_table_alloc();
		pml4e->v = PHYSICAL(pdpt) & MASK_ADDR;
		pml4e->p = 1;
		pml4e->rw = 1;
	} else {
		pdpt = VIRTUAL(pml4e->v & MASK_ADDR);
	}
	pdpte = pdpt->entry + PDPT_IDX(virt);

	if (ps == HUGE_PAGE) {
		if (pdpte->p)
			goto error;

		pdpte->v = phys & MASK_1GB;
		pdpte->p = 1;
		pdpte->rw = 1;
		pdpte->ps = 1;

		goto end;
	}

	if (!pdpte->p) {
		pd = page_table_alloc();
		pdpte->v = PHYSICAL(pd) & MASK_ADDR;
		pdpte->p = 1;
		pdpte->rw = 1;
	} else if (pdpte->ps) {
		goto error;
	} else {
		pd = VIRTUAL(pdpte->v & MASK_ADDR);
	}
	pde = pd->entry + PD_IDX(virt);

	if (ps == LARGE_PAGE) {
		if (pde->p)
			goto error;

		pde->v = phys & MASK_2MB;
		pde->p = 1;
		pde->rw = 1;
		pde->ps = 1;

		goto end;
	}

	if (!pde->p) {
		pt = page_table_alloc();
		pde->v = PHYSICAL(pt) & MASK_ADDR;
		pde->p = 1;
		pde->rw = 1;
	} else if (pde->ps) {
		goto error;
	} else {
		pt = VIRTUAL(pde->v & MASK_ADDR);
	}
	pte = pt->entry + PT_IDX(virt);

	if (ps == STANDARD_PAGE) {
		if (pte->p)
			goto error;

		pte->v = phys & MASK_4KB;
		pte->p = 1;
		pte->rw = 1;

		goto end;
	}

error:
	ret = 0;
end:
	ifrestore(flags);

	return ret;
}

static page_table_t *
page_table_alloc(void)
{
	int n = next_free();

	if (n < 0)
		return NULL;

	MARK_USED(n);

	memset_quad(pool + n, 0, sizeof(page_table_t) / sizeof(uint64_t));

	return pool + n;
}

static void
page_table_free(page_table_t *p)
{
	int n = (p - pool) / sizeof(page_table_t);

	MARK_FREE(n);
}

static int
next_free()
{
	int i;

	for (i = 0; i < POOLMAP_SIZE && poolmap[i] == 0; i++);

	if (i == POOLMAP_SIZE)
		return -1;

	/* Find the index of the first 1 bit, that is the one that is free */
	return (i << (sizeof(unsigned long) == 8 ? 6 : 5)) | __builtin_ctzl(poolmap[i]);
}

static uint64_t
lookup_phys(void *object)
{
	page_table_t *pml4, *pdpt, *pd, *pt;
	page_entry_t *pml4e, *pdpte, *pde, *pte;
	uint64_t virt = (uint64_t)VIRTUAL(object);

	pml4 = VIRTUAL(boot_pml4);
	pml4e = pml4->entry + PML4_IDX(virt);

	if (pml4e->p) {
		pdpt = VIRTUAL(pml4e->v & MASK_ADDR);
	} else {
		return 0;
	}
	pdpte = pdpt->entry + PDPT_IDX(virt);

	if (pdpte->ps) {
		return (pdpte->v & MASK_1GB) | (~(MASK_RSVD | MASK_1GB) & virt);
	}

	if (pdpte->p) {
		pd = VIRTUAL(pdpte->v & MASK_ADDR);
	} else {
		return 0;
	}
	pde = pd->entry + PD_IDX(virt);

	if (pde->ps) {
		return (pde->v & MASK_2MB) | (~(MASK_RSVD | MASK_2MB) & virt);
	}

	if (pde->p) {
		pt = VIRTUAL(pde->v & MASK_ADDR);
	} else {
		return 0;
	}
	pte = pt->entry + PT_IDX(virt);

	return (pte->v & MASK_4KB) | (~(MASK_RSVD | MASK_4KB) & virt);
}

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

