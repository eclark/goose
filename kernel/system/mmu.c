#include <log.h>
#include <mem.h>
#include "mmu.h"

void
mmu_entries(uintptr_t virt, mmu_t **pml4e, mmu_t **pdpte, mmu_t **pde, mmu_t **pte)
{
	*pml4e = MMU_PML4(virt) + MMU_PML4_IDX(virt);
	*pdpte = MMU_PDPT(virt) + MMU_PDPT_IDX(virt);
	*pde = MMU_PD(virt) + MMU_PD_IDX(virt);
	*pte = MMU_PT(virt) + MMU_PT_IDX(virt);
}

void *
mmu_map(uintptr_t phys, uintptr_t virt)
{
	return mmu_maptype(phys, virt, MT_WB);
}

void *
mmu_maptype(uintptr_t phys, uintptr_t virt, mem_type_t typ)
{
	mmu_t *pml4e, *pdpte, *pde, *pte;
	frame_t *f;

	assert((phys & 0xffff000000000fff) == 0);

	mmu_entries(virt, &pml4e, &pdpte, &pde, &pte);

	if (!pml4e->p) {
		f = frame_alloc();

		pml4e->v = f->phys;
		pml4e->rw = 1;
		pml4e->p = 1;

		kfree(f);
		mmu_flush();
	}

	if (!pdpte->p) {
		f = frame_alloc();

		pdpte->v = f->phys;
		pdpte->rw = 1;
		pdpte->p = 1;

		kfree(f);
		mmu_flush();
	}

	if (!pde->p) {
		f = frame_alloc();

		pde->v = f->phys;
		pde->rw = 1;
		pde->p = 1;

		kfree(f);
		mmu_flush();
	}

	assert(!pte->p);

	pte->v = phys;
	pte->rw = 1;
	pte->p = 1;

	switch (typ) {
		case MT_WB:
			break;
		case MT_WT:
			pte->pwt = 1;
			break;
		case MT_UCW:
			pte->pcd = 1;
			break;
		case MT_UC:
			pte->pcd = 1;
			pte->pwt = 1;
			break;
	}

	mmu_flush();

	return (void*)virt;
}

static uintptr_t heap_end = VREGION_HEAP;
static uintptr_t device_end = VREGION_DEVICE;

void *
mmu_mapregion(uintptr_t phys, vregion_t reg)
{
	void *p = NULL;

	if (reg == HEAP) {
		p = mmu_maptype(phys, heap_end, MT_WB);
		heap_end += FRAME_LEN;
	} else if (reg == DEVICE) {
		p = mmu_maptype(phys, device_end, MT_UC);
		device_end += FRAME_LEN;
	}

	return p;
}

uintptr_t
mmu_unmap(void *p)
{
	mmu_t *pte = MMU_PT(p) + MMU_PT_IDX(p);

	assert(pte->p);

	pte->p = 0;
	mmu_flush();

	return pte->v & 0x0000fffffffff000;
}
