#include <efi.h>
#include <efilib.h>
#include <elf.h>

struct boot_info {
    void *rsdp;                                                     /* root rsdp pointer */
    void *rsdp2;                                                    /* root rsdp pointer */
    struct {
        UINTN width;                                                /* screen width */
        UINTN height;                                               /* screen height */
    } screen;
};

/**
 * @brief The entry of the UEFI application and the first executed routines in the kernel.
 * We need not only to gather information and setup environment for the kernel, but also
 * load the kernel into the memory (correct address) to sstart the kernel.
 * 
 * @param ImageHandle The firmware allocated handle for the UEFI image.
 * @param SystemTable address of the UEFI system table, containing all services by UEFI.
 * @return never reaches
 */
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);                        /* must do this */

    struct boot_info boot_info = {NULL, NULL, {0, 0}};
    EFI_STATUS status = uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    if (EFI_ERROR(status)) {                                        /* clears the predefined logo */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }
    
    status = uefi_call_wrapper(SystemTable->BootServices->SetWatchdogTimer, 4, 0, 0, 0, NULL);
    if (EFI_ERROR(status)) {                                        /* prevents UEFI from being rebooted */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    /* **************************************************
     * *        Getting CPU-related Information         *
     * ************************************************** */
    EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;                  /* ACPI 2.0 table GUID, not founded */
    for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; ++i) { /* find the root rsdp 2.0 at first, then 1.0 */
        if (CompareGuid(&SystemTable->ConfigurationTable[i].VendorGuid, &Acpi20TableGuid) == 0) {
            boot_info.rsdp2 = (struct rsdp *)SystemTable->ConfigurationTable[i].VendorTable;
        } else if (CompareGuid(&SystemTable->ConfigurationTable[i].VendorGuid, &AcpiTableGuid) == 0) {
            boot_info.rsdp = (struct rsdp *)SystemTable->ConfigurationTable[i].VendorTable;
        }
    }

    /* **************************************************
     * *         Locating the Kernel Executable         *
     * ************************************************** */
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_proto = NULL;              /* to get access to the file system in a volume*/
    EFI_FILE_PROTOCOL *fs_proto = NULL,                             /* file system protocol for files */
                      *kernel_file = NULL;                          /* kernel file abstraction */
    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &gEfiSimpleFileSystemProtocolGuid, NULL, &sfs_proto);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(sfs_proto->OpenVolume, 2, sfs_proto, &fs_proto);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(fs_proto->Open, 5, fs_proto, &kernel_file, L"\\KERNEL\\NICKEL.EXE", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {                                        /* opens the kernel executable */
        Print(L"[%d] Status: %r; Failed to load the kernel.\r\n", __LINE__, status);
        return status;
    }

    /* **************************************************
     * *         Loading the kernel executable         *
     * ************************************************** */
    UINTN kernel_file_info_size = sizeof(EFI_FILE_INFO) + 128;
    EFI_FILE_INFO *kernel_file_info = NULL;                         /* to get the size of the kernel executable */
    status = uefi_call_wrapper(kernel_file->GetInfo, 4, kernel_file, &gEfiFileInfoGuid, &kernel_file_info_size, kernel_file_info);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    EFI_PHYSICAL_ADDRESS kernel_address = 0x100000;                 /* kernel's physical address */
    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, (kernel_file_info->FileSize + 0xFFF) >> 12, kernel_address);
    if (EFI_ERROR(status)) {                                        /* allocate enough memory for kernel */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &kernel_file_info->FileSize, kernel_address);
    if (EFI_ERROR(status)) {                                        /* load the kernel */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(kernel_file->Close, 1, kernel_file);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(fs_proto->Close, 1, fs_proto);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    /* **************************************************
     * *         Reading the kernel entry point         *
     * ************************************************** */
    Elf64_Ehdr *kernel_header = (Elf64_Ehdr *)kernel_address;       /* replace elf format with purely binary */
    void (*NickelMain)(struct boot_info *) = (void (*)(struct boot_info *))(kernel_header->e_entry);

    /* **************************************************
     * *           Setting the Screen Size              *
     * ************************************************** */
    EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics_protocol = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, &graphics_protocol);
    if (EFI_ERROR(status)) {                                        /* acquires frame buffer and modes */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    UINTN info_size;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;                     /* enumerate each mode to find predefined mode */
    status = uefi_call_wrapper(graphics_protocol->QueryMode, 4, graphics_protocol, graphics_protocol->Mode->Mode, &info_size, &info);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }
    boot_info.screen.width = info->HorizontalResolution;
    boot_info.screen.height = info->VerticalResolution;
    // for (UINT32 i = 0; i < graphics_protocol->Mode->MaxMode; ++i) {
    //     status = uefi_call_wrapper(graphics_protocol->QueryMode, 4, graphics_protocol, i, &info_size, &info);
    //     if (EFI_ERROR(status)) {
    //         Print(L"[%d] Status: %r\r\n", __LINE__, status);/* TODO: store user-specified screen resolution */
    //         return status;
    //     }

    //     // TODO: enable user-specified screen resolution
    // }

    /* **************************************************
     * *      Jumping and Giving Control to Kernel      *
     * ************************************************** */
    UINTN mem_map_size = 0, mem_map_key = 0, mem_desc_size = 0;
    UINT32 mem_desc_ver = 0;
    EFI_MEMORY_DESCRIPTOR *mem_map_descs = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5, &mem_map_size, mem_map_descs, &mem_map_key, &mem_desc_size, &mem_desc_ver);
    if (status != EFI_BUFFER_TOO_SMALL) {                                   /* memory map key to exit boot service */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    /* If I call GetMemoryMap twice, mem_map_key is reported invalid, why? */
    status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, 2, ImageHandle, mem_map_key);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }
    
    // struct boot_info boot_info = {rsdp, rsdp2, {info->HorizontalResolution, info->VerticalResolution}};
    NickelMain(&boot_info);                                                 /* never returns, hopefully */

    while (1);
    return 0;
}
