#ifndef _ACPI_H
#define _ACPI_H

#include <types.h>

typedef struct {
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
} rsdp_t;

typedef uint32_t acpi_signature_t;

extern const acpi_signature_t ACPI_FADT; 
extern const acpi_signature_t ACPI_SSDT;
extern const acpi_signature_t ACPI_MADT;
extern const acpi_signature_t ACPI_HPET;
extern const acpi_signature_t ACPI_DSDT;

typedef struct {
	union {
		char signature[4];
		acpi_signature_t acpi_signature;
	};
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
	uint8_t address_space_id;
	uint8_t register_bit_width;
	uint8_t register_bit_offset;
	uint8_t access_size;
	uint64_t address;
} gas_t;

/* Tables used by QEMU */
typedef struct {
	sdt_header_t h;
	uint32_t entry[0];
} rsdt_t;

typedef struct {
	uint32_t wbinvd : 1;
	uint32_t wbinvd_flush : 1;
	uint32_t proc_c1 : 1;
	uint32_t p_lvl2_up : 1;
	uint32_t pwr_button : 1;
	uint32_t slp_button : 1;
	uint32_t fix_rtc : 1;
	uint32_t rtc_s4 : 1;
	uint32_t tmr_val_ext : 1;
	uint32_t dck_cap : 1;
	uint32_t reset_reg_sup : 1;
	uint32_t sealed_case : 1;
	uint32_t headless : 1;
	uint32_t cpu_sw_slp : 1;
	uint32_t pci_exp_wak : 1;
	uint32_t use_platform_clock : 1;
	uint32_t s4_rtc_sts_valid : 1;
	uint32_t remote_power_on_capable : 1;
	uint32_t force_apic_cluster_model : 1;
	uint32_t force_apic_physical_destination_mode : 1;
	uint32_t hw_reduced_acpi : 1;
	uint32_t low_power_s0_idle_capable : 1;
	uint32_t reserved : 10;
} fadt_flags_t;

typedef struct {
	uint16_t legacy_devices : 1;
	uint16_t _8042 : 1;
	uint16_t vga_not_present : 1;
	uint16_t msi_not_supported : 1;
	uint16_t pcie_aspm_controls : 1;
	uint16_t cmos_rtc_not_present : 1;
	uint16_t reserved : 10;
} fadt_iapc_boot_arch_t;

typedef struct {
	sdt_header_t h;
	uint32_t firmware_ctrl;
	uint32_t dsdt;
	uint8_t int_model; /* Must be zero for ACPI > 2.0 */
	uint8_t preferred_pm_profile;
	uint16_t sci_int;
	uint32_t smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4bios_req;
	uint8_t pstate_cnt;
	uint32_t pm1a_evt_blk;
	uint32_t pm1b_evt_blk;
	uint32_t pm1a_cnt_blk;
	uint32_t pm1b_cnt_blk;
	uint32_t pm2_cnt_blk;
	uint32_t pm_tmr_blk;
	uint32_t gpe0_blk;
	uint32_t gpe1_blk;
	uint8_t pm1_evt_len;
	uint8_t pm1_cnt_len;
	uint8_t pm2_cnt_len;
	uint8_t pm_tmr_len;
	uint8_t gpe0_blk_len;
	uint8_t gpe1_blk_len;
	uint8_t gpe1_base;
	uint8_t cst_cnt;
	uint16_t p_lvl2_lat;
	uint16_t p_lvl3_lat;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alrm;
	uint8_t mon_alrm;
	uint8_t century;
	fadt_iapc_boot_arch_t iapc_boot_arch;
	uint8_t reserved0; /* Constant Zero */
	fadt_flags_t flags;

	/* Further fields are in ACPI 5.0 spec but not present in QEMU */
	uint8_t reset_value;
	uint8_t reserved1;
	uint64_t x_firmware_ctrl;
	uint64_t x_dsdt;
	gas_t x_pm1a_evt_blk;
	gas_t x_pm1b_evt_blk;
	gas_t x_pm1a_cnt_blk;
	gas_t x_pm1b_cnt_blk;
	gas_t x_pm2_cnt_blk;
	gas_t x_pm_tmr_blk;
	gas_t x_gpe0_blk;
	gas_t x_gpe1_blk;
	gas_t sleep_control_reg;
	gas_t sleep_status_reg;
} fadt_t;

