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
 * @return 
 */
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS status = uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    if (EFI_ERROR(status)) {                                /* clears the predefined logo */
        Print(L"[%d] Status: %r\r\n", __LINE__, status);    /* print the error message */
        return status;
    }
    
    while (1);
    return 0;
}