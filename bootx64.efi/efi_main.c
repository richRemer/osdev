#include <efi.h>
#include <efilib.h>
#include "efi_lib.h"
#include "efi_string.h"

static void ascii_table();

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    efi_init(ImageHandle, SystemTable);
    efi_clear_screen();

    efi_out(L"Generic ");
    efi_out_style(EFI_LIGHTBLUE, L"EFI");
    efi_out(L" version 0.1...\r\n");

    while (TRUE) {
        EFI_INPUT_KEY key = efi_read_key();

        switch (key.UnicodeChar) {
            case 'A':
            case 'a': ascii_table(); break;
            case 'Q':
            case 'q': return EFI_SUCCESS;
        }
    }

    efi_hang();

    return EFI_SUCCESS;
}

static void ascii_table() {
    CHAR16* format = L"  %B .";

    for (int i=0; i<128; i++) {
        format[5] = i;
        efi_out(format, i);
        if (i % 8 == 7) efi_out(L"\r\n");
    }
}

// TODO: figure out how StdErr works
// TODO: the following does nothing
//  SIMPLE_TEXT_OUTPUT_INTERFACE* err = SystemTable->StdErr;
//  err->OutputString(err, L"bootloader does nothing\r\n");
//
// TODO: figure out how TestString works
// TODO: the following does nothing
//  out->TestString(out, L"Test String\r\n");
