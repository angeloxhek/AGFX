#include "agfx_ui.h"

agfx_ui_theme_t agfx_ui_theme_win10_dark(void) {
    agfx_ui_theme_t t;
    t.bg              = 0xFF202020;
    t.control         = 0xFF333333;
    t.control_hover   = 0xFF444444;
    t.control_pressed = 0xFF111111;
    t.border          = 0xFF555555;
    t.accent          = 0xFF0078D7;
    t.text            = 0xFFFFFFFF;
    t.text_secondary  = 0xFFAAAAAA;
    t.separator       = 0xFF383838;
    return t;
}

agfx_ui_theme_t agfx_ui_theme_win10_light(void) {
    agfx_ui_theme_t t;
    t.bg              = 0xFFF0F0F0;
    t.control         = 0xFFE1E1E1;
    t.control_hover   = 0xFFE5F1FB;
    t.control_pressed = 0xFFCCE4F7;
    t.border          = 0xFFADADAD;
    t.accent          = 0xFF0078D7;
    t.text            = 0xFF000000;
    t.text_secondary  = 0xFF666666;
    t.separator       = 0xFFD0D0D0;
    return t;
}

void agfx_ui_init(agfx_ui_context_t* ctx, agfx_surface_t* surface, const agfx_font_t* font) {
    if (!ctx) return;
    ctx->surface = surface;
    ctx->font = font;
    ctx->theme = agfx_ui_theme_win10_dark();
    ctx->mouse_x = 0;
    ctx->mouse_y = 0;
    ctx->mouse_down = 0;
    ctx->mouse_clicked = 0;
    ctx->spacing = 8;
    ctx->cursor_x = 0;
    ctx->cursor_y = 0;
    ctx->start_x = 0;
    ctx->start_y = 0;
    ctx->prev_x = 0;
    ctx->prev_y = 0;
    ctx->prev_w = 0;
    ctx->prev_h = 0;
    ctx->row_h = 0;
}

void agfx_ui_set_theme(agfx_ui_context_t* ctx, const agfx_ui_theme_t* theme) {
    if (ctx && theme) ctx->theme = *theme;
}

void agfx_ui_set_input(agfx_ui_context_t* ctx, int mx, int my, int m_down, int m_clicked) {
    if (!ctx) return;
    ctx->mouse_x = mx;
    ctx->mouse_y = my;
    ctx->mouse_down = m_down;
    ctx->mouse_clicked = m_clicked;
}

void agfx_ui_begin(agfx_ui_context_t* ctx, int start_x, int start_y) {
    if (!ctx) return;
    ctx->start_x = start_x;
    ctx->start_y = start_y;
    ctx->cursor_x = start_x;
    ctx->cursor_y = start_y;
    ctx->row_h = 0;
    ctx->prev_w = 0;
    ctx->prev_h = 0;
}

static void agfx_ui_advance(agfx_ui_context_t* ctx, int w, int h) {
    ctx->prev_x = ctx->cursor_x;
    ctx->prev_y = ctx->cursor_y;
    ctx->prev_w = w;
    ctx->prev_h = h;

    if (h > ctx->row_h) ctx->row_h = h;

    ctx->cursor_x = ctx->start_x;
    ctx->cursor_y += ctx->row_h + ctx->spacing;
    ctx->row_h = 0;
}

void agfx_ui_same_line(agfx_ui_context_t* ctx) {
    if (!ctx) return;
    ctx->cursor_y = ctx->prev_y;
    ctx->cursor_x = ctx->prev_x + ctx->prev_w + ctx->spacing;
    ctx->row_h = ctx->prev_h;
}

int agfx_ui_button(agfx_ui_context_t* ctx, const char* text, int w, int h) {
    if (!ctx || !ctx->surface) return 0;

    int x, y, state;
    int clicked = agfx_ui_button_behavior(ctx, w, h, &x, &y, &state);

    uint32_t bg = (state == AGFX_UI_STATE_PRESSED) ? ctx->theme.control_pressed :
                  (state == AGFX_UI_STATE_HOVER)   ? ctx->theme.control_hover : ctx->theme.control;
    uint32_t border = (state == AGFX_UI_STATE_PRESSED) ? ctx->theme.accent : ctx->theme.border;

    agfx_fill_rect(ctx->surface, x, y, w, h, bg);
    agfx_draw_rect(ctx->surface, x, y, w, h, 1, border);

    if (ctx->font && text) {
        int tw, th;
        uint8_t* mask = agfx_mask_generate_string(ctx->font, text, &tw, &th);
        if (mask) {
            int tx = x + (w - tw) / 2;
            int ty = y + (h - th) / 2;
            agfx_fill_alpha_mask(ctx->surface, tx, ty, tw, th, mask, ctx->theme.text);
            agfx_mask_free(mask);
        }
    }
	
    return clicked;
}

