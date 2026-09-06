#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../agfx.h"
#include "../agfx_filters.h"
#include "../agfx_mask.h"
#include "../agfx_text.h"
#include "../agfx_ui.h"

#define WIDTH 800
#define HEIGHT 600

void save_bmp(const char* filename, uint32_t* buffer, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) { printf("Error opening file!\n"); return; }

    uint8_t file_header[14] = { 'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0 };
    int filesize = 54 + (width * height * 4);
    file_header[2] = (uint8_t)(filesize);
    file_header[3] = (uint8_t)(filesize >> 8);
    file_header[4] = (uint8_t)(filesize >> 16);
    file_header[5] = (uint8_t)(filesize >> 24);

    uint8_t info_header[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 32,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
    info_header[4] = (uint8_t)(width);
    info_header[5] = (uint8_t)(width >> 8);
    info_header[6] = (uint8_t)(width >> 16);
    info_header[7] = (uint8_t)(width >> 24);
    
    int h = -height;
    info_header[8] = (uint8_t)(h);
    info_header[9] = (uint8_t)(h >> 8);
    info_header[10] = (uint8_t)(h >> 16);
    info_header[11] = (uint8_t)(h >> 24);

    fwrite(file_header, 1, 14, f);
    fwrite(info_header, 1, 40, f);
    fwrite(buffer, 4, width * height, f);
    fclose(f);
}

uint8_t* load_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t* buf = (uint8_t*)malloc(size);
    if (buf) fread(buf, 1, size, f);
    fclose(f);
    return buf;
}

