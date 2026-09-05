#ifndef AGFX_H
#define AGFX_H

#include <stdint.h>
#include <stddef.h>

#ifndef AGFX_ABS
#define AGFX_ABS(x) ((x) < 0 ? -(x) : (x))
#endif

static inline uint64_t agfx_isqrt(uint64_t n) {
    uint64_t root = 0;
    uint64_t bit = 1ULL << 62;
    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

typedef struct {
    uint32_t* buffer; 
    int width;        
    int height;       
    int pitch;
	int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
} agfx_surface_t;

typedef struct {
    int x;
    int y;
} agfx_point_t;

static inline uint32_t agfx_blend(uint32_t bg_color, uint32_t fg_color) {
    uint8_t alpha = (fg_color >> 24) & 0xFF;
    if (alpha == 255) return fg_color;
    if (alpha == 0) return bg_color;
    uint8_t inv_alpha = 255 - alpha;
    uint8_t r_bg = (bg_color >> 16) & 0xFF, g_bg = (bg_color >> 8) & 0xFF, b_bg = bg_color & 0xFF;
    uint8_t r_fg = (fg_color >> 16) & 0xFF, g_fg = (fg_color >> 8) & 0xFF, b_fg = fg_color & 0xFF;
    uint8_t r = (r_fg * alpha + r_bg * inv_alpha) / 255;
    uint8_t g = (g_fg * alpha + g_bg * inv_alpha) / 255;
    uint8_t b = (b_fg * alpha + b_bg * inv_alpha) / 255;
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void agfx_init(agfx_surface_t* surface, void* buffer, int width, int height, int pitch);
void agfx_set_clip(agfx_surface_t* surface, int x, int y, int w, int h);

void agfx_draw_pixel(agfx_surface_t* surface, int x, int y, uint32_t color);
void agfx_draw_line(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int thickness, uint32_t color);
void agfx_draw_bezier(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int x2, int y2, int thickness, uint32_t color);

void agfx_draw_rect(agfx_surface_t* surface, int x, int y, int w, int h, int thickness, uint32_t color);
void agfx_fill_rect(agfx_surface_t* surface, int x, int y, int w, int h, uint32_t color);
void agfx_draw_rect_rounded(agfx_surface_t* surface, int x, int y, int w, int h, int r, int thickness, uint32_t color);
void agfx_fill_rect_rounded(agfx_surface_t* surface, int x, int y, int w, int h, int r, uint32_t color);

void agfx_fill_alpha_mask(agfx_surface_t* surface, int x, int y, int w, int h,
						  const uint8_t* mask, uint32_t color);
void agfx_blit(agfx_surface_t* dest, int x, int y, agfx_surface_t* src);
void agfx_blit_scaled(agfx_surface_t* dest, int x, int y, int dest_w, int dest_h, agfx_surface_t* src);

void agfx_draw_circle(agfx_surface_t* surface, int xc, int yc, int r, uint32_t color);
void agfx_fill_circle(agfx_surface_t* surface, int xc, int yc, int r, uint32_t color);
void agfx_draw_triangle(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int x2, int y2, int thickness, uint32_t color);
void agfx_fill_triangle(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void agfx_draw_polygon(agfx_surface_t* surface, agfx_point_t* points, int count, int thickness, uint32_t color);
void agfx_fill_polygon(agfx_surface_t* surface, agfx_point_t* points, int count, uint32_t color);

#endif // AGFX_H