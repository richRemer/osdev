#pragma once

#include <stdint.h>

typedef enum BrowserViewShow {
    BrowserViewShowBrowser,
    BrowserViewShowFont,
} BrowserViewShow;

typedef struct FontView {
    uint8_t page;
} FontView;

typedef struct BrowserView {
    BrowserViewShow show;
    FontView font;
} BrowserView;

BrowserView browser_view_create();
void browser_view_show_browser(BrowserView*);
void browser_view_show_font(BrowserView*);
BrowserViewShow browser_view_showing(BrowserView*);
void font_view_next_page(FontView*);
void font_view_prev_page(FontView*);

