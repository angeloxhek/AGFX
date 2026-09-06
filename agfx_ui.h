#ifndef AGFX_UI_H
#define AGFX_UI_H

#include "agfx.h"
#include "agfx_text.h"

#define AGFX_UI_STATE_NORMAL   0
#define AGFX_UI_STATE_HOVER    1
#define AGFX_UI_STATE_PRESSED  2

typedef struct {
    uint32_t bg;
    uint32_t control;
    uint32_t control_hover;
    uint32_t control_pressed;
    uint32_t border;
    uint32_t accent;
    uint32_t text;
    uint32_t text_secondary;
    uint32_t separator;
} agfx_ui_theme_t;

agfx_ui_theme_t agfx_ui_theme_win10_dark(void);
agfx_ui_theme_t agfx_ui_theme_win10_light(void);

typedef struct {
    agfx_surface_t* surface;
    const agfx_font_t* font;
    agfx_ui_theme_t theme;

    int mouse_x;
    int mouse_y;
    int mouse_down;
    int mouse_clicked;

    int cursor_x;
    int cursor_y;
    int start_x;
    int start_y;
    int spacing;

    int prev_x;
    int prev_y;
    int prev_w;
    int prev_h;
    int row_h;
} agfx_ui_context_t;

void agfx_ui_init(agfx_ui_context_t* ctx, agfx_surface_t* surface, const agfx_font_t* font);
void agfx_ui_set_theme(agfx_ui_context_t* ctx, const agfx_ui_theme_t* theme);
void agfx_ui_set_input(agfx_ui_context_t* ctx, int mx, int my, int m_down, int m_clicked);
void agfx_ui_begin(agfx_ui_context_t* ctx, int start_x, int start_y);
void agfx_ui_same_line(agfx_ui_context_t* ctx);

int  agfx_ui_button_behavior(agfx_ui_context_t* ctx, int w, int h, int* out_x, int* out_y, int* out_state);
int agfx_ui_slider_behavior(agfx_ui_context_t* ctx, int* val, int min, int max, int w, int h,
                            int* out_x, int* out_y, int* out_thumb_x);
							
int  agfx_ui_button(agfx_ui_context_t* ctx, const char* text, int w, int h);
int  agfx_ui_checkbox(agfx_ui_context_t* ctx, const char* text, int* checked);
int  agfx_ui_radio(agfx_ui_context_t* ctx, const char* text, int* selected_val, int my_val);
int  agfx_ui_slider(agfx_ui_context_t* ctx, int* val, int min, int max, int w, int h);
void agfx_ui_progress_bar(agfx_ui_context_t* ctx, float progress, int w, int h);
void agfx_ui_textbox(agfx_ui_context_t* ctx, const char* text, int w, int h, int is_focused);
void agfx_ui_label(agfx_ui_context_t* ctx, const char* text);
void agfx_ui_separator(agfx_ui_context_t* ctx, int w);

void agfx_ui_draw_shadow(agfx_surface_t* s, int x, int y, int w, int h, int size);

#endif // AGFX_UI_H