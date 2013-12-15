
#include <system.h>
#include <x86_64.h>
#include <multiboot2.h>
#include <vc.h>
#include <mem.h>
#include <interrupt.h>
#include <ioport.h>

void
main(uint32_t magic, uint32_t addr)
{
	unsigned long long i;
	clear();

	/*
	while (1) {
		puts(" => ");
		puthex(i, 2);
		puts(" <=\n");

		i++;
	}*/

	puthex(magic, 4);
	puts("\n");
	puthex(addr, 4);
	puts("\n");


	struct multiboot_tag* tag = (void*)(HIGH_HALF + addr + 8);

	puts("\nType: ");
	puthex(tag->type, 4);
	puts("\nSize: ");
	puthex(tag->size, 4);

	uint64_t rax, rbx, rcx, rdx;
	char buf[49];

	rax = 0;
	cpuid(&rax, &rbx, &rcx, &rdx);

	*(uint32_t*)buf = (uint32_t)rbx;
	*(uint32_t*)&buf[4] = (uint32_t)rdx;
	*(uint32_t*)&buf[8] = (uint32_t)rcx;
	buf[12] = 0;

	puts("\n");
	puts(buf);
	puts("\n");

	processor_brand(buf);
	puts(buf);

}

void
interrupt(regs_t* regs)
{
	puts("Interrupt Vector: ");
	puthex(regs->vector, 1);
	puts("\nException: ");
	puthex(regs->error_code, 8);
	puts("\nAt: ");
	puthex(regs->rip, 8);
	puts("\n\nHalted\n");

	asm volatile("1: hlt; jmp 1b;");
}
