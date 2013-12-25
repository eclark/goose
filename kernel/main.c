
#include <system.h>
#include <x86_64.h>
#include <multiboot2.h>
#include <mem.h>
#include <interrupt.h>
#include <ioport.h>

#include "device/vc.h"
#include "device/uart.h"

#define PTYP(x) kprintf("sizeof(" #x ") = %z\n", sizeof(x))

void
main(uint32_t magic, uint32_t addr)
{
	vc_clear();

	kconsole = &uartdev;

	if (multiboot_init(magic, addr)) {
		kprintf("multiboot2_init failed.\n");
		return;
	}

	if (acpi_init()) {
		kprintf("acpi_init failed.\n");
		return;
	}

	apic_init();

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
