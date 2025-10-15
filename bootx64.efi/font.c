#include "font.h"
#include "font_view.h"
#include "efi_lib.h"

static void font_init_view(App* app) {
    efi_cursor_to(0, 2);

    for (int i=0; i<128; i++) {
        efi_out(L"    %B");
        if (i % 8 == 7) efi_out(L"\r\n");
    }
}

static void font_refresh_view(App* app) {
    FontView* view = app->state;
    uint16_t page = view->page;
    CHAR16 chr[2];

    chr[1] = '\0';
    
    for (uint16_t row=0; row<8; row++) {
        for (uint16_t col=0; col<8; col++) {
            chr[0] = (page << 8) | (row << 4) | col;
            efi_cursor_to(6*col+7, 2+row);
            efi_out(L" ");
            efi_cursor_to(6*col+7, 2+row);
            efi_out(chr);
        }
    }
}

static bool font_handle_key(App* app, EFI_INPUT_KEY key) {
    switch (key.UnicodeChar) {
        case 'q': app_quit(app); break;
        //default: return FALSE;
        // TODO: pagination
        default:
            efi_cursor_to(0, 0);
            efi_out(L"%W/%W", key.ScanCode, key.UnicodeChar);
    }

    return TRUE;
}

App font_create_app(FontView* view) {
    return (App){
        .init_view = font_init_view,
        .refresh_view = font_refresh_view,
        .handle_key = font_handle_key,
        .state = view,
    };
}