typedef struct {
	uint32_t s4bios_f : 1;
	uint32_t _64bit_wake_supported_f : 1;
	uint32_t reserved : 30; /* Always zero */
} facs_flags_t;

typedef struct {
	uint32_t _64bit_wake_f : 1;
	uint32_t reserved : 31; /* Always zero */
} facs_ospm_flags_t;

typedef struct {
	char signature[4]; /* Must be "FACS" */
	uint32_t length;
	uint32_t hardware_signature;
	uint32_t firmware_waking_vector;
	uint32_t global_lock; /* See section 5.2.10.1 before using */
	facs_flags_t flags;
	uint64_t x_firmware_waking_vector;
	uint8_t version;
	uint8_t reserved0[3]; /* Always zero */
	facs_ospm_flags_t ospm_flags;
	uint8_t reserved1[24]; /* Always zero */
} facs_t;

typedef struct system_descriptor_table {
	sdt_header_t h;
	int8_t definition_block[0];
} dsdt_t;

typedef struct system_descriptor_table ssdt_t;
typedef struct system_descriptor_table psdt_t;

typedef struct {
	uint32_t pcat_compat : 1;
	uint32_t reserved : 31;
} madt_flags_t;

typedef struct {
	uint16_t polarity : 2;
	uint16_t trigger_mode : 2;
	uint16_t reserved : 12;
} mps_inti_flags_t;

