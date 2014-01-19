
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

#include "system/mmu.h"

#define PTYP(x) kprintf("sizeof(" #x ") = %z\n", sizeof(x))

static int kbd(regs_t *regs);

static handler_t intvec[256];

extern uint8_t *_end;

void
main(uint32_t magic, uint32_t addr)
{
	int i;
	/* Temporary place for the memory map of the initial process */

	vc_clear();
	kconsole = &uartdev;
	/* kconsole = &vcdev; */

	if (multiboot_init(magic, addr)) {
		kprintf("multiboot2_init failed.\n");
		return;
	}

	/* Allocator setup */
	{
		/* Give a page to kmalloc, so the frame allocator can use it */
		uintptr_t heapaddr = (uintptr_t)ALIGN(initrd_phys + initrd_len, FRAME_LEN);
		kbfree(mmu_mapregion(heapaddr, HEAP), FRAME_LEN);

		/* Free a small amount of space for the frame allocator */
		heapaddr += FRAME_LEN;
		region_free(heapaddr, 0x400000 - initrd_phys - initrd_len - FRAME_LEN);

		/* Make a small heap */
		frame_t *fr;
		for (i = 0; i < 512; i++) {
			fr = frame_alloc();
			kbfree(mmu_mapregion(fr->phys, HEAP), FRAME_LEN);
			kfree(fr);
		}

		/* Indicate to the frame allocator that 16MB to 64MB is usable */
		region_free(0x01000000, 48*1024*1024);
	}

	/* map QEMU's acpi tables */
	mmu_map(0x07ffe000, (uintptr_t)VIRTUAL(0x07ffe000));
	mmu_map(0x07fff000, (uintptr_t)VIRTUAL(0x07fff000));

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

	tar_demo((uintptr_t)VIRTUAL(initrd_phys));

	kprintf("Kernel End: %#lx\n", &_end);

	klogf(LOG_DEBUG, "IA32_PAT: %#lx\n", rdmsr(IA32_PAT));

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
