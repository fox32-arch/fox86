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

bool background_dirty = true;
uint8_t *hardware_framebuffer;

overlay_t *overlay_get(uint32_t index) {
    //if (index >= 32) abort();
    return &overlays[index];
}

void draw_framebuffer() {
    bool needs_hw_redraw = false;
    bool ignore_dirty_flag = false;

    if (background_dirty) {
        memcpy(framebuffer, &vm.memory_ram[0x02000000], FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4);
        needs_hw_redraw = true;
    }

    for (size_t i = 0; i < 32; i++) {
        overlay_t *overlay = &overlays[i];
        if (!overlay->enabled) continue;
        if (!ignore_dirty_flag) {
            if (overlay->dirty) {
                needs_hw_redraw = true;
                overlay->dirty = false;
                if (i > 0) {
                    // redraw the overlay below this
                    i--;
                    continue;
                }
                ignore_dirty_flag = true;
            } else if (!background_dirty) {
                continue;
            }
        }

        kprintf("drawing overlay %d\n", i);

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
    if (needs_hw_redraw) {
        kprintf("hw redraw...\n");
        for (uint32_t i = 0; i < FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4; i += 1) {
            ((uint32_t *) hardware_framebuffer)[i] = __builtin_bswap32(((uint32_t *) framebuffer)[i]) >> 8;
        }
    }

    if (background_dirty)
        background_dirty = false;
}