typedef struct {
	uint8_t type;
	uint8_t length;
	union {
		struct { /* Type 0x00 Length 8 */
#define ACPI_ICS_PROCESSOR_LOCAL_APIC 0x00
			uint8_t acpi_processor_id;
			uint8_t apic_id;
			struct {
				uint32_t enabled : 1;
				uint32_t reserved : 31;
			} flags;
		} processor_local_apic;

		struct __attribute__((packed)) { /* Type 0x01 Length 12 */
#define ACPI_ICS_IO_APIC 0x01
			uint8_t io_apic_id;
			uint8_t reserved0;
			uint32_t io_apic_address;
			uint32_t global_system_interrupt_base;
		} io_apic;

		struct __attribute__((packed)) { /* Type 0x02 Length 10 */
#define ACPI_ICS_INTERRUPT_SOURCE_OVERRIDE 0x02
			uint8_t bus;
			uint8_t source;
			uint32_t global_system_interrupt;
			mps_inti_flags_t flags;
		} interrupt_source_override;

		struct { /* Type 0x03 Length 8 */
#define ACPI_ICS_NON_MASKABLE_SOURCE 0x03
			mps_inti_flags_t flags;
			uint32_t global_system_interrupt;
		} non_maskable_source;

		struct __attribute__((packed)) { /* Type 0x04 Length 6 */
#define ACPI_ICS_LOCAL_APIC_NMI 0x04
			uint8_t acpi_processor_id;
			mps_inti_flags_t flags;
			uint8_t local_apic_lintn;
		} local_apic_nmi;

		struct { /* Type 0x05 Length 12 */
#define ACPI_ICS_LOCAL_APIC_ADDRESS_OVERRIDE 0x05
			uint16_t reserved;
			uint64_t local_apic_address;
		} local_apic_address_override;

		struct { /* Type 0x06 Length 16 */
#define ACPI_ICS_IO_SAPIC 0x06
			uint8_t io_apic_id;
			uint8_t reserved;
			uint32_t global_system_interrupt_base;
			uint64_t io_sapic_address;
		} io_sapic;

		struct { /* Type 0x07 Length N */
#define ACPI_ICS_PROCESSOR_LOCAL_SAPIC 0x07
			uint8_t acpi_processor_id;
			uint8_t local_sapic_id;
			uint8_t local_sapic_eid;
			uint8_t reserved[3];
			struct {
				int32_t enabled : 1;
				int32_t reserved : 31;
			} flags;
			uint32_t acpi_processor_uid_value;
			uint8_t acpi_processor_uid_string[0]; /* NULL terminated */
		} processor_local_sapic;

		struct { /* Type 0x08 Length 16 */
#define ACPI_ICS_PLATFORM_INTERRUPT_SOURCES 0x08
			mps_inti_flags_t flags;
			uint8_t interrupt_type;
			uint8_t processor_id;
			uint8_t processor_eid;
			uint8_t io_sapic_vector;
			uint32_t global_system_interrupt;
			struct {
				uint32_t cpei_processor_override : 1;
				uint32_t reserved : 31;
			} platform_interrupt_source_flags;
		} platform_interrupt_sources;

		struct { /* Type 0x09 Length 12 */
#define ACPI_ICS_PROCESSOR_LOCAL_X2APIC 0x09
			uint16_t reserved;
			uint32_t x2apic_id;
			struct {
				int32_t enabled : 1;
				int32_t reserved : 31;
			} flags;
			uint32_t acpi_processor_uid;
		} processor_local_x2apic;

		struct { /* Type 0x0a Length 12 */
#define ACPI_ICS_LOCAL_X2APIC_NMI 0x0a
			mps_inti_flags_t flags;
			uint32_t acpi_processor_uid;
			uint8_t local_x2apic_lintn;
			uint8_t reserved[3];
		} local_x2apic_nmi;

		struct { /* Type 0x0b Length 40 */
#define ACPI_ICS_GIC 0x0b
			uint16_t reserved;
			uint32_t gic_id;
			uint32_t acpi_processor_uid;
			struct { 
				uint32_t enabled : 1;
				uint32_t performance_interrupt_mode : 1;
				uint32_t reserved : 30;
			} flags;
			uint32_t parking_protocol_version;
			uint32_t performance_interrupt_gsiv;
			uint64_t parked_address;
			uint64_t physical_base_address;
		} gic;

		struct { /* Type 0x0c Length 24 */
#define ACPI_ICS_GIC_DISTRIBUTOR 0x0c
			uint16_t reserved0;
			uint32_t gic_id;
			uint64_t physical_base_address;
			uint32_t system_vector_base;
			uint32_t reserved1;
		} gic_distributor;
	} __attribute__((packed));
} __attribute__((packed)) acpi_ics_t;

typedef struct {
	sdt_header_t h;
	uint32_t local_interrupt_controller_address;
	madt_flags_t flags;
	acpi_ics_t interrupt_controller_structure[0];
} __attribute__((packed)) madt_t;

typedef struct {
	uint32_t event_timer_block_id;
	gas_t base_address;
	uint8_t hpet_number;
	uint16_t min_tick;
	uint8_t page_protection : 4;
	uint8_t oem_attribute : 4;
} hpet_t;

typedef union {
	sdt_header_t h;
	rsdt_t rsdt;
	fadt_t fadt;
	dsdt_t dsdt;
	ssdt_t ssdt;
	psdt_t psdt;
	madt_t madt;
	hpet_t hpet;
} acpi_table_t;

typedef struct {
	void *curr;
	void *end;
} acpi_iter_t;

int acpi_rsdt_iter_new(acpi_iter_t *i, rsdt_t *rsdt);
int acpi_rsdt_iter_next(acpi_iter_t *i);
acpi_table_t *acpi_rsdt_iter_val(acpi_iter_t *i);

int acpi_madt_iter_new(acpi_iter_t *i, madt_t *madt);
int acpi_madt_iter_next(acpi_iter_t *i);
acpi_ics_t *acpi_madt_iter_val(acpi_iter_t *i);

int acpi_init(void);
acpi_table_t *acpi_get(acpi_signature_t sig);

extern rsdp_t *rsdp;
extern rsdt_t *rsdt;

#endif
