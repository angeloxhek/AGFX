#include "agfx_mask.h"

static inline uint8_t u8_mul_div255(uint8_t a, uint8_t b) {
    return (uint8_t)((a * (uint16_t)b + 127) / 255);
}

static inline uint8_t mask_apply(uint8_t dst, uint8_t src, agfx_mask_op_t op) {
    switch (op) {
        case AGFX_MASK_OP_SET: return src;
        case AGFX_MASK_OP_MAX: return (dst > src) ? dst : src;
        case AGFX_MASK_OP_ADD: {
            int v = (int)dst + (int)src;
            return (v > 255) ? 255 : (uint8_t)v;
        }
        case AGFX_MASK_OP_SUB: {
            int v = (int)dst - (int)src;
            return (v < 0) ? 0 : (uint8_t)v;
        }
        default: return src;
    }
}

void agfx_mask_clear(uint8_t* mask, int w, int h, uint8_t value) {
    if (!mask || w <= 0 || h <= 0) return;
    size_t n = (size_t)w * (size_t)h;
    for (size_t i = 0; i < n; i++) mask[i] = value;
}

void agfx_mask_fill_rect_op(uint8_t* mask, int mw, int mh,
                            int x, int y, int w, int h,
                            uint8_t alpha, agfx_mask_op_t op)
{
    if (!mask || mw <= 0 || mh <= 0 || w <= 0 || h <= 0 || alpha == 0) return;

    int x0 = AGFX_MAX(x, 0);
    int y0 = AGFX_MAX(y, 0);
    int x1 = AGFX_MIN(x + w, mw);
    int y1 = AGFX_MIN(y + h, mh);
    if (x0 >= x1 || y0 >= y1) return;

    for (int yy = y0; yy < y1; yy++) {
        uint8_t* row = mask + (size_t)yy * (size_t)mw;
        for (int xx = x0; xx < x1; xx++) {
            row[xx] = mask_apply(row[xx], alpha, op);
        }
    }
}

static inline uint8_t circle_aa_cov(int dx, int dy, int r) {
    if (r <= 0) return 0;
    if (r == 1) {
        int d2 = dx*dx + dy*dy;
        return (d2 <= 0) ? 255 : 0;
    }

    int r0 = r - 1;
    int d2 = dx*dx + dy*dy;
    int r0_2 = r0 * r0;
    int r1_2 = r * r;

    if (d2 <= r0_2) return 255;
    if (d2 >= r1_2) return 0;

    int denom = (r1_2 - r0_2);
    int num = (r1_2 - d2);
    int a = (num * 255) / denom;
    if (a < 0) a = 0;
    if (a > 255) a = 255;
    return (uint8_t)a;
}

void agfx_mask_fill_circle_op(uint8_t* mask, int mw, int mh,
                              int cx, int cy, int r,
                              uint8_t alpha, agfx_mask_op_t op)
{
    if (!mask || mw <= 0 || mh <= 0 || r <= 0 || alpha == 0) return;

    int x0 = AGFX_MAX(cx - r, 0);
    int y0 = AGFX_MAX(cy - r, 0);
    int x1 = AGFX_MIN(cx + r + 1, mw);
    int y1 = AGFX_MIN(cy + r + 1, mh);
    if (x0 >= x1 || y0 >= y1) return;

    for (int yy = y0; yy < y1; yy++) {
        uint8_t* row = mask + (size_t)yy * (size_t)mw;
        int dy = yy - cy;
        for (int xx = x0; xx < x1; xx++) {
            int dx = xx - cx;
            uint8_t cov = circle_aa_cov(dx, dy, r);
            if (!cov) continue;
            uint8_t v = u8_mul_div255(alpha, cov);
            if (!v) continue;
            row[xx] = mask_apply(row[xx], v, op);
        }
    }
}

void agfx_mask_fill_rounded_rect_op(uint8_t* mask, int mw, int mh,
                                    int x, int y, int w, int h, int r,
                                    uint8_t alpha, agfx_mask_op_t op)
{
    if (!mask || mw <= 0 || mh <= 0 || w <= 0 || h <= 0 || alpha == 0) return;

    int max_r = AGFX_MIN(w, h) / 2;
    if (r > max_r) r = max_r;
    if (r <= 0) {
        agfx_mask_fill_rect_op(mask, mw, mh, x, y, w, h, alpha, op);
        return;
    }

    agfx_mask_fill_rect_op(mask, mw, mh, x + r, y, w - 2*r, h, alpha, op);
    agfx_mask_fill_rect_op(mask, mw, mh, x, y + r, r, h - 2*r, alpha, op);
    agfx_mask_fill_rect_op(mask, mw, mh, x + w - r, y + r, r, h - 2*r, alpha, op);

    int c_tl_x = x + r - 1,     c_tl_y = y + r - 1;
    int c_tr_x = x + w - r,     c_tr_y = y + r - 1;
    int c_bl_x = x + r - 1,     c_bl_y = y + h - r;
    int c_br_x = x + w - r,     c_br_y = y + h - r;

    int x0 = AGFX_MAX(x, 0);
    int y0 = AGFX_MAX(y, 0);
    int x1 = AGFX_MIN(x + w, mw);
    int y1 = AGFX_MIN(y + h, mh);

    for (int yy = y0; yy < y1; yy++) {
        uint8_t* row = mask + (size_t)yy * (size_t)mw;
        for (int xx = x0; xx < x1; xx++) {

            if (xx >= x + r && xx < x + w - r) continue;
            if (yy >= y + r && yy < y + h - r) continue;

            int dx, dy;
            uint8_t cov = 0;

            if (xx < x + r && yy < y + r) {
                dx = xx - c_tl_x;
                dy = yy - c_tl_y;
                cov = circle_aa_cov(dx, dy, r);
            }
            else if (xx >= x + w - r && yy < y + r) {
                dx = xx - c_tr_x;
                dy = yy - c_tr_y;
                cov = circle_aa_cov(dx, dy, r);
            }
            else if (xx < x + r && yy >= y + h - r) {
                dx = xx - c_bl_x;
                dy = yy - c_bl_y;
                cov = circle_aa_cov(dx, dy, r);
            }
            else if (xx >= x + w - r && yy >= y + h - r) {
                dx = xx - c_br_x;
                dy = yy - c_br_y;
                cov = circle_aa_cov(dx, dy, r);
            }

            if (!cov) continue;

            uint8_t v = u8_mul_div255(alpha, cov);
            if (!v) continue;
            row[xx] = mask_apply(row[xx], v, op);
        }
    }
}

