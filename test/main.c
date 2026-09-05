#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../agfx.h"
#include "../agfx_ext.h" // Не забываем подключить модуль расширений!

#define WIDTH 800
#define HEIGHT 600

// Функция сохранения в BMP-файл
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

int main() {
    printf("Starting AGFX Showcase...\n");

    int pitch = WIDTH * 4;
    uint32_t* framebuffer = (uint32_t*)calloc(WIDTH * HEIGHT, 4);

    agfx_surface_t screen;
    agfx_init(&screen, framebuffer, WIDTH, HEIGHT, pitch);

    // ==========================================
    // ФОН И СЕТКА (Для теста прозрачности)
    // ==========================================
    agfx_fill_rect(&screen, 0, 0, WIDTH, HEIGHT, 0xFF1E1E2E); // Темно-серый фон

    // Рисуем тонкую сетку
    for (int x = 0; x < WIDTH; x += 50) {
        agfx_draw_line(&screen, x, 0, x, HEIGHT, 1, 0xFF2A2A3B);
    }
    for (int y = 0; y < HEIGHT; y += 50) {
        agfx_draw_line(&screen, 0, y, WIDTH, y, 1, 0xFF2A2A3B);
    }


    // ==========================================
    // ЗОНА 1: Прямоугольники (Верхний левый угол)
    // ==========================================
    // 1. Градиентный квадрат
    agfx_fill_rect_gradient(&screen, 40, 40, 160, 160, 
                            40, 40, 200, 200, 
                            0xFF8A2387, 0xFFE94057); // Фиолетово-красный

    // 2. Полупрозрачный скругленный прямоугольник (накладывается поверх!)
    agfx_fill_rect_rounded(&screen, 120, 100, 200, 120, 25, 0xBB000000); // Черный, прозрачность ~70%

    // 3. Контурный скругленный прямоугольник (Толщина 3)
    agfx_draw_rect_rounded(&screen, 120, 100, 200, 120, 25, 3, 0xFFF2A65A); // Оранжевый контур


    // ==========================================
    // ЗОНА 2: Круги и Экзотика (Верхний правый угол)
    // ==========================================
    // 1. Градиентный круг (Из модуля agfx_ext)
    agfx_fill_circle_gradient(&screen, 550, 130, 80, 
                              550, 50, 550, 210, 
                              0xFF11998E, 0xFF38EF7D); // Зеленый градиент сверху вниз

    // 2. Сплошной полумесяц (Из модуля agfx_ext)
    agfx_fill_crescent(&screen, 700, 120, 60,   // Основной круг
                                720, 100, 55,   // Вырезающий круг
                                0xFFF9F871);    // Желтый цвет

    // 3. Контурный круг (поверх градиентного, для красоты)
    agfx_draw_circle(&screen, 550, 130, 90, 0x88FFFFFF); // Белый полупрозрачный контур


    // ==========================================
    // ЗОНА 3: Многоугольники (Нижний левый угол)
    // ==========================================
    // 1. Залитый шестиугольник (Триангуляция веером)
    agfx_point_t hex[6] = {
        {150, 350}, {230, 390}, {230, 480}, 
        {150, 520}, {70, 480}, {70, 390}
    };
    agfx_fill_polygon(&screen, hex, 6, 0xFF00C9FF); // Голубой

    // 2. Полупрозрачный треугольник (Пересекает шестиугольник)
    agfx_fill_triangle(&screen, 150, 420, 320, 550, 70, 580, 0x99FF92A5); // Розовый с альфой

    // 3. Контурная звезда с толщиной 2 пикселя
    agfx_point_t star[5] = {
        {280, 370}, {310, 470}, {220, 410}, 
        {340, 410}, {250, 470}
    };
    agfx_draw_polygon(&screen, star, 5, 2, 0xFFFFD700); // Золотой


    // ==========================================
    // ЗОНА 4: Линии и Кривые Безье (Нижний правый угол)
    // ==========================================
    // 1. Толстая градиентная линия (Толщина 10)
    agfx_draw_line_gradient(&screen, 420, 500, 760, 400, 10,
                            420, 500, 760, 400,
                            0xFFFF512F, 0xFFDD2476); // Огненный градиент

    // 2. Кривая Безье (Плавная дуга с толщиной 5)
    // Старт (450, 450) -> Контрольная точка тянет вверх (600, 280) -> Финиш (750, 450)
    agfx_draw_bezier(&screen, 450, 450, 600, 280, 750, 450, 5, 0xFF00FF87); // Неоновый зеленый

    // 3. Тонкие "скелетные" линии для понимания контрольной точки Безье
    agfx_draw_line(&screen, 450, 450, 600, 280, 1, 0x55FFFFFF);
    agfx_draw_line(&screen, 600, 280, 750, 450, 1, 0x55FFFFFF);


    // ==========================================
    // СОХРАНЕНИЕ
    // ==========================================
    save_bmp("output.bmp", framebuffer, WIDTH, HEIGHT);
    printf("Saved to output.bmp\n");

    free(framebuffer);
    return 0;
}