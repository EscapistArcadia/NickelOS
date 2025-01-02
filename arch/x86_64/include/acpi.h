#ifndef __NICKEL_x86_64_ACPI_H__
#define __NICKEL_x86_64_ACPI_H__

#include <types.h>

#define ACPI_RSDP_SIG                   "RSD PTR "                  /* root system description pointer */
#define ACPI_XSDP_SIG                   ACPI_RSDP_SIG
#define ACPI_RSDP_SIG_LEN               8
#define ACPI_XSDP_SIG_LEN               ACPI_RSDP_SIG_LEN

#define ACPI_SIG_LEN                    4
#define ACPI_OEM_ID_LEN                 6
#define ACPI_OEM_TABLE_ID_LEN           8

#define ACPI_SIG_MADT                   "APIC"                      /* multiple APIC description table */
/* TODO: other table */

#define APIC_TYPE_LOCAL                 0
#define APIC_TYPE_IO                    1
#define APIC_TYPE_INTERRUPT_SOURCE_OVERRIDE 2
#define APIC_TYPE_NON_MASKABLE_INTERRUPT_SOURCE 3
#define APIC_TYPE_LOCAL_NMI             4
#define APIC_TYPE_LOCAL_ADDRESS_OVERRIDE 5
#define APIC_TYPE_IO_SAPIC              6
#define APIC_TYPE_LOCAL_SAPIC           7
#define APIC_TYPE_PLATFORM_INTERRUPT_SOURCES 8

struct acpi_table_rsdp {
    char signature[ACPI_RSDP_SIG_LEN];                              /* to find this table */
    uint8_t checksum;                                               /* checksum to check corruption */
    char oem_id[ACPI_OEM_TABLE_ID_LEN];                             /* OEM identification */
    uint8_t revision;                                               /* ACPI version, compatibility */
    uint32_t rsdt_address;                                          /* address of the RSDT */
} __attribute__((packed));

struct acpi_table_xsdp {
    char signature[ACPI_XSDP_SIG_LEN];                              /* to find this table */
    uint8_t checksum;                                               /* checksum to check corruption */
    char oem_id[ACPI_OEM_ID_LEN];                                   /* OEM identification */
    uint8_t revision;                                               /* ACPI version, compatibility */
    uint32_t rsdt_address;                                          /* address of the RSDT */
    uint32_t length;                                                /* length of the XSDT */
    uint64_t xsdt_address;                                          /* address of the XSDT */
    uint8_t extended_checksum;                                      /* checksum to check corruption */
    uint8_t reserved[3];                                            /* reserved */
} __attribute__((packed));

struct acpi_table_header {
    char signature[ACPI_SIG_LEN];                                   /* shows table type */
    uint32_t length;                                                /* total size of the table */
    uint8_t revision;                                               /* ACPI version, compatibility */
    uint8_t checksum;                                               /* checksum to check corruption */
    char oem_id[ACPI_OEM_ID_LEN];                                   /* OEM identification */
    char oem_table_id[ACPI_OEM_TABLE_ID_LEN];                       /* OEM table identification */
    uint32_t oem_revision;                                          /* OEM revision */
    uint32_t creator_id;                                            /* creator identification */
    uint32_t creator_revision;                                      /* creator revision */
} __attribute__((packed));

// enum apic_type {
//     LOCAL_APIC,
//     IO_APIC,
//     INTERRUPT_SOURCE_OVERRIDE,
//     NON_MASKABLE_INTERRUPT_SOURCE,
//     LOCAL_APIC_NMI,
//     LOCAL_APIC_ADDRESS_OVERRIDE,
//     IO_SAPIC,
//     LOCAL_SAPIC,
//     PLATFORM_INTERRUPT_SOURCES,
//     PROCESSOR_LOCAL_X2APIC,
//     LOCAL_X2APIC_NMI,
//     GIC,
    
//     APIC_TYPE_MAX,
// };

int acpi_table_parse(struct acpi_table_xsdp *xsdp, struct acpi_table_rsdp *rsdp);

#endif
