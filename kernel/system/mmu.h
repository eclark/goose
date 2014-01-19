#ifndef _MMU_H
#define _MMU_H

#include <types.h>

/* Similar to
 * http://forum.osdev.org/viewtopic.php?f=15&t=25545 */

#define MMU_RNUM 0x1FFul

/* Convert virtual address to paging index */
#define MMU_PML4_IDX(virt)	(((uintptr_t)(virt) >> 39) & 0x1FF)
#define MMU_PDPT_IDX(virt)	(((uintptr_t)(virt) >> 30) & 0x1FF)
#define MMU_PD_IDX(virt)	(((uintptr_t)(virt) >> 21) & 0x1FF)
#define MMU_PT_IDX(virt)	(((uintptr_t)(virt) >> 12) & 0x1FF)

/* Base addresses */
#define MMU_ADDR_PT		(0xffff000000000000 + (MMU_RNUM << 39))
#define MMU_ADDR_PD		(MMU_ADDR_PT		+ (MMU_RNUM << 30))
#define MMU_ADDR_PDPT	(MMU_ADDR_PD		+ (MMU_RNUM << 21))
#define MMU_ADDR_PML4	(MMU_ADDR_PDPT		+ (MMU_RNUM << 12))

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
			uint64_t pat : 1; 	/* PAT for 4kb pages */
			uint64_t g : 1;		/* Global */
			uint64_t ign0 : 3;
			uint64_t rsvd : 51;
			uint64_t xd : 1;	/* Execute Disable */
		} __attribute__((packed));
	};
} mmu_t;

typedef mmu_t page_entry_t;

#define MMU_PML4(virt)	((mmu_t*)	MMU_ADDR_PML4)
#define MMU_PDPT(virt)	((mmu_t*)(	MMU_ADDR_PDPT	+ (((uintptr_t)(virt) >> 27)	& 0x00000000001ff000) ))
#define MMU_PD(virt)	((mmu_t*)(	MMU_ADDR_PD		+ (((uintptr_t)(virt) >> 18)	& 0x000000003ffff000) ))
#define MMU_PT(virt)	((mmu_t*)(	MMU_ADDR_PT		+ (((uintptr_t)(virt) >> 9)	& 0x0000007ffffff000) ))

/* Matches the memory types encoded with PAT */
typedef enum {
	MT_WB = 0,
	MT_WT = 1,
	MT_UCW = 2,
	MT_UC = 3
	/* We could define WC and WP here, but IA32_PAT MSR is not yet
	 * modified to use them */
} mem_type_t;

#define VREGION_LINEAR	0xffff800000000000
#define VREGION_STACK	0xffff800040000000
#define VREGION_HEAP	0xffff800080000000
#define VREGION_DEVICE	0xffff8000c0000000

typedef enum {
	LINEAR,
	STACK,
	HEAP,
	DEVICE,
	USER
} vregion_t;

/* mmu_clone */

void mmu_entries(uintptr_t virt, mmu_t **pml4, mmu_t **pdpt, mmu_t **pd, mmu_t **pt);

void *mmu_map(uintptr_t phys, uintptr_t virt);
void *mmu_maptype(uintptr_t phys, uintptr_t virt, mem_type_t typ);
void *mmu_mapregion(uintptr_t phys, vregion_t reg);

uintptr_t mmu_unmap(void *p);

static inline void
mmu_flush(void)
{
	asm volatile (
		"movq %%cr3, %%rax; movq %%rax, %%cr3"
		::: "rax", "memory", "cc"
	);
}

#endif