int main() {
    printf("Starting AGFX Showcase...\n1. Basic\n");

    int pitch = WIDTH * 4;
    uint32_t* framebuffer = (uint32_t*)calloc(WIDTH * HEIGHT, 4);

    agfx_surface_t screen;
    agfx_init(&screen, framebuffer, WIDTH, HEIGHT, pitch);
	
	agfx_set_allocators(malloc, free);

    agfx_fill_rect(&screen, 0, 0, WIDTH, HEIGHT, 0xFF1E1E2E);

    for (int x = 0; x < WIDTH; x += 50) {
        agfx_draw_line(&screen, x, 0, x, HEIGHT, 1, 0xFF2A2A3B);
    }
    for (int y = 0; y < HEIGHT; y += 50) {
        agfx_draw_line(&screen, 0, y, WIDTH, y, 1, 0xFF2A2A3B);
    }

    agfx_filter_gradient(&screen, 40, 40, 160, 160, 
                         40, 40, 200, 200, 
                         0xFF8A2387, 0xFFE94057);

    uint8_t* mask_rect = (uint8_t*)calloc(200 * 120, 1);
    agfx_mask_fill_rounded_rect(mask_rect, 200, 120, 0, 0, 200, 120, 25, 255);
    agfx_filter_gradient_mask(&screen, 120, 100, 200, 120, mask_rect,
                              120, 100, 320, 220,
                              0xFFF2A65A, 0xFF7700FF);
    free(mask_rect);

    uint8_t* mask_circle = (uint8_t*)calloc(160 * 160, 1);
    agfx_mask_fill_circle(mask_circle, 160, 160, 80, 80, 80, 255);
    agfx_filter_gradient_mask(&screen, 470, 50, 160, 160, mask_circle,
                              470, 50, 470, 210,
                              0xFF11998E, 0xFF38EF7D);
    free(mask_circle);

    uint8_t* mask_poly = (uint8_t*)calloc(200 * 200, 1);
    agfx_point_t hex[6] = { {80, 0}, {160, 40}, {160, 130}, {80, 170}, {0, 130}, {0, 40} };
    agfx_mask_fill_polygon_fan_op(mask_poly, 200, 200, hex, 6, 255, AGFX_MASK_OP_SET);
    agfx_filter_gradient_mask(&screen, 70, 350, 200, 200, mask_poly,
                              70, 350, 270, 550,
                              0xFF00C9FF, 0xFF92FE9D);
    free(mask_poly);

    agfx_fill_triangle(&screen, 150, 420, 320, 550, 70, 580, 0x99FF92A5);
    agfx_draw_circle(&screen, 550, 130, 90, 0x88FFFFFF);
    agfx_draw_bezier(&screen, 450, 450, 600, 280, 750, 450, 5, 0xFF00FF87);

    save_bmp("output_basic.bmp", framebuffer, WIDTH, HEIGHT);
    printf("Saved to output_basic.bmp\n2. Filters\n");
	
    int fx = 90, fy = 70, fw = 300, fh = 200;
    size_t scratch_n = agfx_filters_scratch_u32(fw, fh);

    uint32_t* tmp1 = (uint32_t*)malloc(scratch_n * sizeof(uint32_t));
    uint32_t* tmp2 = (uint32_t*)malloc(scratch_n * sizeof(uint32_t));

    if (tmp1 && tmp2) {
        agfx_filter_glass(&screen, fx, fy, fw, fh, 8, 0x55FFFFFF, tmp1, tmp2);

        agfx_draw_rect_rounded(&screen, fx, fy, fw, fh, 22, 2, 0x88FFFFFF);

        agfx_filter_grayscale(&screen, 40, 340, 220, 220);
        agfx_draw_rect(&screen, 40, 340, 220, 220, 2, 0xAAFFFFFF);

        agfx_filter_invert(&screen, 270, 340, 120, 120);
        agfx_draw_rect(&screen, 270, 340, 120, 120, 2, 0xAAFFFFFF);
		
		agfx_filter_acrylic(&screen, 400, 220, 200, 300,
                            5,
                            0x66222222,
                            16,
                            0xC0FFEEu,
                            tmp1, tmp2);
		
		agfx_draw_rect(&screen, 400, 220, 200, 300, 1, 0x88FFFFFF);
    }

    free(tmp1);
    free(tmp2);

    save_bmp("output_filters.bmp", framebuffer, WIDTH, HEIGHT);
    printf("Saved to output_filters.bmp\n3. Text Rendering\n");
    
    agfx_fill_rect(&screen, 0, 0, WIDTH, HEIGHT, 0xFF1E1E2E);

    uint8_t* font_buffer = load_file("selawk.ttf");
    if (font_buffer) {
        agfx_font_t font;
        if (agfx_font_init(&font, font_buffer, 72.0f)) {
            
            int tw, th;
            uint8_t* text_mask = agfx_mask_generate_string(&font, "AGFX Typography", &tw, &th);
            
            if (text_mask) {
                agfx_fill_alpha_mask(&screen, 50, 50, tw, th, text_mask, 0xFF00FFCC);

                agfx_filter_gradient_mask(&screen, 50, 150, tw, th, text_mask,
                                          50, 150, 50 + tw, 150,
                                          0xFFFF512F, 0xFFDD2476);

                agfx_filter_gradient(&screen, 40, 280, 600, 120, 40, 280, 640, 400, 0xFF11998E, 0xFF38EF7D);
                
                size_t txt_scratch = agfx_filters_scratch_u32(tw, th);
                uint32_t* t1 = (uint32_t*)malloc(txt_scratch * 4);
                uint32_t* t2 = (uint32_t*)malloc(txt_scratch * 4);
                
                agfx_filter_acrylic_masked(&screen, 50, 300, tw, th, 
                                           4,
                                           0x66000000,
                                           15,
                                           12345,
                                           text_mask, t1, t2);
                free(t1);
                free(t2);

                free(text_mask);
            }
            agfx_font_destroy(&font);
        } else {
            printf("Failed to initialize font!\n");
        }
        free(font_buffer);
    } else {
        printf("WARNING: Could not load 'selawk.ttf'. Please place a TTF file in the test directory.\n");
    }

    save_bmp("output_text.bmp", framebuffer, WIDTH, HEIGHT);
    printf("Saved to output_text.bmp\n4. UI Framework\n");
    
    agfx_fill_rect(&screen, 0, 0, WIDTH, HEIGHT, 0xFF111111);

    // Загружаем шрифт
    uint8_t* font_buf = load_file("selawk.ttf");
    agfx_font_t ui_font;
    int font_ok = agfx_font_init(&ui_font, font_buf, 16.0f);

    // -------------------------------------------------------------
    // [ ОКНО 1: Settings (Основное окно) ]
    // -------------------------------------------------------------
    int client1_w = 420, client1_h = 370;
    uint32_t* shm1 = (uint32_t*)calloc(client1_w * client1_h, 4);
    agfx_surface_t surf1;
    agfx_init(&surf1, shm1, client1_w, client1_h, client1_w * 4);

    agfx_ui_context_t ui1;
    agfx_ui_init(&ui1, &surf1, font_ok ? &ui_font : NULL);
    
    // Берем стандартную темную тему Windows 10
    agfx_ui_theme_t theme = agfx_ui_theme_win10_dark();
    agfx_ui_set_theme(&ui1, &theme);

    // Заливаем фон клиентского окна цветом из темы
    agfx_fill_rect(&surf1, 0, 0, client1_w, client1_h, ui1.theme.bg);

    agfx_ui_begin(&ui1, 20, 16);

    agfx_ui_label(&ui1, "System Settings (AOS UI)");
    agfx_ui_separator(&ui1, 380);

    agfx_ui_button(&ui1, "Apply", 90, 28);
    agfx_ui_same_line(&ui1);
    agfx_ui_button(&ui1, "Cancel", 90, 28);

    agfx_ui_separator(&ui1, 380);

    int check1 = 1;
    agfx_ui_checkbox(&ui1, "Enable Dark Theme", &check1);

    // Тестируем новый виджет: Радио-кнопки (выбор режима питания)
    agfx_ui_label(&ui1, "Performance Mode:");
    int power_mode = 1; // 0 = Eco, 1 = Balanced, 2 = High Performance
    agfx_ui_radio(&ui1, "Eco", &power_mode, 0);
    agfx_ui_same_line(&ui1);
    agfx_ui_radio(&ui1, "Balanced", &power_mode, 1);
    agfx_ui_same_line(&ui1);
    agfx_ui_radio(&ui1, "Max Performance", &power_mode, 2);

    agfx_ui_label(&ui1, "Computer Name:");
    agfx_ui_textbox(&ui1, "AOS-Workstation-PC", 260, 26, 1);

    agfx_ui_label(&ui1, "Storage Drive C:");
    agfx_ui_progress_bar(&ui1, 0.72f, 260, 14);

    // Размеры Окна 1 с учетом рамок
    int w1_x = 120, w1_y = 70, title_h = 32, border = 1;
    int tot1_w = client1_w + border * 2;
    int tot1_h = client1_h + title_h + border * 2;

    // 1. Мягкая тень Окна 1 (в одну строчку!)
    agfx_ui_draw_shadow(&screen, w1_x, w1_y, tot1_w, tot1_h, 16);

    // 2. Заголовок Окна 1
    agfx_fill_rect(&screen, w1_x, w1_y, tot1_w, title_h, ui1.theme.bg);
    if (font_ok) {
        int tw, th;
        uint8_t* title_mask = agfx_mask_generate_string(&ui_font, "Settings", &tw, &th);
        if (title_mask) {
            agfx_fill_alpha_mask(&screen, w1_x + 12, w1_y + (title_h - th) / 2, tw, th, title_mask, ui1.theme.text);
            agfx_mask_free(title_mask);
        }
    }

    // Кнопка закрытия Окна 1
    int btn_w = 46;
    int close_x = w1_x + tot1_w - btn_w - border;
    agfx_draw_line(&screen, close_x + 18, w1_y + 11, close_x + 28, w1_y + 21, 1, ui1.theme.text_secondary);
    agfx_draw_line(&screen, close_x + 18, w1_y + 21, close_x + 28, w1_y + 11, 1, ui1.theme.text_secondary);

    // 3. Вставляем буфер программы внутрь рамки
    agfx_blit(&screen, w1_x + border, w1_y + title_h, client1_w, client1_h, &surf1);

    // 4. Рамка окна
    agfx_draw_rect(&screen, w1_x, w1_y, tot1_w, tot1_h, border, ui1.theme.border);

    // -------------------------------------------------------------
    // [ ОКНО 2: Dialog (Стеклянное модальное окно ПОВЕРХ первого) ]
    // -------------------------------------------------------------
    int w2_x = 260, w2_y = 170, w2_w = 340, w2_h = 180;

    // 1. Мягкая тень Окна 2 (падает прямо на Окно 1, в 1 строчку!)
    agfx_ui_draw_shadow(&screen, w2_x, w2_y, w2_w, w2_h, 14);

    // 2. Размываем фон под Окном 2 (прозрачность 50%, без шума)
    size_t sc2 = agfx_filters_scratch_u32(w2_w, w2_h);
    uint32_t* tmp_c = (uint32_t*)malloc(sc2 * 4);
    uint32_t* tmp_d = (uint32_t*)malloc(sc2 * 4);
    agfx_filter_glass(&screen, w2_x, w2_y, w2_w, w2_h, 8, 0x88202028, tmp_c, tmp_d);

    // 3. Рамка активного Окна 2 (Акцентный синий цвет из темы)
    agfx_draw_rect(&screen, w2_x, w2_y, w2_w, w2_h, 1, ui1.theme.accent);

    // 4. Заголовок Окна 2
    if (font_ok) {
        int tw, th;
        uint8_t* m = agfx_mask_generate_string(&ui_font, "Save Changes?", &tw, &th);
        if (m) {
            agfx_fill_alpha_mask(&screen, w2_x + 16, w2_y + 12, tw, th, m, ui1.theme.text);
            agfx_mask_free(m);
        }
    }
    agfx_draw_line(&screen, w2_x, w2_y + 36, w2_x + w2_w, w2_y + 36, 1, 0x33FFFFFF);

    // 5. Текст диалога
    if (font_ok) {
        int tw, th;
        uint8_t* m = agfx_mask_generate_string(&ui_font, "Do you want to apply new settings?", &tw, &th);
        if (m) {
            agfx_fill_alpha_mask(&screen, w2_x + 16, w2_y + 55, tw, th, m, ui1.theme.text_secondary);
            agfx_mask_free(m);
        }
    }

    // 6. Кнопки диалога (рисуем через AGFX_UI поверх стекла)
    agfx_ui_context_t ui2;
    agfx_ui_init(&ui2, &screen, font_ok ? &ui_font : NULL);
    agfx_ui_set_theme(&ui2, &theme);
    agfx_ui_begin(&ui2, w2_x + 120, w2_y + 125);
    agfx_ui_button(&ui2, "Yes", 90, 30);
    agfx_ui_same_line(&ui2);
    agfx_ui_button(&ui2, "No", 90, 30);

    // 7. Курсор мыши поверх всего
    int mx = w2_x + 165, my = w2_y + 140;
    agfx_fill_triangle(&screen, mx, my, mx, my + 18, mx + 13, my + 13, 0xFFFFFFFF);
    agfx_draw_triangle(&screen, mx, my, mx, my + 18, mx + 13, my + 13, 1, 0xFF000000);

    free(tmp_c); free(tmp_d);
    free(shm1);
    if (font_buf) {
        free(font_buf);
        agfx_font_destroy(&ui_font);
    }

    save_bmp("output_ui.bmp", framebuffer, WIDTH, HEIGHT);
    printf("Saved to output_ui.bmp\n");

    free(framebuffer);
    return 0;
}