int agfx_ui_checkbox(agfx_ui_context_t* ctx, const char* text, int* checked) {
    if (!ctx || !ctx->surface || !checked) return 0;

    int x = ctx->cursor_x, y = ctx->cursor_y;
    int box_size = 18;

    int is_hover = (ctx->mouse_x >= x && ctx->mouse_x < x + box_size + 200 &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + box_size);

    if (is_hover && ctx->mouse_clicked) *checked = !(*checked);

    uint32_t bg = (*checked) ? ctx->theme.accent : 
                  (is_hover ? ctx->theme.control_hover : ctx->theme.control);
    agfx_fill_rect(ctx->surface, x, y, box_size, box_size, bg);
    agfx_draw_rect(ctx->surface, x, y, box_size, box_size, 1, ctx->theme.border);

    if (*checked) {
        agfx_draw_line(ctx->surface, x + 4, y + 9, x + 8, y + 13, 2, 0xFFFFFFFF);
        agfx_draw_line(ctx->surface, x + 8, y + 13, x + 14, y + 5, 2, 0xFFFFFFFF);
    }

    int text_w = 0;
    if (ctx->font && text) {
        int th;
        uint8_t* mask = agfx_mask_generate_string(ctx->font, text, &text_w, &th);
        if (mask) {
            agfx_fill_alpha_mask(ctx->surface, x + box_size + 8, y + (box_size - th) / 2, text_w, th, mask, ctx->theme.text);
            agfx_mask_free(mask);
        }
    }

    agfx_ui_advance(ctx, box_size + 8 + text_w, box_size);
    return is_hover && ctx->mouse_clicked;
}

int agfx_ui_radio(agfx_ui_context_t* ctx, const char* text, int* selected_val, int my_val) {
    if (!ctx || !ctx->surface || !selected_val) return 0;

    int x = ctx->cursor_x, y = ctx->cursor_y;
    int r = 8;
    int diameter = r * 2;
    int is_selected = (*selected_val == my_val);

    int is_hover = (ctx->mouse_x >= x && ctx->mouse_x < x + diameter + 200 &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + diameter);

    if (is_hover && ctx->mouse_clicked) *selected_val = my_val;

    uint32_t bg = is_hover ? ctx->theme.control_hover : ctx->theme.control;
    agfx_fill_circle(ctx->surface, x + r, y + r, r, bg);
    agfx_draw_circle(ctx->surface, x + r, y + r, r, is_selected ? ctx->theme.accent : ctx->theme.border);

    if (is_selected) {
        agfx_fill_circle(ctx->surface, x + r, y + r, 4, ctx->theme.accent);
    }

    int text_w = 0;
    if (ctx->font && text) {
        int th;
        uint8_t* mask = agfx_mask_generate_string(ctx->font, text, &text_w, &th);
        if (mask) {
            agfx_fill_alpha_mask(ctx->surface, x + diameter + 8, y + (diameter - th) / 2, text_w, th, mask, ctx->theme.text);
            agfx_mask_free(mask);
        }
    }

    agfx_ui_advance(ctx, diameter + 8 + text_w, diameter);
    return is_hover && ctx->mouse_clicked;
}

int agfx_ui_slider(agfx_ui_context_t* ctx, int* val, int min, int max, int w, int h) {
    if (!ctx || !ctx->surface || !val || max <= min) return 0;

    int x = ctx->cursor_x, y = ctx->cursor_y;
    int is_hover = (ctx->mouse_x >= x && ctx->mouse_x < x + w &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + h);

    if ((is_hover && ctx->mouse_down) || (ctx->mouse_down && is_hover)) {
        int rel_x = ctx->mouse_x - x;
        if (rel_x < 0) rel_x = 0;
        if (rel_x > w) rel_x = w;
        *val = min + (rel_x * (max - min)) / w;
    }

    int track_h = 4;
    int track_y = y + (h - track_h) / 2;
    agfx_fill_rect(ctx->surface, x, track_y, w, track_h, ctx->theme.control_hover);

    int thumb_x = x + ((*val - min) * w) / (max - min);
    agfx_fill_rect(ctx->surface, x, track_y, thumb_x - x, track_h, ctx->theme.accent);

    int thumb_w = 10;
    int thumb_draw_x = thumb_x - thumb_w / 2;
    agfx_fill_rect(ctx->surface, thumb_draw_x, y, thumb_w, h, 
                   is_hover ? ctx->theme.text : ctx->theme.accent);

    agfx_ui_advance(ctx, w, h);
    return is_hover && ctx->mouse_down;
}

