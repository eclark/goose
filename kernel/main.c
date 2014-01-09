
#include <system.h>
#include <x86_64.h>
#include <multiboot2.h>
#include <acpi.h>
#include <mem.h>
#include <interrupt.h>
#include <ioport.h>

#include "device/vc.h"
#include "device/uart.h"

#define PTYP(x) kprintf("sizeof(" #x ") = %z\n", sizeof(x))

void
main(uint32_t magic, uint32_t addr)
{
	int i;
	/* Temporary place for the memory map of the initial process */
	static frame_t *mmap;

	vc_clear();
	kconsole = &uartdev;

	if (multiboot_init(magic, addr)) {
		kprintf("multiboot2_init failed.\n");
		return;
	}

	/* QEMU's hardware data structures are around 0x07f00000 */
	map_page(0x07e00000, LARGE_PAGE);
	/* APIC and other hardware are above 0xfe000000 */
	for (i = 0; i < 16; i++) {
		map_page(0xfe000000 + 0x200000 * i, LARGE_PAGE);
	}
	flush_tlb();

	/* Some parts of the initialization depend on dynamic memory allocation
	 * in the kernel. entry.S reserves the second 2MB page for this purpose
	 * Further allocation of space beyond the initial 2MB heap requires a
	 * the page frame allocator. */
	kbfree(VIRTUAL(0x200000), 0x200000);

	/* Set up the page frame allocator. TODO Fix this to read the multiboot
	 * tables. */
	{
		uintptr_t p = 0;
		frame_t *curr;

		curr = kmalloc(sizeof(frame_t));
		curr->phys = 0;
		curr->refcnt = 1;
		curr->ps = LARGE_PAGE;
		framelist_add(&mmap, curr);

		curr = kmalloc(sizeof(frame_t));
		curr->phys = 2 * 1024 * 1024;
		curr->refcnt = 1;
		curr->ps = LARGE_PAGE;
		framelist_add(&mmap, curr);

		curr = kmalloc(sizeof(frame_t));
		curr->phys = 126 * 1024 * 1024;
		curr->refcnt = 1;
		curr->ps = LARGE_PAGE;
		framelist_add(&mmap, curr);

		for (p = 4*1024*1024; p < 126 * 1024 * 1024; p += page_size(LARGE_PAGE)) {
			curr = kmalloc(sizeof(frame_t));
			curr->phys = p;
			curr->refcnt = 0;
			curr->ps = LARGE_PAGE;
			frame_free(curr);
		}
	}

	/* Parse the ACPI tables for information needed by the other drivers */
	if (acpi_init()) {
		kprintf("acpi_init failed.\n");
		return;
	}

	/* Initialize the interrupt controller */
	if (apic_init()) {
		kprintf("apic_init failed. No apic?\n");
		return;
	}

	/* print cpu identification */
	uint32_t regs[4];
	regs[3] = 0;
	cpuid(regs + 3, regs, regs + 2, regs + 1);
	regs[3] = 0;
	kprintf("%s\n", regs);

	/* print cpu brand */
	char buf[49];
	processor_brand(buf);
	kprintf("%s\n", buf);

	frame_t *p;

	p = frame_alloc(STANDARD_PAGE);
	framelist_add(&mmap, p);
	kprintf("A1: %#lx %#lx\n", p, p->phys);
	frame_free(framelist_remove(&mmap, p));

	p = frame_alloc(STANDARD_PAGE);
	framelist_add(&mmap, p);
	kprintf("A2: %#lx %#lx\n", p, p->phys);
	frame_free(framelist_remove(&mmap, p));

	p = frame_alloc(LARGE_PAGE);
	framelist_add(&mmap, p);
	kprintf("A3: %#lx %#lx\n", p, p->phys);
	frame_free(framelist_remove(&mmap, p));

	int *x;

	kprintf("x = %#lx\n", x);
	x = kmalloc(sizeof(int));

	kprintf("x = %#lx\t*x = %#x\n", x, *x);

	*x = 0xDEADBEEF;
	kprintf("x = %#lx\t*x = %#x\n", x, *x);

	kfree(x);
	kprintf("x = %#lx\n", x);


	sti();
	while (1) {
		asm volatile("hlt");
		kprintf("Interrupted\n");
	}
}

void
interrupt(regs_t* regs)
{
	uint64_t cr2;

	kprintf(
		"=== PANIC ==================================================\n"
		"Interrupt Vector: %d\n"
		"Error code: %#lx\n"
		"Flags: %#lx\n"
		"RIP: %#lx\n"
		"RAX: %#lx RBX: %#lx\n"
		"RCX: %#lx RDX: %#lx\n"
		"RDI: %#lx RSI: %#lx\n"
		"RBP: %#lx RSP: %#lx\n"
		" R8: %#lx  R9: %#lx\n"
		"R10: %#lx R11: %#lx\n"
		"R12: %#lx R13: %#lx\n"
		"R14: %#lx R15: %#lx\n",

		regs->vector,
		regs->error_code,
		regs->rflags,
		regs->rip,
		regs->rax,	regs->rbx,	regs->rcx,	regs->rdx,
		regs->rdi,	regs->rsi,	regs->rbp,	regs->rsp,
		regs->r8,	regs->r9,	regs->r10,	regs->r11,
		regs->r12,	regs->r13,	regs->r14,	regs->r15
	);

	if (regs->vector == 14) {
		asm volatile("movq %%cr2, %q0" : "=a"(cr2) : :);
		kprintf("CR2: %#lx\n", cr2);
	}

	kprintf("\nHalted\n");

	asm volatile("1: hlt; jmp 1b;");
}
