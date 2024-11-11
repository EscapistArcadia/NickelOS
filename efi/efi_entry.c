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

    EFI_GUID sfs_proto_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs_proto = NULL;
    EFI_FILE_PROTOCOL *fs_proto = NULL, *kernel_file = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &sfs_proto_guid, NULL, (VOID **)&sfs_proto);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    status = uefi_call_wrapper(sfs_proto->OpenVolume, 2, sfs_proto, &fs_proto);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    // status = uefi_call_wrapper(fs_proto->Open, 5, fs_proto, &kernel_file, L"cyan.exe", EFI_FILE_MODE_READ, 0);
    // if (EFI_ERROR(status)) {                                /* trys to open the kernel executable */
    //     Print(L"[%d] Status: %r; Failed to load the kernel.\r\n", __LINE__, status);
    //     return status;
    // }

    EFI_GUID graphics_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics_protocol = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &graphics_guid, NULL, &graphics_protocol);
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
        // if (info->HorizontalResolution == 1280 && info->VerticalResolution == 960) {
        //     status = uefi_call_wrapper(graphics_protocol->SetMode, 2, graphics_protocol, i);
        //     if (EFI_ERROR(status)) {                        /* set the mode */
        //         Print(L"[%d] Status: %r\r\n", __LINE__, status);
        //         return status;
        //     }
        //     break;
        // }
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

    while (1);
    return 0;
}
