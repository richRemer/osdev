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
        case 'q': app_quit(app); break;
        default: return FALSE;
    }

    return TRUE;
}

static void browser_init_view(App* app) {
    efi_out(L"\r\n");
    efi_out_style(EFI_WHITE, L"Choose one of the following\r\n");
    efi_out(L"  "); efi_out_style(EFI_GREEN, L"F"); efi_out(L"ont viewer\r\n");
    efi_out(L"  "); efi_out_style(EFI_GREEN, L"Q"); efi_out(L"uit\r\n");
    efi_out_style(EFI_WHITE, L"Choose an option from above\r\n");
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
