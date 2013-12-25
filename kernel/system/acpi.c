
#include <system.h>
#include <x86_64.h>
#include <mem.h>

#define RSDP_SIGNATURE 0x2052545020445352
#define RSDP_SIZE(r) (r->revision == 1 ? sizeof(rsdp_t) : 20)
#define SDT_COUNT ((rsdt->header.length - sizeof(sdt_header_t))/4)

struct rsdp_struct {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_addr;

	/* Version 2 fields */
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t extended_checksum;
	uint8_t reserved[3];
};

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} sdt_header_t;

typedef struct {
	sdt_header_t header;
	uint32_t sdt[0];
} rsdt_t;

static bool checksum(sdt_header_t *sdt);
static sdt_header_t *find(void);

rsdp_t *rsdp;
rsdt_t *rsdt;

int
acpi_init(void)
{
	int i;
	uint8_t sum;

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
	rsdt = map_page(rsdp->rsdt_addr, LARGE_PAGE);
	flush_tlb();

	char buf[5];
	for (i = 0; i < SDT_COUNT; i++) {
		sdt_header_t *q = VIRTUAL(rsdt->sdt[i]);
		memcpy(buf, q->signature, 4);
		buf[4] = 0;

		kprintf("%#x %#lx %s\n", rsdt->sdt[i], q, buf);
	}

	return 0;
}

static bool
checksum(sdt_header_t *sdt)
{
	int i;
	uint8_t sum;

	for (i = 0; i < sdt->length; i++)
		sum += ((char*)sdt)[i];

	return sum == 0;
}
