#pragma once

#include "font_view.h"

typedef enum BrowserViewShow {
    BrowserViewShowBrowser,
    BrowserViewShowFont,
} BrowserViewShow;

typedef struct BrowserView {
    BrowserViewShow show;
    FontView font;
} BrowserView;

BrowserView browser_view_create();
void browser_view_show_browser(BrowserView*);
void browser_view_show_font(BrowserView*);
BrowserViewShow browser_view_showing(BrowserView*);
