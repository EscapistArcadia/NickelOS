#include <efi.h>
#include <efilib.h>
#include <elf.h>

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
    InitializeLib(ImageHandle, SystemTable);                /* must do this */

    EFI_STATUS status = uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    if (EFI_ERROR(status)) {                                /* clears the predefined logo */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);    /* print the error message */
        return status;
    }
    
    status = uefi_call_wrapper(SystemTable->BootServices->SetWatchdogTimer, 4, 0, 0, 0, NULL);
    if (EFI_ERROR(status)) {                                /* disables the watchdog or UEFI will be rebooted */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_proto = NULL;
    EFI_FILE_PROTOCOL *fs_proto = NULL, *kernel_file = NULL;
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
    if (EFI_ERROR(status)) {                                /* trys to open the kernel executable */
        Print(L"[%d] Status: %r; Failed to load the kernel.\r\n", __LINE__, status);
        return status;
    }

    UINTN kernel_file_info_size = sizeof(EFI_FILE_INFO) + 128;
    EFI_FILE_INFO *kernel_file_info = NULL;
    status = uefi_call_wrapper(kernel_file->GetInfo, 4, kernel_file, &gEfiFileInfoGuid, &kernel_file_info_size, kernel_file_info);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    EFI_PHYSICAL_ADDRESS kernel_address = 0x100000;
    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, (kernel_file_info->FileSize + 0xFFF) >> 12, kernel_address);
    if (EFI_ERROR(status)) {                                /* allocate kerenl for kernel executable */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &kernel_file_info->FileSize, kernel_address);
    if (EFI_ERROR(status)) {
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

    UINTN mem_map_size = 0, mem_map_key = 0, mem_desc_size = 0;
    UINT32 mem_desc_ver = 0;
    EFI_MEMORY_DESCRIPTOR *mem_map_descs = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5, &mem_map_size, mem_map_descs, &mem_map_key, &mem_desc_size, &mem_desc_ver);
    if (status != EFI_BUFFER_TOO_SMALL) {                   /* memory map key to exit boot service */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    /* If I call GetMemoryMap twice, mem_map_key is reported invalid, why? */
    status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, 2, ImageHandle, mem_map_key);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    Elf64_Ehdr *kernel_header = (Elf64_Ehdr *)kernel_address;
    void (*NickelMain)() = (void (*)())(kernel_header->e_entry);
    NickelMain();                                           /* never returns, hopefully */

    EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics_protocol = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, &graphics_protocol);
    if (EFI_ERROR(status)) {                                /* acquires frame buffer and modes */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    UINTN info_size;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;             /* enumerate each mode to find predefined mode */
    for (UINT32 i = 0; i < graphics_protocol->Mode->MaxMode; ++i) {
        status = uefi_call_wrapper(graphics_protocol->QueryMode, 4, graphics_protocol, i, &info_size, &info);
        if (EFI_ERROR(status)) {
            Print(L"[%d] Status: %r\r\n", __LINE__, status);/* TODO: store user-specified screen resolution */
            return status;
        }
        // if (info->HorizontalResolution == 1920 && info->VerticalResolution == 1080) {
        //     status = uefi_call_wrapper(graphics_protocol->SetMode, 2, graphics_protocol, i);
        //     if (EFI_ERROR(status)) {                        /* set the mode */
        //         Print(L"[%d] Status: %r\r\n", __LINE__, status);
        //         return status;
        //     }
        //     break;
        // }
    }

    while (1);
    return 0;
}
