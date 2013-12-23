
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

	apic_init();

	uint64_t rax, rbx, rcx, rdx;
	char buf[49];

	rax = 0;
	cpuid(&rax, &rbx, &rcx, &rdx);

	*(uint32_t*)buf = (uint32_t)rbx;
	*(uint32_t*)&buf[4] = (uint32_t)rdx;
	*(uint32_t*)&buf[8] = (uint32_t)rcx;
	buf[12] = 0;

	kprintf("%s\n", buf);

	processor_brand(buf);
	kprintf("%s\n", buf);

	PTYP(int);
	PTYP(long);
	PTYP(long long);
}

void
interrupt(regs_t* regs)
{
	kprintf(
		"=== PANIC ==================================================\n"
		"Interrupt Vector: %d\n"
		"Error code: %#lx\n"
		"Flags: %#lx\n"
		"RAX: %#lx RBX: %#lx\n"
		"RCX: %#lx RDX: %#lx\n"
		"RDI: %#lx RSI: %#lx\n"
		"RBP: %#lx RSP: %#lx\n"
		" R8: %#lx  R9: %#lx\n"
		"R10: %#lx R11: %#lx\n"
		"R12: %#lx R13: %#lx\n"
		"R14: %#lx R15: %#lx\n"
		"\nHalted\n",
		
		regs->vector,
		regs->error_code,
		regs->rflags,
		regs->rax,	regs->rbx,	regs->rcx,	regs->rdx,
		regs->rdi,	regs->rsi,	regs->rbp,	regs->rsp,
		regs->r8,	regs->r9,	regs->r10,	regs->r11,
		regs->r12,	regs->r13,	regs->r14,	regs->r15
	);

	asm volatile("1: hlt; jmp 1b;");
}
