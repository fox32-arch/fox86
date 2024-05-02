#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fox/string.h>

#include "cpu.h"
#include "keyboard.h"

extern fox32_vm_t vm;

typedef struct node_s {
    struct node_s *prev;
    struct node_s *next;
    keycode_t code;
} node_t;

static node_t *head = NULL;
static node_t *tail = NULL;

keycode_t key_take(void) {
    node_t *node = head;

    if (node == NULL) {
        return 0;
    }

    if (node == tail) {
        head = NULL;
        tail = NULL;
    } else {
        head = node->next;
        head->prev = NULL;
    }

    keycode_t code = node->code;
    return free(node), code;
}

void key_put(keycode_t code) {
    if (code == 0) return;

    node_t *node = malloc(sizeof(node_t));

    node->prev = tail;
    node->next = NULL;
    node->code = code;

    if (head == NULL) {
        head = node;
    } else {
        tail->next = node;
    }

    tail = node;
}

keycode_t key_convert(int sdlcode) {
    //if (sdlcode < 0 || sdlcode > SDL_NUM_SCANCODES) return 0;
    //return key_map[sdlcode];
    return sdlcode;
}

void key_pressed(int sdlcode) {
    //if (sdlcode == SDL_SCANCODE_F11) vm.debug = !vm.debug;
    keycode_t code = key_convert(sdlcode);
    if (code) key_put(code);
}

void key_released(int sdlcode) {
    keycode_t code = key_convert(sdlcode) | 0x80;
    if (code) key_put(code);
}
