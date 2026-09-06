#include "agfx_text.h"

static size_t internal_strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

static void* internal_memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dest;
}

static void* internal_memset(void* ptr, int value, size_t n) {
    uint8_t* p = (uint8_t*)ptr;
    while (n--) *p++ = (uint8_t)value;
    return ptr;
}

static int agfx_ifloor(float x) { return (int)x - (x < 0.0f && x != (int)x); }
static int agfx_iceil(float x) { return (int)x + (x > 0.0f && x != (int)x); }
static float agfx_fabs(float x) { return x < 0.0f ? -x : x; }
static float agfx_fmod(float x, float y) { return x - (int)(x / y) * y; }

static float agfx_sqrt(float x) {
    if (x <= 0.0f) return 0.0f;
    float res = x;
    for (int i = 0; i < 10; i++) res = 0.5f * (res + x / res);
    return res;
}

static float agfx_cos(float x) {
    float x2 = x * x;
    return 1.0f - x2/2.0f + (x2*x2)/24.0f - (x2*x2*x2)/720.0f;
}

static float agfx_acos(float x) {
    float negate = (float)(x < 0);
    x = agfx_fabs(x);
    float ret = -0.0187293f;
    ret = ret * x + 0.0742610f;
    ret = ret * x - 0.2121144f;
    ret = ret * x + 1.5707288f;
    ret = ret * agfx_sqrt(1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * 3.14159265358979f + ret;
}

static float agfx_pow(float x, float y) {
    (void)y;
    if (x == 0.0f) return 0.0f;
    float z = x < 0 ? -x : x;
    float res = z;
    for(int i=0; i<15; i++) res = (2.0f*res + z/(res*res))/3.0f;
    return x < 0 ? -res : res;
}

#define STBTT_NO_STDIO
#define STBTT_assert(x)    ((void)(0))
#define STBTT_malloc(x,u)  ((void)(u), agfx_malloc(x))
#define STBTT_free(x,u)    ((void)(u), agfx_free(x))

#define STBTT_strlen       internal_strlen
#define STBTT_memcpy       internal_memcpy
#define STBTT_memset       internal_memset

#define STBTT_ifloor(x)    agfx_ifloor(x)
#define STBTT_iceil(x)     agfx_iceil(x)
#define STBTT_fabs(x)      agfx_fabs(x)
#define STBTT_sqrt(x)      agfx_sqrt(x)
#define STBTT_fmod(x,y)    agfx_fmod(x,y)
#define STBTT_cos(x)       agfx_cos(x)
#define STBTT_acos(x)      agfx_acos(x)
#define STBTT_pow(x,y)     agfx_pow(x,y)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

int agfx_font_init(agfx_font_t* font, const uint8_t* ttf_buffer, float pixel_height) {
    if (!font || !ttf_buffer) return 0;

    font->info = agfx_malloc(sizeof(stbtt_fontinfo));
    if (!font->info) return 0;

    stbtt_fontinfo* info = (stbtt_fontinfo*)font->info;

    if (!stbtt_InitFont(info, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0))) {
        agfx_free(font->info);
        font->info = NULL;
        return 0;
    }

    font->scale = stbtt_ScaleForPixelHeight(info, pixel_height);
    stbtt_GetFontVMetrics(info, &font->ascent, &font->descent, &font->line_gap);

    return 1;
}

void agfx_font_destroy(agfx_font_t* font) {
    if (font && font->info) {
        agfx_free(font->info);
        font->info = NULL;
    }
}

void agfx_mask_free(void* mask) {
    agfx_free(mask);
}

uint8_t* agfx_mask_generate_string(const agfx_font_t* font, const char* text, int* out_w, int* out_h) {
    if (!font || !font->info || !text || !out_w || !out_h) return NULL;

    stbtt_fontinfo* info = (stbtt_fontinfo*)font->info;

    int total_width = 0;
    int i = 0;
    while (text[i]) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(info, text[i], &advance, &lsb);
        total_width += (int)(advance * font->scale);
        
        if (text[i+1]) {
            total_width += (int)(stbtt_GetCodepointKernAdvance(info, text[i], text[i+1]) * font->scale);
        }
        i++;
    }

    int total_height = (int)((font->ascent - font->descent) * font->scale);
    if (total_width <= 0 || total_height <= 0) return NULL;

    *out_w = total_width;
    *out_h = total_height;

    uint8_t* string_mask = (uint8_t*)agfx_malloc(total_width * total_height);
    if (!string_mask) return NULL;
    internal_memset(string_mask, 0, total_width * total_height);

    int current_x = 0;
    int baseline = (int)(font->ascent * font->scale);

    i = 0;
    while (text[i]) {
        int w, h, xoff, yoff;
        uint8_t* char_bitmap = stbtt_GetCodepointBitmap(info, 0, font->scale, text[i], &w, &h, &xoff, &yoff);

        if (char_bitmap) {
            int draw_y = baseline + yoff;
            int draw_x = current_x + xoff;

            for (int cy = 0; cy < h; cy++) {
                for (int cx = 0; cx < w; cx++) {
                    if (draw_y + cy >= 0 && draw_y + cy < total_height &&
                        draw_x + cx >= 0 && draw_x + cx < total_width) {
                        
                        int mask_idx = (draw_y + cy) * total_width + (draw_x + cx);
                        uint8_t src_val = char_bitmap[cy * w + cx];
                        
                        if (src_val > string_mask[mask_idx]) {
                            string_mask[mask_idx] = src_val;
                        }
                    }
                }
            }
            agfx_free(char_bitmap);
        }

        int advance, lsb;
        stbtt_GetCodepointHMetrics(info, text[i], &advance, &lsb);
        current_x += (int)(advance * font->scale);
        if (text[i+1]) {
            current_x += (int)(stbtt_GetCodepointKernAdvance(info, text[i], text[i+1]) * font->scale);
        }
        i++;
    }

    return string_mask;
}