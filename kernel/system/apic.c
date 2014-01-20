/* Copyright (c) 2014 Eric Clark
 * See the file LICENSE for copying permission.
 */

#include <system.h>
#include <interrupt.h>
#include <x86_64.h>
#include <acpi.h>
#include <ioport.h>
#include <mem.h>

#include "mmu.h"

#define I8259_MASTER_DATA 0x21
#define I8259_SLAVE_DATA 0xa1
#define I8259_DISABLE 0xff

#define IOREGSEL 0x00
#define IOWIN 0x10
#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOAPICARB 0x02
#define IOREDTBL 0x10

typedef struct {
	union {
		uint64_t v;
		struct {
			uint64_t intvec : 8;
			uint64_t delmod : 3;
			uint64_t destmod : 1;
			uint64_t delivs : 1;
			uint64_t intpol : 1;
			uint64_t remote_irr : 1;
			uint64_t trigger_mode : 1;
			uint64_t interrupt_mask : 1;
			uint64_t reserved : 39;
			uint64_t destination : 8;
		};
	};
} ioapic_red_t;

static uint32_t read_lapic(uint8_t offset);
static void write_lapic(uint8_t offset, uint32_t val);

static uint32_t read_ioapic(uint8_t offset);
static void write_ioapic(uint8_t offset, uint32_t val);

/* local apic */
static uint32_t *lapic_base;

/* IOAPIC must be accessed using 32 bit loads and stores */
static uint32_t *ioapic_base;

int
apic_init(void)
{
	int i;
	acpi_table_t *madt;
	acpi_iter_t it;
	acpi_ics_t *ics;

	madt = acpi_get(ACPI_MADT);
	if (madt == NULL)
		return -1;

	if (madt->madt.flags.pcat_compat) {
		/* Disable I8259 PICs */
		outb(I8259_SLAVE_DATA, I8259_DISABLE);
		outb(I8259_MASTER_DATA, I8259_DISABLE);
		kprintf("Disabled 8259 PICs\n");
	}

	lapic_base = mmu_mapregion(madt->madt.local_interrupt_controller_address, DEVICE);
	kprintf("Local APIC Address: %#x %#lx\n",
		madt->madt.local_interrupt_controller_address,
		lapic_base);

	/* Enumerate the interrupt controller structures */
	acpi_madt_iter_new(&it, &madt->madt);
	do {
		ics = acpi_madt_iter_val(&it);

		switch (ics->type) {
			case ACPI_ICS_PROCESSOR_LOCAL_APIC:
				kprintf("PROCESSOR_LOCAL_APIC(acpi_processor_id: %d, "
					"apic_id: %d)\n", ics->processor_local_apic.acpi_processor_id,
					ics->processor_local_apic.apic_id);
				break;
			case ACPI_ICS_IO_APIC:
				kprintf("IO_APIC(io_apic_id: %d, io_apic_address: %#x, "
					"global_system_interrupt_base: %#x)\n",
					ics->io_apic.io_apic_id, ics->io_apic.io_apic_address,
					ics->io_apic.global_system_interrupt_base);

				/* Assume only one */
				ioapic_base = mmu_mapregion(ics->io_apic.io_apic_address, DEVICE);
				kprintf("IO APIC Address: %#x %#lx\n",
					ics->io_apic.io_apic_address,
					ioapic_base);
				break;
			case ACPI_ICS_INTERRUPT_SOURCE_OVERRIDE:
				kprintf("INTERRUPT_SOURCE_OVERRIDE(bus: %d, source: %d, "
					"global_system_interrupt: %#x)\n",
					ics->interrupt_source_override.bus,
					ics->interrupt_source_override.source,
					ics->interrupt_source_override.global_system_interrupt);
				break;
			case ACPI_ICS_LOCAL_APIC_NMI:
				kprintf("LOCAL_APIC_NMI(acpi_processor_id: %d, local_apic_lintn: %d)\n",
					ics->local_apic_nmi.acpi_processor_id, ics->local_apic_nmi.local_apic_lintn);
				break;
			default:
				kprintf("Unknown ICS\n");
				break;
		}
	} while (acpi_madt_iter_next(&it));

	/* This should match the value found in the ACPI table.
	 * What if it doesn't? I dont know... */
	uint64_t ia32_apic_base = rdmsr(IA32_APIC_BASE);
	kprintf("IA32_APIC_BASE %#lx\n", ia32_apic_base);

	kprintf("LAPICID %#x LAPICVER %#x\n", read_lapic(0x20), read_lapic(0x30));

	kprintf("IOAPICID %#x IOAPICVER %#x IOAPICARB %#x\n",
		read_ioapic(IOAPICID), read_ioapic(IOAPICVER), read_ioapic(IOAPICARB));

	for (i = 0; i < 24; i++) {
		uint64_t redtbl = 0;

		redtbl |= read_ioapic(IOREDTBL + 2 * i);
		redtbl |= (uint64_t)read_ioapic(IOREDTBL + 2 * i + 1) << 32;

		kprintf("IOREDTBL%d %#lx\n", i, redtbl);
	}

	/* Enable APIC */
	write_lapic(0xf0, read_lapic(0xf0) | 0x100);

	return 0;
}

void
enable_isa_irq(uint8_t irq)
{
	uint64_t flags;

	cli_ifsave(&flags);
	write_ioapic(IOREDTBL + 2 * irq, IRQBASEVEC + irq);

	ifrestore(flags);
}

void
disable_isa_irq(uint8_t irq)
{
	uint64_t flags;

	cli_ifsave(&flags);
	write_ioapic(IOREDTBL + 2 * irq, IRQBASEVEC + irq);

	ifrestore(flags);
}

void
send_eoi(uint8_t irq)
{
	uint64_t flags;
	cli_ifsave(&flags);

	write_lapic(0xb0, IRQBASEVEC + irq);

	ifrestore(flags);
}

static uint32_t
read_lapic(uint8_t offset)
{
	assert((offset & 0xf) == 0);
	return *(lapic_base + (offset >> 2));
}

static void
write_lapic(uint8_t offset, uint32_t val)
{
	assert((offset & 0xf) == 0);
	*(lapic_base + (offset >> 2)) = val;
}

/* Unsafe, caller must CLI/STI */
static uint32_t
read_ioapic(uint8_t offset)
{
	*ioapic_base = offset;
	return *(ioapic_base + 4);
}
static void
write_ioapic(uint8_t offset, uint32_t val)
{
	*ioapic_base = offset;
	*(ioapic_base + 4) = val;
}
