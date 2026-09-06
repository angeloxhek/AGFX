#ifndef AGFX_TEXT_H
#define AGFX_TEXT_H

#include "agfx.h"

typedef struct {
    void* info;
    float scale;
    int ascent;
    int descent;
    int line_gap;
} agfx_font_t;

int agfx_font_init(agfx_font_t* font, const uint8_t* ttf_buffer, float pixel_height);

void agfx_font_destroy(agfx_font_t* font);
void agfx_mask_free(void* mask);

uint8_t* agfx_mask_generate_string(const agfx_font_t* font, const char* text, int* out_w, int* out_h);

#endif // AGFX_TEXT_H