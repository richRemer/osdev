#include <efi.h>
#include <efilib.h>
#include "efi_lib.h"
#include "browser.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    efi_init(ImageHandle, SystemTable);
    efi_clear_screen();
    efi_out(L"Generic EFI version 0.1\r\n");

    BrowserView app_view = browser_view_create();
    App app = browser_create_app(&app_view);
    app_run(&app);

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
