#include "agfx.h"

void agfx_init(agfx_surface_t* surface, uint32_t* buffer, int width, int height, int pitch) {
    if (!surface) return;
    surface->buffer = buffer;
    surface->width = width;
    surface->height = height;
    surface->pitch = pitch;
    agfx_set_clip(surface, 0, 0, width, height);
}

void agfx_set_clip(agfx_surface_t* surface, int x, int y, int w, int h) {
    if (!surface) return;
    agfx_surface_t screen_bounds = { .clip_x = 0, .clip_y = 0, .clip_w = surface->width, .clip_h = surface->height };
    if (agfx_clip_rect(&screen_bounds, &x, &y, &w, &h)) {
        surface->clip_x = x; surface->clip_y = y;
        surface->clip_w = w; surface->clip_h = h;
    } else {
        surface->clip_w = 0; surface->clip_h = 0;
    }
}

void agfx_draw_pixel(agfx_surface_t* surface, int x, int y, uint32_t color) {
    if (!surface || !surface->buffer) return;
    if (x >= surface->clip_x && x < surface->clip_x + surface->clip_w && 
        y >= surface->clip_y && y < surface->clip_y + surface->clip_h) {
        agfx_plot_fast(surface, x, y, color);
    }
}

void agfx_fill_rect(agfx_surface_t* surface, int x, int y, int w, int h, uint32_t color) {
    if (!surface || !surface->buffer || !agfx_clip_rect(surface, &x, &y, &w, &h)) return;

    int pitch_pixels = surface->pitch / 4;
    uint8_t alpha = (color >> 24) & 0xFF;
    if (alpha == 0) return;

    for (int cy = y; cy < y + h; cy++) {
        uint32_t* row = surface->buffer + (cy * pitch_pixels);
        for (int cx = x; cx < x + w; cx++) {
            row[cx] = (alpha == 255) ? color : agfx_blend(row[cx], color);
        }
    }
}

