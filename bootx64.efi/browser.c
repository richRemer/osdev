#include "browser.h"
#include "font.h"
#include "efi_lib.h"

static bool browser_handle_key(App*, EFI_INPUT_KEY);
static void browser_init_view(App*);
static void browser_refresh_view(App*);
static void browser_run_app(App*, App*);

App browser_create_app(BrowserView* view) {
    return (App){
        .init_view = browser_init_view,
        .refresh_view = browser_refresh_view,
        .handle_key = browser_handle_key,
        .state = view,
    };
}

static bool browser_handle_key(App* app, EFI_INPUT_KEY key) {
    switch (key.UnicodeChar) {
        case 'f': browser_view_show_font(app->state); break;
        case 'u': app_quit(app); break;
        case 'q': efi_reset_shutdown(); break;
        default: return FALSE;
    }

    return TRUE;
}

static void browser_init_view(App* app) {
    efi_cursor_to(10, 10);
    efi_out_style(EFI_WHITE, L"Choose");

    efi_cursor_to(12, 11);
    efi_out_style(EFI_GREEN, L"f");
    efi_out(L"  Font Viewer");

    efi_cursor_to(12, 12);
    efi_out_style(EFI_GREEN, L"u");
    efi_out(L"  Exit to system UEFI");

    efi_cursor_to(12, 13);
    efi_out_style(EFI_GREEN, L"q");
    efi_out(L"  Shutdown");
}

static void browser_refresh_view(App* app) {
    BrowserView* view = app->state;

    switch (view->show) {
        case BrowserViewShowBrowser:
            break;
        case BrowserViewShowFont:
            App font_app = font_create_app(&view->font);
            browser_run_app(app, &font_app);
            break;
    }

    browser_view_show_browser(view);
}

static void browser_run_app(App* app, App* run_app) {
    app_run(run_app);
    efi_clear_screen();
    browser_init_view(app);
}
