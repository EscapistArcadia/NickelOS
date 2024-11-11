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

    EFI_GUID graphics_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics_protocol = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &graphics_guid, NULL, &graphics_protocol);
    if (EFI_ERROR(status)) {
        Print(L"[%d] Status: %r\r\n", __LINE__, status);
        return status;
    }

    Print(L"%u %u %p %u", graphics_protocol->Mode->Info->HorizontalResolution, graphics_protocol->Mode->Info->VerticalResolution, graphics_protocol->Mode->FrameBufferBase, graphics_protocol->Mode->FrameBufferSize);

    for (int i = 0; i < 33333; ++i) {
        *((uint32_t *)graphics_protocol->Mode->FrameBufferBase + 65536 + i) = 0x00FF96;
    }

    return 0;
}
