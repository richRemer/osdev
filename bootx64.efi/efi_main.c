#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;
    SIMPLE_TEXT_OUTPUT_INTERFACE* err = SystemTable->StdErr;

    out->OutputString(out, L"booting custom EFI\n");
    err->OutputString(err, L"bootloader does nothing\n");

    for (;;) ;

    return EFI_SUCCESS;
}
