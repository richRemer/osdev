#include "font_view.h"

void font_view_next_page(FontView* view) {
    view->page++;
}

void font_view_prev_page(FontView* view) {
    view->page--;
}
