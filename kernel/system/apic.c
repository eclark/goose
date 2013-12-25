
#include <system.h>
#include <interrupt.h>
#include <ioport.h>
#include <mem.h>

#define I8259_MASTER_DATA 0x21
#define I8259_SLAVE_DATA 0xa1
#define I8259_DISABLE 0xff

void
apic_init(void)
{
	/* Disable I8259 PICs */
	outb(I8259_SLAVE_DATA, I8259_DISABLE);
	outb(I8259_MASTER_DATA, I8259_DISABLE);
}
