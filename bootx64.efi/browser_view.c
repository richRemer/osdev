#include "browser_view.h"

BrowserView browser_view_create() {
    return (BrowserView){
        .show = BrowserViewShowBrowser,
        .font = (FontView){
            .page = 0,
        },
    };
}

void browser_view_show_browser(BrowserView* view) {
    view->show = BrowserViewShowBrowser;
}

void browser_view_show_font(BrowserView* view) {
    view->show = BrowserViewShowFont;
}

BrowserViewShow browser_view_showing(BrowserView* view) {
    switch (view->show) {
        case BrowserViewShowBrowser: return view->show;

        case BrowserViewShowFont:
        default: return BrowserViewShowFont;
    }
}