void agfx_mask_fill_triangle_op(uint8_t* mask, int mw, int mh,
                                int x0,int y0,int x1,int y1,int x2,int y2,
                                uint8_t alpha, agfx_mask_op_t op)
{
    if (!mask || mw <= 0 || mh <= 0 || alpha == 0) return;

    if (y0 > y1) { AGFX_SWAP(y0,y1,int); AGFX_SWAP(x0,x1,int); }
    if (y0 > y2) { AGFX_SWAP(y0,y2,int); AGFX_SWAP(x0,x2,int); }
    if (y1 > y2) { AGFX_SWAP(y1,y2,int); AGFX_SWAP(x1,x2,int); }

    int total_h = y2 - y0;
    if (total_h == 0) return;

    for (int i = 0; i <= total_h; i++) {
        int yy = y0 + i;
        if (yy < 0 || yy >= mh) continue;

        int second = (i > (y1 - y0)) || (y1 == y0);
        int seg_h = second ? (y2 - y1) : (y1 - y0);
        if (seg_h == 0) continue;

        int x_long = x0 + (int)((int64_t)(x2 - x0) * i / total_h);
        int x_short;

        if (!second) {
            x_short = x0 + (int)((int64_t)(x1 - x0) * i / seg_h);
        } else {
            x_short = x1 + (int)((int64_t)(x2 - x1) * (i - (y1 - y0)) / seg_h);
        }

        if (x_long > x_short) AGFX_SWAP(x_long, x_short, int);

        int xs = AGFX_MAX(x_long, 0);
        int xe = AGFX_MIN(x_short + 1, mw);
        if (xs >= xe) continue;

        uint8_t* row = mask + (size_t)yy * (size_t)mw;
        for (int xx = xs; xx < xe; xx++) {
            row[xx] = mask_apply(row[xx], alpha, op);
        }
    }
}

void agfx_mask_fill_polygon_fan_op(uint8_t* mask, int mw, int mh,
                                   const agfx_point_t* pts, int count,
                                   uint8_t alpha, agfx_mask_op_t op)
{
    if (!mask || !pts || count < 3 || alpha == 0) return;

    for (int i = 1; i < count - 1; i++) {
        agfx_mask_fill_triangle_op(mask, mw, mh,
                                  pts[0].x, pts[0].y,
                                  pts[i].x, pts[i].y,
                                  pts[i+1].x, pts[i+1].y,
                                  alpha, op);
    }
}

void agfx_mask_fill_crescent_op(uint8_t* mask, int mw, int mh,
                                int cx, int cy, int r,
                                int cut_x, int cut_y, int cut_r,
                                uint8_t alpha, agfx_mask_op_t op)
{
    if (!mask || mw <= 0 || mh <= 0 || r <= 0 || alpha == 0) return;

    int x0 = AGFX_MAX(cx - r, 0);
    int y0 = AGFX_MAX(cy - r, 0);
    int x1 = AGFX_MIN(cx + r + 1, mw);
    int y1 = AGFX_MIN(cy + r + 1, mh);
    if (x0 >= x1 || y0 >= y1) return;

    for (int yy = y0; yy < y1; yy++) {
        uint8_t* row = mask + (size_t)yy * (size_t)mw;
        int dy = yy - cy;
        int cut_dy = yy - cut_y;
        
        for (int xx = x0; xx < x1; xx++) {
            uint8_t cov = circle_aa_cov(xx - cx, dy, r);
            if (!cov) continue;
            
            uint8_t cut_cov = circle_aa_cov(xx - cut_x, cut_dy, cut_r);
            
            if (cut_cov == 255) continue;
            
            if (cut_cov > 0) {
                cov = u8_mul_div255(cov, 255 - cut_cov);
            }
            
            uint8_t v = u8_mul_div255(alpha, cov);
            if (!v) continue;
            
            row[xx] = mask_apply(row[xx], v, op);
        }
    }
}