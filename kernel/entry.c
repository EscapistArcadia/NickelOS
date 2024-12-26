#include <types.h>

struct boot_info {
    struct rsdp *rsdp;
    struct rsdp *rsdp2;
};

struct rsdp {
    char signature[8];                                              /* shows block begins */
    uint8_t checksum;                                               /* if corrupted */
    char oem_id[6];
    uint8_t revision;                                               /* ACPI version */
    uint32_t rsdt_address;                                          /* 32-bit address */

    uint32_t length;
    uint64_t xsdt_address;                                          /* 64-bit address */
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct xsdt_header {
    char signature[4];                                              /* shows block type */
    uint32_t length;                                                /* count of data */
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct madt {
    struct xsdt_header header;
    uint32_t local_apic_address;
    uint32_t flags;
} __attribute__((packed));

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, uint32_t n) {
    while (n && *s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    return n ? *s1 - *s2 : 0;
}

void NickelMain(struct boot_info *boot_info) {
    int processor_count = 0;
    struct rsdp *rsdp = boot_info->rsdp, *rsdp2 = boot_info->rsdp2;

    if (rsdp2 && strncmp(rsdp2->signature, "RSD PTR ", 8) == 0) {
        struct xsdt_header *xsdt = (struct xsdt_header *)rsdp2->xsdt_address,
                           **xsdt_entries = (struct xsdt_header **)(xsdt + 1);
        int entry_count = (xsdt->length - sizeof(struct xsdt_header)) / sizeof(struct xsdt_header *);

        for (int i = 0; i < entry_count; i++) {
            struct xsdt_header *entry = xsdt_entries[i];
            if (strncmp(entry->signature, "APIC", 4) == 0) {
                struct madt *madt = (struct madt *)entry;
                uint8_t *madt_begin = (uint8_t *)(madt + 1);
                uint8_t *madt_end = (uint8_t *)madt + madt->header.length;

                while (madt_begin < madt_end) {
                    uint8_t type = *madt_begin;
                    uint8_t length = *(madt_begin + 1);

                    if (type == 0) {
                        uint32_t flags = *(madt_begin + 4);

                        if (flags & 1) {
                            processor_count++;
                        }
                    }

                    madt_begin += length;
                }
            }
        }
    }

    while (0xECEBCAFE);
}
