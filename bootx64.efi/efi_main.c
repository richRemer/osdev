#include <efi.h>
#include <efilib.h>
#include "efi_string.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;

    out->ClearScreen(out);
    out->OutputString(out, L"booting custom EFI\r\n");

    out->OutputString(out, uint32_str(out->Mode->Attribute));
    out->OutputString(out, L" (Attribute)\r\n");

    out->OutputString(out, uint32_str(out->Mode->CursorColumn));
    out->OutputString(out, L" (CursorColumn)\r\n");

    out->OutputString(out, uint32_str(out->Mode->CursorRow));
    out->OutputString(out, L" (CursorRow)\r\n");

    out->OutputString(out, uint8_str(out->Mode->CursorVisible));
    out->OutputString(out, L" (CursorVisible)\r\n");

    out->OutputString(out, uint32_str(out->Mode->MaxMode));
    out->OutputString(out, L" (MaxMode)\r\n");

    out->OutputString(out, uint32_str(out->Mode->Mode));
    out->OutputString(out, L" (Mode)\r\n");

    for (INT32 mode=0; mode<=out->Mode->MaxMode; mode++) {
        UINTN cols;
        UINTN rows;

        out->QueryMode(out, mode, &cols, &rows);
        out->OutputString(out, L"Mode ");
        out->OutputString(out, uint32_str(mode));
        out->OutputString(out, L" [");
        out->OutputString(out, uint64_str(cols));
        out->OutputString(out, L"x");
        out->OutputString(out, uint64_str(rows));
        out->OutputString(out, L"]\r\n");
    }

    out->EnableCursor(out, TRUE);

    for (;;) ;

    return EFI_SUCCESS;
}

// TODO: figure out how StdErr works
// TODO: the following does nothing
//  SIMPLE_TEXT_OUTPUT_INTERFACE* err = SystemTable->StdErr;
//  err->OutputString(err, L"bootloader does nothing\r\n");
