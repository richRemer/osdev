#include <efi.h>
#include <efilib.h>
#include "efi_lib.h"
#include "efi_string.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    efi_init(ImageHandle, SystemTable);
    efi_clear_screen();
    efi_out(L"Generic EFI version 0.1...\r\n");

    while (TRUE) {
        EFI_INPUT_KEY key = efi_read_key();

        if (key.ScanCode != 0 || key.UnicodeChar != '\0') {
            efi_out(L"key (Scan/Char): %W/%W.\r\n", key.ScanCode, key.UnicodeChar);
        }
    }

    //efi_hang();

    return EFI_SUCCESS;
}

// TODO: figure out how StdErr works
// TODO: the following does nothing
//  SIMPLE_TEXT_OUTPUT_INTERFACE* err = SystemTable->StdErr;
//  err->OutputString(err, L"bootloader does nothing\r\n");
//
// TODO: figure out how TestString works
// TODO: the following does nothing
//  out->TestString(out, L"Test String\r\n");
