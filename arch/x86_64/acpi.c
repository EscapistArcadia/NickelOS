#include <acpi.h>
#include <lib.h>


#define acpi_validate_signature(table, sig, len)  \
    (strncmp((table)->signature, sig, len) == 0)

static uint32_t processor_count = 0;

/**
 * @brief checks if the checksum of an ACPI table is correct.
 * 
 * @param table the table data
 * @param length the length of the table
 * @return 1 if the checksum is correct, 0 otherwise
 */
static inline int acpi_validate_checksum(uint8_t *table, size_t length) {
    // if (table == NULL) {                                         /* valid has been checked */
    //     return 0;
    // }

    uint8_t *pos = table, sum = 0;
    for (; length; ++pos, --length) {                               /* sum == 0 => table is valid */
        sum += *pos;
    }
    return (sum == 0);
}

/**
 * @brief parses the multiple APIC description table (MADT) from the ACPI tables.
 * From this table, we could know processor count and their APIC's memory-mapped
 * IO address, for further interprocessor communication.
 * 
 * @param table the table data
 * @return 1 if the table is parsed successfully, 0 otherwise
 */
int acpi_table_parse_madt(struct acpi_table_header *table) {
    struct acpi_table_madt {
        struct acpi_table_header header;
        uint32_t local_apic_address;
        uint32_t flags;
    } __attribute__((packed)) *madt = (struct acpi_table_madt *)table;

    // if (madt == NULL
    //     || acpi_validate_checksum((uint8_t *)madt, madt->header.length)) {
    //     return 0;
    // }

    processor_count = 0;
    uint8_t *begin = (uint8_t *)(madt + 1), *end = (uint8_t *)madt + madt->header.length;
    while (begin < end) {
        uint8_t type = *begin;
        uint8_t length = *(begin + 1);

        switch (type) {
            case APIC_TYPE_LOCAL: {
                volatile uint8_t local_id = *(begin + 2),              /* volatile for debug */
                        local_apic_id = *(begin + 3),
                        flags = *(begin + 4);
                if (flags & 0x1) {
                    ++processor_count;
                }
                break;
            }
            case APIC_TYPE_IO: {
                volatile uint8_t io_apic_id = *(begin + 2),
                        reserved = *(begin + 3),
                        io_apic_address = *(uint32_t *)(begin + 4),
                        global_system_interrupt_base = *(uint32_t *)(begin + 8);
                break;
            }
            default: {
                break;
            }
        }

        begin += length;
    }
    
    return 1;
}

int acpi_table_parse(struct acpi_table_xsdp *xsdp, struct acpi_table_rsdp *rsdp) {
    if (xsdp != NULL
        && acpi_validate_signature(xsdp, ACPI_XSDP_SIG, ACPI_XSDP_SIG_LEN)
        // && acpi_validate_checksum((uint8_t *)xsdp, sizeof(struct acpi_table_rsdp))
        && acpi_validate_checksum((uint8_t *)xsdp, sizeof(struct acpi_table_xsdp))) {
        struct acpi_table_header *xsdt = (struct acpi_table_header *)xsdp->xsdt_address,
                                 **entry_list = (struct acpi_table_header **)(xsdt + 1);
        uint64_t entry_count = (xsdt->length - sizeof(struct acpi_table_header)) / sizeof(uint64_t);
        for (uint64_t i = 0; i < entry_count; ++i, ++entry_list) {
            if (acpi_validate_signature(*entry_list, ACPI_SIG_MADT, ACPI_SIG_LEN)) {
                acpi_table_parse_madt(*entry_list);
            } else if (acpi_validate_signature(*entry_list, ACPI_SIG_MADT, ACPI_SIG_LEN)) {
                /* TODO: other table to be considered */
            }
        }
        return 1;
    } else if (rsdp != NULL
        && acpi_validate_signature(rsdp, ACPI_RSDP_SIG, ACPI_RSDP_SIG_LEN)
        && acpi_validate_checksum((uint8_t *)rsdp, sizeof(struct acpi_table_rsdp))) {
        return 1;
    } else {
        return 0;
    }
}
