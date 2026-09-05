#ifndef AGFX_MASK_H
#define AGFX_MASK_H

#include "agfx.h"

typedef enum {
    AGFX_MASK_OP_SET = 0,  // dst = src
    AGFX_MASK_OP_MAX,      // dst = max(dst, src)  (удобно для "union")
    AGFX_MASK_OP_ADD,      // dst = min(255, dst + src)
    AGFX_MASK_OP_SUB       // dst = max(0, dst - src)
} agfx_mask_op_t;

void agfx_mask_clear(uint8_t* mask, int w, int h, uint8_t value);

void agfx_mask_fill_rect_op(uint8_t* mask, int mw, int mh,
                            int x, int y, int w, int h,
                            uint8_t alpha, agfx_mask_op_t op);

void agfx_mask_fill_circle_op(uint8_t* mask, int mw, int mh,
                              int cx, int cy, int r,
                              uint8_t alpha, agfx_mask_op_t op);

void agfx_mask_fill_rounded_rect_op(uint8_t* mask, int mw, int mh,
                                    int x, int y, int w, int h, int r,
                                    uint8_t alpha, agfx_mask_op_t op);

void agfx_mask_fill_triangle_op(uint8_t* mask, int mw, int mh,
                                int x0,int y0,int x1,int y1,int x2,int y2,
                                uint8_t alpha, agfx_mask_op_t op);

void agfx_mask_fill_polygon_fan_op(uint8_t* mask, int mw, int mh,
                                   const agfx_point_t* pts, int count,
                                   uint8_t alpha, agfx_mask_op_t op);
								   
void agfx_mask_fill_crescent_op(uint8_t* mask, int mw, int mh,
                                int cx, int cy, int r,
                                int cut_x, int cut_y, int cut_r,
                                uint8_t alpha, agfx_mask_op_t op);

static inline void agfx_mask_fill_rect(uint8_t* m,int mw,int mh,int x,int y,int w,int h,uint8_t a){
    agfx_mask_fill_rect_op(m,mw,mh,x,y,w,h,a,AGFX_MASK_OP_MAX);
}
static inline void agfx_mask_fill_circle(uint8_t* m,int mw,int mh,int cx,int cy,int r,uint8_t a){
    agfx_mask_fill_circle_op(m,mw,mh,cx,cy,r,a,AGFX_MASK_OP_MAX);
}
static inline void agfx_mask_fill_rounded_rect(uint8_t* m,int mw,int mh,int x,int y,int w,int h,int r,uint8_t a){
    agfx_mask_fill_rounded_rect_op(m,mw,mh,x,y,w,h,r,a,AGFX_MASK_OP_MAX);
}

static inline void agfx_mask_fill_crescent(uint8_t* m, int mw, int mh, int cx, int cy, int r, int cut_x, int cut_y, int cut_r, uint8_t a) {
    agfx_mask_fill_crescent_op(m, mw, mh, cx, cy, r, cut_x, cut_y, cut_r, a, AGFX_MASK_OP_MAX);
}

#endif // AGFX_MASK_H