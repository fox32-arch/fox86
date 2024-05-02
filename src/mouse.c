#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mouse.h"

mouse_t mouse;

void mouse_moved(int x, int y) {
    mouse.x = x;
    mouse.y = y;

    if (mouse.x > 0x8000) mouse.x = 0;
    if (mouse.x > 640) mouse.x = 640;
    if (mouse.y > 0x8000) mouse.y = 0;
    if (mouse.y > 480) mouse.y = 480;
}

void mouse_pressed(int button) {
    (void) button; // TODO: check which button was clicked
    mouse.clicked = true;
    mouse.held = true;
}

void mouse_released(int button) {
    (void) button; // TODO: check which button was released
    mouse.released = true;
    mouse.held = false;
}
