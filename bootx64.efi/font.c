#include "font.h"
#include "efi_lib.h"

static bool font_handle_key(App*, EFI_INPUT_KEY);
static void font_init_view(App*);
static void font_refresh_view(App*);

App font_create_app(FontView* view) {
    return (App){
        .init_view = font_init_view,
        .refresh_view = font_refresh_view,
        .handle_key = font_handle_key,
        .state = view,
    };
}

static bool font_handle_key(App* app, EFI_INPUT_KEY key) {
    FontView* view = app->state;

    switch (key.UnicodeChar) {
        case '\0':
            switch (key.ScanCode) {
                case 3: font_view_next_page(view); break;
                case 4: font_view_prev_page(view); break;
                default: return FALSE;
            }
            break;
        case 'q': app_quit(app); break;
        default: return FALSE;
    }

    return TRUE;
}

static void font_init_view(App* app) {
    efi_cursor_to(0, 2);

    for (int i=0; i<128; i++) {
        efi_out(L"    %B");
        if (i % 8 == 7) efi_out(L"  \r\n");
    }
}

static void font_refresh_view(App* app) {
    FontView* view = app->state;
    uint16_t page = view->page;
    uint8_t last_page = view->page - 1;
    uint8_t next_page = view->page + 1;
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

    efi_cursor_to(4, 20);
    efi_out_style(EFI_WHITE, L"%B", last_page);
    efi_cursor_to(25, 20);
    efi_out_style(EFI_WHITE, L"%B", page);
    efi_cursor_to(46, 20);
    efi_out_style(EFI_WHITE, L"%B", next_page);
}
