
#include <stddef.h>
#include <stdint.h>

#include "x86_64.h"
#include "vc.h"
#include "mem.h"
#include "interrupt.h"

void
main(void)
{
	unsigned long long i;
	clear();

	while (1) {
		puts(" => ");
		puthex(i, 2);
		puts(" <=\n");

		i++;
	}
}

void
interrupt(regs_t* regs)
{

	puts("Interrupt Vector: ");
	puthex(regs->vector, 1);
	puts("\nException: ");
	puthex(regs->error_code, 8);
	puts("\n\nHalted\n");

	asm volatile("1: hlt; jmp 1b;");
}
