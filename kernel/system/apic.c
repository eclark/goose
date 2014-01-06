
#include <system.h>
#include <interrupt.h>
#include <x86_64.h>
#include <acpi.h>
#include <ioport.h>
#include <mem.h>

#define I8259_MASTER_DATA 0x21
#define I8259_SLAVE_DATA 0xa1
#define I8259_DISABLE 0xff

int
apic_init(void)
{
	acpi_table_t *madt;

	madt = acpi_get(ACPI_MADT);
	if (madt == NULL)
		return -1;

	if (madt->madt.flags.pcat_compat) {
		/* Disable I8259 PICs */
		outb(I8259_SLAVE_DATA, I8259_DISABLE);
		outb(I8259_MASTER_DATA, I8259_DISABLE);
		kprintf("Disabled 8259 PICs\n");
	}

	kprintf("Local APIC Address: %#x %#lx\n",
		madt->madt.local_interrupt_controller_address,
		VIRTUAL(madt->madt.local_interrupt_controller_address));


	return 0;
}
