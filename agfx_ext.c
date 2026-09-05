#include "agfx_ext.h"

void agfx_fill_crescent(agfx_surface_t* surface, int xc, int yc, int r, 
                        int cut_x, int cut_y, int cut_r, uint32_t color) {
    if (!surface || !surface->buffer || r <= 0) return;

    uint8_t alpha = (color >> 24) & 0xFF;
    if (alpha == 0) return;

    int r_sq = r * r;
    int cut_r_sq = cut_r * cut_r;
    int pitch_pixels = surface->pitch / 4;

    int min_y = yc - r; if (min_y < surface->clip_y) min_y = surface->clip_y;
    int max_y = yc + r; if (max_y >= surface->clip_y + surface->clip_h) max_y = surface->clip_y + surface->clip_h - 1;
    int min_x = xc - r; if (min_x < surface->clip_x) min_x = surface->clip_x;
    int max_x = xc + r; if (max_x >= surface->clip_x + surface->clip_w) max_x = surface->clip_x + surface->clip_w - 1;

    for (int cy = min_y; cy <= max_y; cy++) {
        uint32_t* row = surface->buffer + (cy * pitch_pixels);
        int dy_circle = cy - yc;
        int dy_cut = cy - cut_y;

        for (int cx = min_x; cx <= max_x; cx++) {
            int dx_circle = cx - xc;
            int dx_cut = cx - cut_x;

            if (dx_circle * dx_circle + dy_circle * dy_circle <= r_sq) {
                if (dx_cut * dx_cut + dy_cut * dy_cut > cut_r_sq) {
                    
                    if (alpha == 255) {
                        row[cx] = color;
                    } else {
                        row[cx] = agfx_blend(row[cx], color);
                    }
                }
            }
        }
    }
}

void agfx_fill_rect_gradient(agfx_surface_t* surface,
                             int x, int y, int w, int h,
                             int gx0, int gy0, int gx1, int gy1,
                             uint32_t color1, uint32_t color2) {
    if (!surface || !surface->buffer) return;

    if (x < surface->clip_x) { w -= (surface->clip_x - x); x = surface->clip_x; }
    if (y < surface->clip_y) { h -= (surface->clip_y - y); y = surface->clip_y; }
    if (x + w > surface->clip_x + surface->clip_w) w = (surface->clip_x + surface->clip_w) - x;
    if (y + h > surface->clip_y + surface->clip_h) h = (surface->clip_y + surface->clip_h) - y;
    if (w <= 0 || h <= 0) return;

    agfx_linear_gradient_t g;
    agfx_gradient_init_lin(&g, gx0, gy0, gx1, gy1, color1, color2);

    if (g.len_sq == 0) {
        agfx_fill_rect(surface, x, y, w, h, color1);
        return;
    }

    int pitch_pixels = surface->pitch / 4;

    for (int cy = y; cy < y + h; cy++) {
        uint32_t* row = surface->buffer + (cy * pitch_pixels);

        int64_t py = (int64_t)cy - g.gy0;
        int64_t dot = ((int64_t)x - g.gx0) * g.dx + py * g.dy;

        for (int cx = x; cx < x + w; cx++) {
            int percent = (int)((dot * 255) / g.len_sq);

            uint32_t col = agfx_gradient_color_from_percent(&g, percent);

            if (g.opaque) {
                row[cx] = col;
            } else {
                uint8_t a = (col >> 24) & 0xFF;
                if (a == 255) row[cx] = col;
                else if (a > 0) row[cx] = agfx_blend(row[cx], col);
            }

            dot += g.dx;
        }
    }
}

void agfx_draw_rect_gradient(agfx_surface_t* surface,
                             int x, int y, int w, int h, int thickness,
                             int gx0, int gy0, int gx1, int gy1,
                             uint32_t color1, uint32_t color2)
{
    if (!surface || !surface->buffer) return;
    if (w <= 0 || h <= 0 || thickness <= 0) return;

    if (thickness * 2 > w) thickness = w / 2;
    if (thickness * 2 > h) thickness = h / 2;
    if (thickness <= 0) return;

    agfx_fill_rect_gradient(surface, x, y, w, thickness,
                            gx0, gy0, gx1, gy1, color1, color2);

    agfx_fill_rect_gradient(surface, x, y + h - thickness, w, thickness,
                            gx0, gy0, gx1, gy1, color1, color2);

    int inner_h = h - 2 * thickness;
    if (inner_h <= 0) return;

    agfx_fill_rect_gradient(surface, x, y + thickness, thickness, inner_h,
                            gx0, gy0, gx1, gy1, color1, color2);

    agfx_fill_rect_gradient(surface, x + w - thickness, y + thickness, thickness, inner_h,
                            gx0, gy0, gx1, gy1, color1, color2);
}

