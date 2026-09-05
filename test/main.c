#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../agfx.h"
#include "../agfx_filters.h"
#include "../agfx_mask.h"

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

int main() {
    printf("Starting AGFX Showcase...\n1. Basic\n");

    int pitch = WIDTH * 4;
    uint32_t* framebuffer = (uint32_t*)calloc(WIDTH * HEIGHT, 4);

    agfx_surface_t screen;
    agfx_init(&screen, framebuffer, WIDTH, HEIGHT, pitch);

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
    printf("Saved to output_filters.bmp\n");

    free(framebuffer);
    return 0;
}