void agfx_draw_line(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int thickness, uint32_t color) {
    if (!surface || !surface->buffer || thickness <= 0) return;

    if (thickness == 1) {
        int dx = AGFX_ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -AGFX_ABS(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        while (1) {
            agfx_draw_pixel(surface, x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        return;
    }

    int pad = (thickness / 2) + 1;
    int min_x = (x0 < x1 ? x0 : x1) - pad;
    int max_x = (x0 > x1 ? x0 : x1) + pad;
    int min_y = (y0 < y1 ? y0 : y1) - pad;
    int max_y = (y0 > y1 ? y0 : y1) + pad;

    int bw = max_x - min_x + 1;
    int bh = max_y - min_y + 1;
    if (!agfx_clip_rect(surface, &min_x, &min_y, &bw, &bh)) return;
    max_x = min_x + bw - 1;
    max_y = min_y + bh - 1;

    int64_t l2 = (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0);
    int64_t r_sq = (thickness * thickness) / 4;
    int pitch_pixels = surface->pitch / 4;
    uint8_t alpha = (color >> 24) & 0xFF;

    for (int py = min_y; py <= max_y; py++) {
        uint32_t* row = surface->buffer + (py * pitch_pixels);
        for (int px = min_x; px <= max_x; px++) {
            int64_t dist_sq = 0;

            if (l2 == 0) {
                dist_sq = (px - x0)*(px - x0) + (py - y0)*(py - y0);
            } else {
                int64_t t = ((px - x0)*(x1 - x0) + (py - y0)*(y1 - y0));
                if (t < 0) dist_sq = (px - x0)*(px - x0) + (py - y0)*(py - y0);
                else if (t > l2) dist_sq = (px - x1)*(px - x1) + (py - y1)*(py - y1);
                else {
                    int64_t cross = (y1 - y0)*px - (x1 - x0)*py + x1*y0 - y1*x0;
                    dist_sq = (cross * cross) / l2;
                }
            }

            if (dist_sq <= r_sq) {
                if (alpha == 255) row[px] = color;
                else if (alpha > 0) row[px] = agfx_blend(row[px], color);
            }
        }
    }
}

void agfx_draw_circle(agfx_surface_t* surface, int xc, int yc, int r, uint32_t color) {
    if (!surface) return;

    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        agfx_draw_pixel(surface, xc + x, yc + y, color);
        agfx_draw_pixel(surface, xc - x, yc + y, color);
        agfx_draw_pixel(surface, xc + x, yc - y, color);
        agfx_draw_pixel(surface, xc - x, yc - y, color);
        agfx_draw_pixel(surface, xc + y, yc + x, color);
        agfx_draw_pixel(surface, xc - y, yc + x, color);
        agfx_draw_pixel(surface, xc + y, yc - x, color);
        agfx_draw_pixel(surface, xc - y, yc - x, color);

        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void agfx_fill_alpha_mask(agfx_surface_t* surface, int x, int y, int w, int h, const uint8_t* mask, uint32_t color) {
    if (!surface || !surface->buffer || !mask) return;
    
    int orig_x = x, orig_y = y;
    if (!agfx_clip_rect(surface, &x, &y, &w, &h)) return;

    uint8_t text_r = (color >> 16) & 0xFF, text_g = (color >> 8) & 0xFF, text_b = color & 0xFF, text_a = (color >> 24) & 0xFF; 
    int pitch_pixels = surface->pitch / 4;

    for (int cy = 0; cy < h; cy++) {
        uint32_t* row = surface->buffer + ((y + cy) * pitch_pixels);
        int mask_y = (y + cy) - orig_y;
        
        for (int cx = 0; cx < w; cx++) {
            int mask_x = (x + cx) - orig_x;
            uint8_t mask_val = mask[mask_y * w + mask_x];
            if (mask_val == 0) continue;

            uint8_t final_alpha = (mask_val * text_a) / 255;
            uint32_t src_color = (final_alpha << 24) | (text_r << 16) | (text_g << 8) | text_b;
            row[x + cx] = agfx_blend(row[x + cx], src_color);
        }
    }
}

void agfx_fill_circle(agfx_surface_t* surface, int xc, int yc, int r, uint32_t color) {
    if (!surface) return;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        agfx_fill_rect(surface, xc - x, yc + y, x * 2 + 1, 1, color);
        agfx_fill_rect(surface, xc - x, yc - y, x * 2 + 1, 1, color);
        agfx_fill_rect(surface, xc - y, yc + x, y * 2 + 1, 1, color);
        agfx_fill_rect(surface, xc - y, yc - x, y * 2 + 1, 1, color);

        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void agfx_blit(agfx_surface_t* dest, int x, int y, int dest_w, int dest_h, const agfx_surface_t* src) {
    if (!dest || !dest->buffer || !src || !src->buffer || dest_w <= 0 || dest_h <= 0) return;

    uint32_t dx = (src->width << 16) / dest_w;
    uint32_t dy = (src->height << 16) / dest_h;

    int orig_x = x, orig_y = y;
    int w = dest_w, h = dest_h;
    if (!agfx_clip_rect(dest, &x, &y, &w, &h)) return;

    int dest_pitch = dest->pitch / 4;
    int src_pitch = src->pitch / 4;

    for (int cy = 0; cy < h; cy++) {
        int screen_y = y + cy;
        int src_y = ((screen_y - orig_y) * dy) >> 16;
        
        uint32_t* dest_row = dest->buffer + (screen_y * dest_pitch);
        uint32_t* src_row = src->buffer + (src_y * src_pitch);

        for (int cx = 0; cx < w; cx++) {
            int screen_x = x + cx;
            int src_x = ((screen_x - orig_x) * dx) >> 16;

            uint32_t fg = src_row[src_x];
            uint8_t alpha = (fg >> 24) & 0xFF;

            if (alpha == 255) dest_row[screen_x] = fg;
            else if (alpha > 0) dest_row[screen_x] = agfx_blend(dest_row[screen_x], fg);
        }
    }
}

void agfx_draw_triangle(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int x2, int y2, int thickness, uint32_t color) {
    agfx_draw_line(surface, x0, y0, x1, y1, thickness, color);
    agfx_draw_line(surface, x1, y1, x2, y2, thickness, color);
    agfx_draw_line(surface, x2, y2, x0, y0, thickness, color);
}

void agfx_fill_triangle(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    if (!surface) return;

    if (y0 > y1) { AGFX_SWAP(y0, y1, int); AGFX_SWAP(x0, x1, int); }
    if (y0 > y2) { AGFX_SWAP(y0, y2, int); AGFX_SWAP(x0, x2, int); }
    if (y1 > y2) { AGFX_SWAP(y1, y2, int); AGFX_SWAP(x1, x2, int); }

    int total_height = y2 - y0;
    if (total_height == 0) return;

    for (int i = 0; i <= total_height; i++) {
        int second_half = i > (y1 - y0) || y1 == y0;
        int segment_height = second_half ? (y2 - y1) : (y1 - y0);
        
        if (segment_height == 0) continue;

        int dx_long = x2 - x0;
        int x_long = x0 + (dx_long * i) / total_height;

        int x_short;
        if (!second_half) {
            int dx_short = x1 - x0;
            x_short = x0 + (dx_short * i) / segment_height;
        } else {
            int dx_short = x2 - x1;
            x_short = x1 + (dx_short * (i - (y1 - y0))) / segment_height;
        }

        if (x_long > x_short) AGFX_SWAP(x_long, x_short, int);

        agfx_fill_rect(surface, x_long, y0 + i, x_short - x_long + 1, 1, color);
    }
}

void agfx_fill_rect_rounded(agfx_surface_t* surface, int x, int y, int w, int h, int r, uint32_t color) {
    if (!surface) return;
    
    int max_r = (w < h ? w : h) / 2;
    if (r > max_r) r = max_r;
    if (r <= 0) { agfx_fill_rect(surface, x, y, w, h, color); return; }

    int cx = 0, cy = r, d = 3 - 2 * r;
    
    while (cx <= cy) {
        if (d > 0 || cx == cy) {
            agfx_fill_rect(surface, x + r - cx, y + r - cy, w - 2*r + 2*cx, 1, color);
            agfx_fill_rect(surface, x + r - cx, y + h - r + cy - 1, w - 2*r + 2*cx, 1, color);
        }
        
        if (cx > 0 && cx != cy) {
            agfx_fill_rect(surface, x + r - cy, y + r - cx, w - 2*r + 2*cy, 1, color);
            agfx_fill_rect(surface, x + r - cy, y + h - r + cx - 1, w - 2*r + 2*cy, 1, color);
        }

        cx++;
        if (d > 0) {
            cy--;
            d = d + 4 * (cx - cy) + 10;
        } else {
            d = d + 4 * cx + 6;
        }
    }
    
    agfx_fill_rect(surface, x, y + r, w, h - 2*r, color);
}

void agfx_draw_polygon(agfx_surface_t* surface, const agfx_point_t* points, int count, int thickness, uint32_t color) {
    if (!surface || !points || count < 3 || thickness <= 0) return;

    for (int i = 0; i < count - 1; i++) {
        agfx_draw_line(surface, points[i].x, points[i].y, points[i+1].x, points[i+1].y, thickness, color);
    }
    agfx_draw_line(surface, points[count - 1].x, points[count - 1].y, points[0].x, points[0].y, thickness, color);
}

void agfx_draw_bezier(agfx_surface_t* surface, int x0, int y0, int x1, int y1, int x2, int y2, int thickness, uint32_t color) {
    if (!surface || thickness <= 0) return;

    int prev_x = x0;
    int prev_y = y0;

    for (int i = 0; i <= 256; i += 16) {
        int t = i;
        int inv_t = 256 - t;

        int px = (inv_t * inv_t * x0 + 2 * inv_t * t * x1 + t * t * x2) >> 16;
        int py = (inv_t * inv_t * y0 + 2 * inv_t * t * y1 + t * t * y2) >> 16;

        agfx_draw_line(surface, prev_x, prev_y, px, py, thickness, color);
        
        prev_x = px;
        prev_y = py;
    }
    agfx_draw_line(surface, prev_x, prev_y, x2, y2, thickness, color);
}

void agfx_draw_rect(agfx_surface_t* surface, int x, int y, int w, int h, int thickness, uint32_t color) {
    if (thickness <= 0) return;
    if (thickness > w/2) thickness = w/2;
    if (thickness > h/2) thickness = h/2;

    agfx_fill_rect(surface, x, y, w, thickness, color);
    agfx_fill_rect(surface, x, y + h - thickness, w, thickness, color);
    agfx_fill_rect(surface, x, y + thickness, thickness, h - 2 * thickness, color);
    agfx_fill_rect(surface, x + w - thickness, y + thickness, thickness, h - 2 * thickness, color);
}

void agfx_draw_rect_rounded(agfx_surface_t* surface, int x, int y, int w, int h, int r, int thickness, uint32_t color) {
    if (!surface || thickness <= 0) return;
    int max_r = (w < h ? w : h) / 2;
    if (r > max_r) r = max_r;
    if (r <= 0) { agfx_draw_rect(surface, x, y, w, h, thickness, color); return; }
    if (thickness > r) thickness = r;

    agfx_fill_rect(surface, x + r, y, w - 2*r, thickness, color);
    agfx_fill_rect(surface, x + r, y + h - thickness, w - 2*r, thickness, color);
    agfx_fill_rect(surface, x, y + r, thickness, h - 2*r, color);
    agfx_fill_rect(surface, x + w - thickness, y + r, thickness, h - 2*r, color);

    int r_out_sq = r * r;
    int r_in = r - thickness;
    int r_in_sq = r_in * r_in;

    for (int cy = 0; cy < r; cy++) {
        for (int cx = 0; cx < r; cx++) {
            int dx = r - cx - 1;
            int dy = r - cy - 1;
            int dist_sq = dx*dx + dy*dy;

            if (dist_sq < r_out_sq && dist_sq >= r_in_sq) {
                agfx_draw_pixel(surface, x + cx, y + cy, color);
                agfx_draw_pixel(surface, x + w - r + dx, y + cy, color);
                agfx_draw_pixel(surface, x + cx, y + h - r + dy, color);
                agfx_draw_pixel(surface, x + w - r + dx, y + h - r + dy, color);
            }
        }
    }
}

void agfx_fill_polygon(agfx_surface_t* surface, const agfx_point_t* points, int count, uint32_t color) {
    if (!surface || !points || count < 3) return;

    for (int i = 1; i < count - 1; i++) {
        agfx_fill_triangle(surface, 
                           points[0].x, points[0].y, 
                           points[i].x, points[i].y, 
                           points[i+1].x, points[i+1].y, 
                           color);
    }
}