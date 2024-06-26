#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "cpu.h"
#include "framebuffer.h"

extern fox32_vm_t vm;

static uint8_t framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4];
static overlay_t overlays[32];

uint8_t *hardware_framebuffer = (void *) 0xF0000000;

overlay_t *overlay_get(uint32_t index) {
    //if (index >= 32) abort();
    return &overlays[index];
}

void draw_framebuffer() {
    memcpy(framebuffer, &vm.memory_ram[0x02000000], FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4);

    for (size_t i = 0; i < 32; i++) {
        overlay_t *overlay = &overlays[i];
        if (!overlay->enabled) continue;

        size_t pointer = overlay->pointer;

        size_t height = overlay->height;
        size_t width = overlay->width;

        size_t ymin = overlay->y, ymax = ymin + height;
        size_t xmin = overlay->x, xmax = xmin + width;
        ymax = ymax < FRAMEBUFFER_HEIGHT ? ymax : FRAMEBUFFER_HEIGHT;
        xmax = xmax < FRAMEBUFFER_WIDTH ? xmax : FRAMEBUFFER_WIDTH;
        if (ymin >= ymax) continue;
        if (xmin >= xmax) continue;

        for (size_t y = ymin; y < ymax; y++) {
            for (size_t x = xmin; x < xmax; x++) {
                size_t index_dst = (x + y * FRAMEBUFFER_WIDTH) * 4;
                size_t index_src = ((x - xmin) + (y - ymin) * width) * 4 + pointer;
                if (vm.memory_ram[index_src + 3] > 0) {
                    memcpy(&framebuffer[index_dst], &vm.memory_ram[index_src], 4);
                }
            }
        }
    }

    //memcpy((void *) 0xF0000000, framebuffer, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4);
    for (uint32_t i = 0; i < FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4; i += 4) {
        hardware_framebuffer[i] = framebuffer[i+2];
        hardware_framebuffer[i+1] = framebuffer[i+1];
        hardware_framebuffer[i+2] = framebuffer[i];
    }
}
