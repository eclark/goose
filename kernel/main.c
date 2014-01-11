
#include <system.h>
#include <x86_64.h>
#include <multiboot2.h>
#include <acpi.h>
#include <mem.h>
#include <interrupt.h>
#include <ioport.h>

#include "device/vc.h"
#include "device/uart.h"

#include "fs/tar.h"

#define PTYP(x) kprintf("sizeof(" #x ") = %z\n", sizeof(x))

static int kbd(regs_t *regs);

static handler_t intvec[256];

extern uint8_t *_end;

void
main(uint32_t magic, uint32_t addr)
{
	int i;
	/* Temporary place for the memory map of the initial process */
	static frame_t *mmap;

	vc_clear();
	kconsole = &uartdev;
	/* kconsole = &vcdev; */

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
	 * in the kernel. Give it the rest of the second 2MB page, but leave
	 * space for the ramdisk. */
	/* TODO make this less fragile */
	kbfree(VIRTUAL(initrd_phys + initrd_len), initrd_phys + initrd_len - 0x400000);

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

		/* Add the rest of the 0-128MB as free pages */
		for (p = 4*1024*1024; p < 126 * 1024 * 1024; p += page_size(LARGE_PAGE)) {
			curr = kmalloc(sizeof(frame_t));
			curr->phys = p;
			curr->refcnt = 0;
			curr->ps = LARGE_PAGE;
			frame_free(curr);
		}

		for (i = 0; i < 16; i++) {
			curr = kmalloc(sizeof(frame_t));
			curr->phys = 0xfe000000 + 0x200000 * i;
			curr->refcnt = 1;
			curr->ps = LARGE_PAGE;
			framelist_add(&mmap, curr);
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

	uart_init();
	kconsole = &uartdev;

	kprintf("initrd: %#lx %d\n", initrd_phys, initrd_len);

	tar_demo(VIRTUAL(initrd_phys));

	kprintf("Kernel End: %#lx\n", &_end);

	/* Enable an interrpt for testing */
	bind_vector(IRQBASEVEC + 1, kbd);
	enable_isa_irq(1);

	sti();
	while (1) {
		char ch;
		kconsole->read(kconsole, &ch, 1);
		kprintf("%d ", ch);
	}
}

static int
kbd(regs_t *regs)
{
	kprintf("Kbd\n");

	send_eoi(1);

	return 1;
}

void
interrupt(regs_t* regs)
{
	uint64_t cr2;

	assert(regs->vector < 256);

	if (intvec[regs->vector] != NULL && intvec[regs->vector](regs))
		return;

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

void
bind_vector(uint8_t irq, handler_t f)
{
	assert(intvec[irq] == NULL);

	intvec[irq] = f;
}

void
clear_vector(uint8_t irq)
{
	intvec[irq] = NULL;
}
