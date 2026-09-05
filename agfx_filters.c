#include "agfx_filters.h"

static inline uint8_t clamp_u8(int v) {
    return (v < 0) ? 0 : ((v > 255) ? 255 : (uint8_t)v);
}

static inline int noise_delta(int x, int y, uint32_t seed, uint8_t strength) {
    uint32_t n = (uint32_t)(x * 73856093u) ^ (uint32_t)(y * 19349663u) ^ (seed * 83492791u);
    n ^= n << 13; n ^= n >> 17; n ^= n << 5;
    return (((int)(n & 0xFF) - 128) * (int)strength) / 128;
}

static inline uint32_t apply_acrylic_pixel(uint32_t c, int x, int y, uint32_t tint_argb, uint8_t noise_strength, uint32_t seed) {
    if ((tint_argb >> 24) & 0xFF) c = agfx_blend(c, tint_argb);
    
    if (noise_strength) {
        int d = noise_delta(x, y, seed, noise_strength);
        uint8_t a = (c >> 24) & 0xFF;
        uint8_t r = clamp_u8((int)((c >> 16) & 0xFF) + d);
        uint8_t g = clamp_u8((int)((c >> 8) & 0xFF) + d);
        uint8_t b = clamp_u8((int)(c & 0xFF) + d);
        c = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
    return c;
}

size_t agfx_filters_scratch_u32(int w, int h) {
    if (w <= 0 || h <= 0) return 0;
    return (size_t)w * (size_t)h;
}

static inline void unpack(uint32_t c, int* a, int* r, int* g, int* b) {
    *a = (c >> 24) & 0xFF;
    *r = (c >> 16) & 0xFF;
    *g = (c >> 8) & 0xFF;
    *b = (c) & 0xFF;
}

static inline uint32_t pack(int a, int r, int g, int b) {
    return ((uint32_t)(a & 0xFF) << 24) |
           ((uint32_t)(r & 0xFF) << 16) |
           ((uint32_t)(g & 0xFF) << 8)  |
           ((uint32_t)(b & 0xFF));
}

static void blur_h(agfx_surface_t* s, int x, int y, int w, int h, int radius, uint32_t* dst) {
    int pitch = s->pitch / 4;
    int win = radius * 2 + 1;

    for (int yy = 0; yy < h; yy++) {
        int sy = y + yy;
        uint32_t* src_row = s->buffer + sy * pitch;

        int sumA = 0, sumR = 0, sumG = 0, sumB = 0;

        for (int k = -radius; k <= radius; k++) {
            int sx = x + AGFX_MAX(0, AGFX_MIN(w - 1, k));
            int a,r,g,b;
            unpack(src_row[sx], &a,&r,&g,&b);
            sumA += a; sumR += r; sumG += g; sumB += b;
        }

        for (int xx = 0; xx < w; xx++) {
            int outA = sumA / win;
            int outR = sumR / win;
            int outG = sumG / win;
            int outB = sumB / win;

            dst[yy * w + xx] = pack(outA, outR, outG, outB);

            int left = xx - radius;
            int right = xx + radius + 1;

            int sxL = x + AGFX_MAX(0, AGFX_MIN(w - 1, left));
            int sxR = x + AGFX_MAX(0, AGFX_MIN(w - 1, right));

            int aL,rL,gL,bL;
            int aR,rR,gR,bR;

            unpack(src_row[sxL], &aL,&rL,&gL,&bL);
            unpack(src_row[sxR], &aR,&rR,&gR,&bR);

            sumA += aR - aL;
            sumR += rR - rL;
            sumG += gR - gL;
            sumB += bR - bL;
        }
    }
}

static void blur_v(int w, int h, int radius, const uint32_t* src, uint32_t* dst) {
    int win = radius * 2 + 1;

    for (int xx = 0; xx < w; xx++) {
        int sumA = 0, sumR = 0, sumG = 0, sumB = 0;

        for (int k = -radius; k <= radius; k++) {
            int sy = AGFX_MAX(0, AGFX_MIN(h - 1, k));
            int a,r,g,b;
            unpack(src[sy * w + xx], &a,&r,&g,&b);
            sumA += a; sumR += r; sumG += g; sumB += b;
        }

        for (int yy = 0; yy < h; yy++) {
            int outA = sumA / win;
            int outR = sumR / win;
            int outG = sumG / win;
            int outB = sumB / win;

            dst[yy * w + xx] = pack(outA, outR, outG, outB);

            int top = yy - radius;
            int bot = yy + radius + 1;

            int syT = AGFX_MAX(0, AGFX_MIN(h - 1, top));
            int syB = AGFX_MAX(0, AGFX_MIN(h - 1, bot));

            int aT,rT,gT,bT;
            int aB,rB,gB,bB;

            unpack(src[syT * w + xx], &aT,&rT,&gT,&bT);
            unpack(src[syB * w + xx], &aB,&rB,&gB,&bB);

            sumA += aB - aT;
            sumR += rB - rT;
            sumG += gB - gT;
            sumB += bB - bT;
        }
    }
}

static void agfx_box_blur_to(agfx_surface_t* s, int x, int y, int w, int h, int radius,
                             uint32_t* tmp1, uint32_t* out_tmp2)
{
    if (!s || !s->buffer || !tmp1 || !out_tmp2) return;
    if (radius <= 0) return;

    blur_h(s, x, y, w, h, radius, tmp1);
    blur_v(w, h, radius, tmp1, out_tmp2);
}

void agfx_filter_box_blur(agfx_surface_t* s, int x, int y, int w, int h, int radius,
                          uint32_t* tmp1, uint32_t* tmp2)
{
    if (!s || !s->buffer || !tmp1 || !tmp2) return;
    if (radius <= 0) return;
    if (!agfx_clip_rect(s, &x, &y, &w, &h)) return;

    blur_h(s, x, y, w, h, radius, tmp1);

    blur_v(w, h, radius, tmp1, tmp2);

    int pitch = s->pitch / 4;
    for (int yy = 0; yy < h; yy++) {
        uint32_t* dst_row = s->buffer + (y + yy) * pitch;
        const uint32_t* src_row = tmp2 + yy * w;
        for (int xx = 0; xx < w; xx++) {
            dst_row[x + xx] = src_row[xx];
        }
    }
}

void agfx_filter_glass(agfx_surface_t* s, int x, int y, int w, int h, int radius,
                       uint32_t tint_argb,
                       uint32_t* tmp1, uint32_t* tmp2)
{
    if (!s || !s->buffer) return;
    if (!agfx_clip_rect(s, &x, &y, &w, &h)) return;

    if (radius > 0) {
        agfx_filter_box_blur(s, x, y, w, h, radius, tmp1, tmp2);
    }

    int pitch = s->pitch / 4;
    uint8_t ta = (tint_argb >> 24) & 0xFF;
    if (ta == 0) return;

    for (int yy = 0; yy < h; yy++) {
        uint32_t* row = s->buffer + (y + yy) * pitch;
        for (int xx = 0; xx < w; xx++) {
            int px = x + xx;
            row[px] = agfx_blend(row[px], tint_argb);
        }
    }
}

void agfx_filter_grayscale(agfx_surface_t* s, int x, int y, int w, int h) {
    if (!s || !s->buffer) return;
    if (!agfx_clip_rect(s, &x, &y, &w, &h)) return;

    int pitch = s->pitch / 4;
    for (int yy = 0; yy < h; yy++) {
        uint32_t* row = s->buffer + (y + yy) * pitch;
        for (int xx = 0; xx < w; xx++) {
            uint32_t c = row[x + xx];
            uint8_t a = (c >> 24) & 0xFF;
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = (c) & 0xFF;

            uint8_t yv = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
            row[x + xx] = ((uint32_t)a << 24) | ((uint32_t)yv << 16) | ((uint32_t)yv << 8) | yv;
        }
    }
}

void agfx_filter_invert(agfx_surface_t* s, int x, int y, int w, int h) {
    if (!s || !s->buffer) return;
    if (!agfx_clip_rect(s, &x, &y, &w, &h)) return;

    int pitch = s->pitch / 4;
    for (int yy = 0; yy < h; yy++) {
        uint32_t* row = s->buffer + (y + yy) * pitch;
        for (int xx = 0; xx < w; xx++) {
            uint32_t c = row[x + xx];
            uint8_t a = (c >> 24) & 0xFF;
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = (c) & 0xFF;

            row[x + xx] = ((uint32_t)a << 24) |
                          ((uint32_t)(255 - r) << 16) |
                          ((uint32_t)(255 - g) << 8) |
                          (uint32_t)(255 - b);
        }
    }
}

void agfx_filter_acrylic(agfx_surface_t* s, int x, int y, int w, int h,
                         int blur_radius, uint32_t tint_argb, uint8_t noise_strength,
                         uint32_t seed, uint32_t* tmp1, uint32_t* tmp2)
{
    if (!s || !s->buffer || !tmp1 || !tmp2 || !agfx_clip_rect(s, &x, &y, &w, &h)) return;

    if (blur_radius > 0) agfx_filter_box_blur(s, x, y, w, h, blur_radius, tmp1, tmp2);

    if (((tint_argb >> 24) & 0xFF) == 0 && noise_strength == 0) return;

    int pitch = s->pitch / 4;
    for (int yy = 0; yy < h; yy++) {
        uint32_t* row = s->buffer + (y + yy) * pitch;
        for (int xx = 0; xx < w; xx++) {
            row[x + xx] = apply_acrylic_pixel(row[x + xx], x + xx, y + yy, tint_argb, noise_strength, seed);
        }
    }
}

void agfx_filter_acrylic_masked(agfx_surface_t* s, int x, int y, int w, int h,
                                int blur_radius, uint32_t tint_argb, uint8_t noise_strength,
                                uint32_t seed, const uint8_t* mask, uint32_t* tmp1, uint32_t* tmp2)
{
    if (!s || !s->buffer || !tmp1 || !tmp2 || !mask || !agfx_clip_rect(s, &x, &y, &w, &h)) return;

    if (blur_radius > 0) {
        agfx_box_blur_to(s, x, y, w, h, blur_radius, tmp1, tmp2);
    } else {
        int pitch = s->pitch / 4;
        for (int yy = 0; yy < h; yy++) {
            uint32_t* src_row = s->buffer + (y + yy) * pitch;
            uint32_t* dst_row = tmp2 + yy * w;
            for (int xx = 0; xx < w; xx++) dst_row[xx] = src_row[x + xx];
        }
    }

    int pitch = s->pitch / 4;
    for (int yy = 0; yy < h; yy++) {
        uint32_t* dst_row = s->buffer + (y + yy) * pitch;
        uint32_t* fx_row  = tmp2 + yy * w;

        for (int xx = 0; xx < w; xx++) {
            uint8_t m = mask[yy * w + xx];
            if (m == 0) continue;

            uint32_t c = apply_acrylic_pixel(fx_row[xx], x + xx, y + yy, tint_argb, noise_strength, seed);

            if (m == 255) {
                dst_row[x + xx] = c;
            } else {
                uint8_t a = (c >> 24) & 0xFF;
                uint32_t src = (c & 0x00FFFFFF) | ((uint32_t)((a * m) / 255) << 24);
                dst_row[x + xx] = agfx_blend(dst_row[x + xx], src);
            }
        }
    }
}

void agfx_filter_gradient(agfx_surface_t* s, int x, int y, int w, int h,
                          int gx0, int gy0, int gx1, int gy1,
                          uint32_t color1, uint32_t color2) 
{
    if (!s || !s->buffer || !agfx_clip_rect(s, &x, &y, &w, &h)) return;

    agfx_linear_gradient_t g;
    agfx_gradient_init_lin(&g, gx0, gy0, gx1, gy1, color1, color2);
    int pitch = s->pitch / 4;

    for (int cy = 0; cy < h; cy++) {
        uint32_t* row = s->buffer + ((y + cy) * pitch);
        int64_t py = (int64_t)(y + cy) - g.gy0;
        int64_t dot = ((int64_t)x - g.gx0) * g.dx + py * g.dy;

        for (int cx = 0; cx < w; cx++) {
            int pct = (g.len_sq == 0) ? 0 : (int)((dot * 255) / g.len_sq);
            uint32_t col = agfx_gradient_color_from_percent(&g, pct);
            
            if (g.opaque) row[x + cx] = col;
            else agfx_plot_fast(s, x + cx, y + cy, col);

            dot += g.dx;
        }
    }
}

void agfx_filter_gradient_mask(agfx_surface_t* s, int x, int y, int w, int h,
                               const uint8_t* mask,
                               int gx0, int gy0, int gx1, int gy1,
                               uint32_t color1, uint32_t color2) 
{
    if (!s || !s->buffer || !mask) return;
    
    int orig_x = x, orig_y = y;
    if (!agfx_clip_rect(s, &x, &y, &w, &h)) return;

    agfx_linear_gradient_t g;
    agfx_gradient_init_lin(&g, gx0, gy0, gx1, gy1, color1, color2);
    int pitch = s->pitch / 4;

    for (int cy = 0; cy < h; cy++) {
        uint32_t* row = s->buffer + ((y + cy) * pitch);
        int mask_y = (y + cy) - orig_y;
        int64_t py = (int64_t)(y + cy) - g.gy0;
        int64_t dot = ((int64_t)x - g.gx0) * g.dx + py * g.dy;

        for (int cx = 0; cx < w; cx++) {
            int mask_x = (x + cx) - orig_x;
            uint8_t m = mask[mask_y * w + mask_x];
            
            if (m > 0) {
                int pct = (g.len_sq == 0) ? 0 : (int)((dot * 255) / g.len_sq);
                uint32_t col = agfx_gradient_color_from_percent(&g, pct);
                
                uint8_t a = (col >> 24) & 0xFF;
                a = (uint8_t)((a * m) / 255);
                col = (col & 0x00FFFFFF) | ((uint32_t)a << 24);
                
                row[x + cx] = agfx_blend(row[x + cx], col);
            }
            dot += g.dx;
        }
    }
}