
#include <system.h>
#include <x86_64.h>
#include <multiboot2.h>
#include <vc.h>
#include <mem.h>
#include <interrupt.h>
#include <ioport.h>

struct multiboot_tag_all {
	union {
		struct multiboot_tag tag;
		struct multiboot_tag_string string;
		struct multiboot_tag_module module;
		struct multiboot_tag_basic_meminfo basic_meminfo;
		struct multiboot_tag_bootdev bootdev;
		struct multiboot_tag_mmap mmap;
		struct multiboot_tag_vbe vbe;
		struct multiboot_tag_framebuffer_common framebuffer_common;
		struct multiboot_tag_framebuffer framebuffer;
		struct multiboot_tag_elf_sections elf_sections;
		struct multiboot_tag_apm apm;
		struct multiboot_tag_efi32 efi32;
		struct multiboot_tag_efi64 efi64;
		struct multiboot_tag_smbios smbios;
		struct multiboot_tag_old_acpi old_acpi;
		struct multiboot_tag_new_acpi new_acpi;
		struct multiboot_tag_network network;
	};
};

void multiboot_parse(uint32_t addr)
{
	struct multiboot_tag_all *tag = (void*)VIRTUAL(addr + 8);

	do {
		puthex(tag->tag.type, 4);
		puts("\n");

		tag = (void*)ALIGN((uint64_t)tag + tag->tag.size, MULTIBOOT_TAG_ALIGN);
	} while (tag->tag.type != 0);
}

void
main(uint32_t magic, uint32_t addr)
{
	unsigned long long i;
	clear();

	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		puts("Multiboot magic number error.");
		return;
	}

	multiboot_parse(addr);

	apic_init();

	uint64_t rax, rbx, rcx, rdx;
	char buf[49];

	rax = 0;
	cpuid(&rax, &rbx, &rcx, &rdx);

	*(uint32_t*)buf = (uint32_t)rbx;
	*(uint32_t*)&buf[4] = (uint32_t)rdx;
	*(uint32_t*)&buf[8] = (uint32_t)rcx;
	buf[12] = 0;

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
