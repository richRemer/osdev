#include "efi_lib.h"
#include "efi_string.h"

static EFI_HANDLE efi_image = NULL;
static EFI_SYSTEM_TABLE* efi = NULL;

void efi_clear_screen() {
    efi->ConOut->ClearScreen(efi->ConOut);
}

void efi_cursor_disable() {
    efi->ConOut->EnableCursor(efi->ConOut, FALSE);
}

void efi_cursor_enable() {
    efi->ConOut->EnableCursor(efi->ConOut, TRUE);
}

void efi_hang() {
    for (;;) ;
}

void efi_init(EFI_HANDLE image, EFI_SYSTEM_TABLE* system) {
    efi_image = image;
    efi = system;
}

void efi_out(const CHAR16* format, ...) {
    // TODO: break into chunks to accomodate static buffer
    CHAR16* buf[255];

    va_list args;
    va_start(args, format);
    vfmt((CHAR16*)buf, 255, format, args);
    va_end(args);

    efi->ConOut->OutputString(efi->ConOut, (CHAR16*)buf);
}

EFI_INPUT_KEY efi_read_key() {
    SIMPLE_INPUT_INTERFACE* input = efi->ConIn;
    EFI_INPUT_KEY key;

    // TODO: remove this once verified unnecessary
    key.ScanCode = 0;
    key.UnicodeChar = '\0';

    input->ReadKeyStroke(input, &key);

    return key;
}