void agfx_draw_line_gradient(agfx_surface_t* surface,
                             int x0, int y0, int x1, int y1, int thickness,
                             int gx0, int gy0, int gx1, int gy1,
                             uint32_t c1, uint32_t c2)
{
    if (!surface || !surface->buffer || thickness <= 0) return;
	
	agfx_linear_gradient_t gr;
	agfx_gradient_init_lin(&gr, gx0, gy0, gx1, gy1, c1, c2);

    const int pad = (thickness / 2) + 2;
    int min_x = (x0 < x1 ? x0 : x1) - pad;
    int max_x = (x0 > x1 ? x0 : x1) + pad;
    int min_y = (y0 < y1 ? y0 : y1) - pad;
    int max_y = (y0 > y1 ? y0 : y1) + pad;

    if (min_x < surface->clip_x) min_x = surface->clip_x;
    if (max_x >= surface->clip_x + surface->clip_w) max_x = surface->clip_x + surface->clip_w - 1;
    if (min_y < surface->clip_y) min_y = surface->clip_y;
    if (max_y >= surface->clip_y + surface->clip_h) max_y = surface->clip_y + surface->clip_h - 1;

    int64_t abx = (int64_t)x1 - x0;
    int64_t aby = (int64_t)y1 - y0;
    int64_t l2  = abx*abx + aby*aby;

    const int pitch_pixels = surface->pitch / 4;

    // радиус в “субпикселях” (x256)
    const int64_t radius_256 = ((int64_t)thickness * 256) / 2;
    // зона сглаживания: +/-0.5 пикселя
    const int64_t aa_256 = 128;

    // длина отрезка |AB| (в пикселях), нужна для расстояния до прямой
    // (считаем один раз!)
    int64_t ab_len = (l2 == 0) ? 0 : (int64_t)agfx_isqrt((uint64_t)l2);

    for (int py = min_y; py <= max_y; py++) {
        uint32_t* row = surface->buffer + (py * pitch_pixels);

        for (int px = min_x; px <= max_x; px++) {
            int64_t dist_256;

            if (l2 == 0) {
                // отрезок = точка
                int64_t dx = (int64_t)px - x0;
                int64_t dy = (int64_t)py - y0;
                dist_256 = (int64_t)agfx_isqrt((uint64_t)(dx*dx + dy*dy)) * 256;
            } else {
                // t = dot(AP, AB) (диапазон 0..l2)
                int64_t apx = (int64_t)px - x0;
                int64_t apy = (int64_t)py - y0;
                int64_t t = apx*abx + apy*aby;

                if (t <= 0) {
                    int64_t dx = apx;
                    int64_t dy = apy;
                    dist_256 = (int64_t)agfx_isqrt((uint64_t)(dx*dx + dy*dy)) * 256;
                } else if (t >= l2) {
                    int64_t dx = (int64_t)px - x1;
                    int64_t dy = (int64_t)py - y1;
                    dist_256 = (int64_t)agfx_isqrt((uint64_t)(dx*dx + dy*dy)) * 256;
                } else {
                    // расстояние до прямой через cross / |AB|
                    // cross = |AB x AP|
                    int64_t cross = abx*apy - aby*apx;  // int64-safe
                    cross = AGFX_ABS(cross);

                    // dist (px) = cross / |AB| ; в x256: cross*256/|AB|
                    dist_256 = (ab_len != 0) ? ((cross * 256) / ab_len) : 0;
                }
            }

            // coverage_256: 0..256 (насколько пиксель “покрыт” линией)
            // внутри линии: dist <= radius -> coverage=256
            // в зоне сглаживания: плавно падает к 0
            int64_t coverage_256 = 0;
            if (dist_256 <= radius_256 - aa_256) {
                coverage_256 = 256;
            } else if (dist_256 <= radius_256 + aa_256) {
                // линейный спад на 1 пиксель ширины
                coverage_256 = (radius_256 + aa_256) - dist_256; // 0..256
            } else {
                continue;
            }

            uint32_t col = agfx_gradient_sample_lin(&gr, px, py);

            // домножаем альфу градиента на coverage
            uint8_t a = (col >> 24) & 0xFF;
            if (a == 0) continue;

            a = (uint8_t)((a * coverage_256) / 256);
            col = (col & 0x00FFFFFF) | ((uint32_t)a << 24);

            row[px] = agfx_blend(row[px], col);
        }
    }
}