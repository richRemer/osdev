#pragma once

#include <stdbool.h>
#include <efi.h>

typedef struct App App;

typedef void (*app_start_call)(App*);
typedef void (*app_init_view_call)(App*);
typedef void (*app_refresh_view_call)(App*);
typedef bool (*app_handle_key_call)(App*, EFI_INPUT_KEY);

typedef enum AppStatus {
    AppStatusRunning = 0,
    AppStatusQuitting = 0x1,
    AppStatusQuit = 0x2,
} AppStatus;

typedef struct App {
    void* state;
    AppStatus status;

    app_start_call start;
    app_init_view_call init_view;
    app_refresh_view_call refresh_view;
    app_handle_key_call handle_key;
} App;

void app_quit(App*);
void app_run(App*);
