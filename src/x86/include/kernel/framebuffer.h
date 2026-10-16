#pragma once

#include <stdint.h>

typedef struct font_s {
    uint8_t *font;
    uint8_t width;
    uint8_t height;
} font_t;

typedef struct fb_s {
    uint8_t bpp;
    uint16_t width;
    uint16_t height;
    uint32_t pitch;
    uint8_t *framebuffer;
} fb_t;

void init_framebuffer(
    uintptr_t physical_address,
    uint16_t width, uint16_t height,
    uint32_t pitch, uint8_t bpp,
    uint32_t fill_color
);
fb_t *get_framebuffer();
