#pragma once

#include <stdint.h>

typedef struct FontView {
    uint8_t page;
} FontView;

void font_view_next_page(FontView*);
void font_view_prev_page(FontView*);