void agfx_ui_progress_bar(agfx_ui_context_t* ctx, float progress, int w, int h) {
    if (!ctx || !ctx->surface) return;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    int x = ctx->cursor_x, y = ctx->cursor_y;
    agfx_fill_rect(ctx->surface, x, y, w, h, ctx->theme.control_pressed);
    agfx_draw_rect(ctx->surface, x, y, w, h, 1, ctx->theme.border);

    int fill_w = (int)((w - 2) * progress);
    if (fill_w > 0) {
        agfx_fill_rect(ctx->surface, x + 1, y + 1, fill_w, h - 2, ctx->theme.accent);
    }

    agfx_ui_advance(ctx, w, h);
}

void agfx_ui_textbox(agfx_ui_context_t* ctx, const char* text, int w, int h, int is_focused) {
    if (!ctx || !ctx->surface) return;

    int x = ctx->cursor_x, y = ctx->cursor_y;
    uint32_t bg = is_focused ? ctx->theme.control_pressed : ctx->theme.control;
    uint32_t border = is_focused ? ctx->theme.accent : ctx->theme.border;

    agfx_fill_rect(ctx->surface, x, y, w, h, bg);
    agfx_draw_rect(ctx->surface, x, y, w, h, 1, border);

    int text_w = 0;
    if (ctx->font && text) {
        int th;
        uint8_t* mask = agfx_mask_generate_string(ctx->font, text, &text_w, &th);
        if (mask) {
            agfx_fill_alpha_mask(ctx->surface, x + 8, y + (h - th) / 2, text_w, th, mask, ctx->theme.text);
            agfx_mask_free(mask);
        }
    }

    if (is_focused) {
        int cur_x = x + 8 + text_w + 2;
        agfx_draw_line(ctx->surface, cur_x, y + 5, cur_x, y + h - 5, 1, ctx->theme.text);
    }

    agfx_ui_advance(ctx, w, h);
}

void agfx_ui_label(agfx_ui_context_t* ctx, const char* text) {
    if (!ctx || !ctx->surface || !text) return;

    int tw = 0, th = 16;
    if (ctx->font) {
        uint8_t* mask = agfx_mask_generate_string(ctx->font, text, &tw, &th);
        if (mask) {
            agfx_fill_alpha_mask(ctx->surface, ctx->cursor_x, ctx->cursor_y, tw, th, mask, ctx->theme.text);
            agfx_mask_free(mask);
        }
    }
    agfx_ui_advance(ctx, tw, th);
}

void agfx_ui_separator(agfx_ui_context_t* ctx, int w) {
    if (!ctx || !ctx->surface) return;
    agfx_draw_line(ctx->surface, ctx->cursor_x, ctx->cursor_y, ctx->cursor_x + w, ctx->cursor_y, 1, ctx->theme.separator);
    agfx_ui_advance(ctx, w, 6);
}

void agfx_ui_draw_shadow(agfx_surface_t* s, int x, int y, int w, int h, int size) {
    if (!s || size <= 0) return;
    for (int i = 1; i <= size; i++) {
        uint8_t alpha = (uint8_t)(((size + 1 - i) * 45) / size);
        agfx_draw_rect(s, x - i, y - i + (size / 3), w + i * 2, h + i * 2, 1, ((uint32_t)alpha << 24));
    }
}

int agfx_ui_button_behavior(agfx_ui_context_t* ctx, int w, int h, int* out_x, int* out_y, int* out_state) {
    if (!ctx) return 0;

    int x = ctx->cursor_x;
    int y = ctx->cursor_y;

    int is_hover = (ctx->mouse_x >= x && ctx->mouse_x < x + w &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + h);
    int is_pressed = is_hover && ctx->mouse_down;
    int is_clicked = is_hover && ctx->mouse_clicked;

    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    if (out_state) {
        *out_state = is_pressed ? AGFX_UI_STATE_PRESSED :
                    (is_hover   ? AGFX_UI_STATE_HOVER   : AGFX_UI_STATE_NORMAL);
    }

    agfx_ui_advance(ctx, w, h);
    return is_clicked;
}

int agfx_ui_slider_behavior(agfx_ui_context_t* ctx, int* val, int min, int max, int w, int h,
                            int* out_x, int* out_y, int* out_thumb_x) 
{
    if (!ctx || !val || max <= min) return 0;

    int x = ctx->cursor_x, y = ctx->cursor_y;
    int is_hover = (ctx->mouse_x >= x && ctx->mouse_x < x + w &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + h);

    int is_active = 0;
    if (is_hover && ctx->mouse_down) {
        is_active = 1;
        int rel_x = ctx->mouse_x - x;
        if (rel_x < 0) rel_x = 0;
        if (rel_x > w) rel_x = w;
        *val = min + (rel_x * (max - min)) / w;
    }

    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    if (out_thumb_x) *out_thumb_x = x + ((*val - min) * w) / (max - min);

    agfx_ui_advance(ctx, w, h);
    return is_active;
}