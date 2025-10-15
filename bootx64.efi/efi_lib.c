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

void efi_cursor_to(uint8_t col, uint8_t row) {
    efi->ConOut->SetCursorPosition(efi->ConOut, col, row);
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
    CHAR16 buf[255];

    va_list args;
    va_start(args, format);
    vfmt((CHAR16*)buf, 255, format, args);
    va_end(args);

    efi->ConOut->OutputString(efi->ConOut, (CHAR16*)buf);
}

void efi_out_style(uint64_t style, const CHAR16* format, ...) {
    // TODO: break into chunks to accomodate static buffer
    CHAR16 buf[255];
    uint64_t restore = efi->ConOut->Mode->Attribute;

    efi->ConOut->SetAttribute(efi->ConOut, style);

    va_list args;
    va_start(args, format);
    vfmt((CHAR16*)buf, 255, format, args);
    va_end(args);

    efi->ConOut->OutputString(efi->ConOut, (CHAR16*)buf);
    efi->ConOut->SetAttribute(efi->ConOut, restore);
}

EFI_INPUT_KEY efi_read_key() {
    SIMPLE_INPUT_INTERFACE* input = efi->ConIn;
    EFI_EVENT wait = input->WaitForKey;
    EFI_INPUT_KEY key;
    EFI_STATUS status;
    UINTN index;

    status = efi->BootServices->WaitForEvent(1, &wait, &index);

    if (!EFI_ERROR(status)) {
        input->ReadKeyStroke(input, &key);
    }

    return key;
}

void efi_watchdog_disable() {
    efi->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
}
