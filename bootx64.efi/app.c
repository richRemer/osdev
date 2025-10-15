#include "app.h"
#include "efi_lib.h"

static bool app_handle_key_default(App*, EFI_INPUT_KEY);

void app_quit(App* app) {
    app->status |= AppStatusQuitting;
}

void app_run(App* app) {
    efi_clear_screen();

    if (app->start) {
        app->start(app);
    }

    if (app->init_view) {
        app->init_view(app);
    }

    while (app->status == AppStatusRunning) {
        if (app->refresh_view) {
            app->refresh_view(app);
        }

        EFI_INPUT_KEY key = efi_read_key();

        if (app->handle_key && app->handle_key(app, key)) {
            // app handler
        } else if (app_handle_key_default(app, key)) {
            // default handler;
        }
    }

    app->status = AppStatusQuit;
}

static bool app_handle_key_default(App* app, EFI_INPUT_KEY key) {
    switch (key.UnicodeChar) {
        case 'q': app_quit(app); break;
        case 'r': break;
        default: return FALSE;
    }

    return TRUE;
}
