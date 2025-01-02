#include <types.h>
#include <acpi.h>

typedef struct boot_info {
    struct acpi_table_rsdp *rsdp;
    struct acpi_table_xsdp *xsdp;
} boot_info;

void NickelMain(struct boot_info *boot_info) {
    acpi_table_parse(boot_info->xsdp, boot_info->rsdp);
}
