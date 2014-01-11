
#include <x86_64.h>
#include <acpi.h>
#include <system.h>
#include <mem.h>
#include <multiboot2.h>

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

static char * tag_names[16] = {
	"cmdline",
	"boot_loader_name",
	"module",
	"basic_meminfo",
	"bootdev",
	"mmap",
	"vbe",
	"framebuffer",
	"elf_sections",
	"apm",
	"efi32",
	"efi64",
	"smbios",
	"acpi_old",
	"acpi_new",
	"network"
};

uintptr_t initrd_phys;
size_t initrd_len;

int
multiboot_init(uint32_t magic, uint32_t addr)
{
	int i;
	struct multiboot_tag_all *tag = VIRTUAL(addr + 8);

	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		return -1;

	while (tag->tag.type != MULTIBOOT_TAG_TYPE_END) {
		kprintf("%#x %s ", tag->tag.type, tag_names[tag->tag.type - 1]);
		
		switch (tag->tag.type) {
			case MULTIBOOT_TAG_TYPE_CMDLINE:
				kprintf("%s\n", tag->string.string);
				break;
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				kprintf("%s\n", tag->string.string);
				break;
			case MULTIBOOT_TAG_TYPE_MODULE:
				kprintf("%#x %#x\n", tag->module.mod_start, tag->module.mod_end);
				initrd_phys = tag->module.mod_start;
				initrd_len = tag->module.mod_end - tag->module.mod_start;
				break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
				kprintf("%#x %#x\n", tag->basic_meminfo.mem_lower, tag->basic_meminfo.mem_upper);
				break;
			case MULTIBOOT_TAG_TYPE_BOOTDEV:
				kprintf("%x %x %x\n", tag->bootdev.biosdev, tag->bootdev.slice, tag->bootdev.part);
				break;
			case MULTIBOOT_TAG_TYPE_MMAP:
				kprintf( "%x %x\n", tag->mmap.entry_size, tag->mmap.entry_version);

				for (i = 0; i < (tag->mmap.size - 8) / tag->mmap.entry_size; i++) {
					kprintf("         %#lx %#lx %x\n", tag->mmap.entries[i].addr, tag->mmap.entries[i].len, tag->mmap.entries[i].type);
				}

				break;
			case MULTIBOOT_TAG_TYPE_VBE:
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
			case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
			case MULTIBOOT_TAG_TYPE_APM:
			case MULTIBOOT_TAG_TYPE_EFI32:
			case MULTIBOOT_TAG_TYPE_EFI64:
			case MULTIBOOT_TAG_TYPE_SMBIOS:
				kprintf("\n");
				break;
			case MULTIBOOT_TAG_TYPE_ACPI_OLD:
				for (i = 0; i < tag->old_acpi.size - 8; i++) {
					kprintf("%#hhx ", tag->old_acpi.rsdp[i]);
				}
				rsdp = (rsdp_t*)tag->old_acpi.rsdp;
			case MULTIBOOT_TAG_TYPE_ACPI_NEW:
			case MULTIBOOT_TAG_TYPE_NETWORK:
				kprintf("\n");
				break;
		}

		tag = ALIGN((uint64_t)tag + tag->tag.size, MULTIBOOT_TAG_ALIGN);
	}

	return 0;
}
