#ifndef AGFX_FILTERS_H
#define AGFX_FILTERS_H

#include "agfx.h"

typedef struct {
    int gx0, gy0;
    int64_t dx, dy;
    int64_t len_sq;
    uint8_t a1, r1, g1, b1;
    uint8_t a2, r2, g2, b2;
    int opaque;
} agfx_linear_gradient_t;

size_t agfx_filters_scratch_u32(int w, int h);

static inline void agfx_gradient_init_lin(agfx_linear_gradient_t* g,
                                          int gx0, int gy0, int gx1, int gy1,
                                          uint32_t c1, uint32_t c2) {
    g->gx0 = gx0; g->gy0 = gy0;
    g->dx = (int64_t)gx1 - gx0;
    g->dy = (int64_t)gy1 - gy0;
    g->len_sq = g->dx * g->dx + g->dy * g->dy;

    g->a1 = (c1 >> 24) & 0xFF; g->r1 = (c1 >> 16) & 0xFF; g->g1 = (c1 >> 8) & 0xFF; g->b1 = c1 & 0xFF;
    g->a2 = (c2 >> 24) & 0xFF; g->r2 = (c2 >> 16) & 0xFF; g->g2 = (c2 >> 8) & 0xFF; g->b2 = c2 & 0xFF;
    g->opaque = (g->a1 == 255 && g->a2 == 255);
}

static inline uint32_t agfx_gradient_color_from_percent(const agfx_linear_gradient_t* g, int percent) {
    if (percent < 0) percent = 0;
    if (percent > 255) percent = 255;
    int inv = 255 - percent;
    uint8_t a = (uint8_t)((g->a1 * inv + g->a2 * percent) / 255);
    uint8_t r = (uint8_t)((g->r1 * inv + g->r2 * percent) / 255);
    uint8_t gg= (uint8_t)((g->g1 * inv + g->g2 * percent) / 255);
    uint8_t b = (uint8_t)((g->b1 * inv + g->b2 * percent) / 255);
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)gg << 8) | (uint32_t)b;
}

void agfx_filter_gradient(agfx_surface_t* s, int x, int y, int w, int h,
                          int gx0, int gy0, int gx1, int gy1,
                          uint32_t color1, uint32_t color2);

void agfx_filter_gradient_mask(agfx_surface_t* s, int x, int y, int w, int h,
                               const uint8_t* mask,
                               int gx0, int gy0, int gx1, int gy1,
                               uint32_t color1, uint32_t color2);

void agfx_filter_box_blur(agfx_surface_t* s, int x, int y, int w, int h, int radius,
                          uint32_t* tmp1, uint32_t* tmp2);

void agfx_filter_glass(agfx_surface_t* s, int x, int y, int w, int h, int radius,
                       uint32_t tint_argb,
                       uint32_t* tmp1, uint32_t* tmp2);

void agfx_filter_grayscale(agfx_surface_t* s, int x, int y, int w, int h);
void agfx_filter_invert(agfx_surface_t* s, int x, int y, int w, int h);

void agfx_filter_acrylic(agfx_surface_t* s, int x, int y, int w, int h,
                         int blur_radius,
                         uint32_t tint_argb,
                         uint8_t noise_strength,
                         uint32_t seed,
                         uint32_t* tmp1, uint32_t* tmp2);

void agfx_filter_acrylic_masked(agfx_surface_t* s,
                                int x, int y, int w, int h,
                                int blur_radius,
                                uint32_t tint_argb,
                                uint8_t noise_strength,
                                uint32_t seed,
                                const uint8_t* mask,
                                uint32_t* tmp1, uint32_t* tmp2);

#endif // AGFX_FILTERS_H