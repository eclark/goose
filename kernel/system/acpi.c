
#include <acpi.h>
#include <system.h>
#include <x86_64.h>
#include <mem.h>

/*
 * Everything here expects the memory location of the ACPI tables to be
 * linearly mapped upward so the VIRTUAL macro can translate the physical
 * addresses in the tables.
 */

#define RSDP_SIGNATURE 0x2052545020445352
#define RSDP_SIZE(r) (r->revision == 1 ? sizeof(rsdp_t) : 20)
#define SDT_COUNT ((rsdt->h.length - sizeof(sdt_header_t))/4)

static bool checksum(sdt_header_t *sdt);

const acpi_signature_t ACPI_FADT = 0x50434146; /* FACP */ 
const acpi_signature_t ACPI_SSDT = 0x54445353; /* SSDT */
const acpi_signature_t ACPI_MADT = 0x43495041; /* APIC */
const acpi_signature_t ACPI_HPET = 0x54455048; /* HPET */
const acpi_signature_t ACPI_DSDT = 0x54445344; /* DSDT */

rsdp_t *rsdp;
rsdt_t *rsdt;

int
acpi_init(void)
{
	int i;
	uint8_t sum;
	acpi_iter_t it;
	char buf[5];

	/* Check that the rsdp is valid */
	if (*(uint64_t*)rsdp->signature != RSDP_SIGNATURE)
		return -1;

	/* Check revision */
	if (rsdp->revision > 1)
		return -1;

	/* Validate the checksum */
	for (sum = 0, i = 0; i < RSDP_SIZE(rsdp); sum += ((uint8_t*)rsdp)[i], i++);
	if (sum != 0)
		return -1;

	/* Set up the root system descriptor table */
	kprintf("RSDT_ADDR %#x\n", rsdp->rsdt_addr);
	rsdt = VIRTUAL(rsdp->rsdt_addr);

	if (!acpi_rsdt_iter_new(&it, rsdt))
		return -1;

	buf[4] = 0;
	do {
		acpi_table_t *t = acpi_rsdt_iter_val(&it);

		memcpy(buf, t->h.signature, 4);
		kprintf("%#lx %d %s\n", t, t->h.length, buf);
	} while (acpi_rsdt_iter_next(&it));

	return 0;
}

int
acpi_rsdt_iter_new(acpi_iter_t *i, rsdt_t *rsdt)
{
	if (!checksum(&rsdt->h))
		return 0;

	i->curr = rsdt->entry;
	i->end = (void*)rsdt + rsdt->h.length;

	return 1;
}

int
acpi_rsdt_iter_next(acpi_iter_t *i)
{
	acpi_table_t *t;

	do {
		i->curr += 4;

		if (i->curr >= i->end)
			return 0;

		t = acpi_rsdt_iter_val(i);
	} while (!checksum(&t->h));

	return 1;
}

acpi_table_t *
acpi_rsdt_iter_val(acpi_iter_t *i)
{
	return VIRTUAL(*(uint32_t*)(i->curr));
}

acpi_table_t *
acpi_get(acpi_signature_t sig)
{
	acpi_iter_t it;

	if (sig == ACPI_DSDT) {
		acpi_table_t *fadt = acpi_get(ACPI_FADT);
		return VIRTUAL(fadt->fadt.dsdt);
	}

	if (!acpi_rsdt_iter_new(&it, rsdt))
		return NULL;

	do {
		acpi_table_t *t = acpi_rsdt_iter_val(&it);
		if (t->h.acpi_signature == sig)
			return t;
	} while (acpi_rsdt_iter_next(&it));

	return NULL;
}

static bool
checksum(sdt_header_t *sdt)
{
	int i;
	uint8_t sum = 0;

	for (i = 0; i < sdt->length; i++)
		sum += ((int8_t*)sdt)[i];

	return sum == 0;
}
