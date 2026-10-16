#include <kernel/mouse.h>
#include <kernel/isr.h>
#include <kernel/io.h>
#include <kernel/trapframe.h>
#include <kernel/event.h>
#include <kernel/pic.h>

#include <stdint.h>
#include <stdbool.h>

uint32_t state = mouse_state_0;
uint8_t byte_0;
uint8_t byte_1;
uint8_t byte_2;

mouse_buttons_t mouse_buttons = { .left = false, .middle = false, .right = false };
uint16_t mouse_x = 0;
uint16_t mouse_y = 0;

static void update_mouse() {
    int16_t rel_x = byte_1 - ((byte_0 << 4) & 0x100);
    int16_t rel_y = byte_2 - ((byte_0 << 3) & 0x100);
    if (rel_x > 0) mouse_x += rel_x; else if ((int16_t) mouse_x > -rel_x) mouse_x -= (-rel_x);
    if (rel_y > 0) {
        if (mouse_y > rel_y) mouse_y -= rel_y;
    } else mouse_y += (-rel_y);
    if (mouse_x > 640) mouse_x = 640;
    if (mouse_y > 480) mouse_y = 480;

    mouse_buttons_t old_mouse_buttons;
    old_mouse_buttons.left = mouse_buttons.left;
    old_mouse_buttons.middle = mouse_buttons.middle;
    old_mouse_buttons.right = mouse_buttons.right;

    mouse_buttons.left = (byte_0 & 1) != 0;
    mouse_buttons.middle = (byte_0 & 4) != 0;
    mouse_buttons.right = (byte_0 & 2) != 0;

    // TODO: other buttons?
    if (mouse_buttons.left != old_mouse_buttons.left) {
        event_t event;
        event.type = mouse_buttons.left ? MOUSE_DOWN : MOUSE_UP;
        event.arg0 = mouse_x;
        event.arg1 = mouse_y;
        new_event(&event);
    }
}

void mouse_wait(bool read) {
    uint32_t timeout = 900000;
    if (read) {
        while (--timeout)
            if ((inb(0x64) & 1) != 0)
                return;
    } else {
        while (--timeout)
            if ((inb(0x64) & 2) == 0)
                return;
    }
}

void mouse_wait_ack() {
    uint8_t value = 0;
    while (value != 0xFA) {
        mouse_wait(true);
        value = inb(0x60);
    }
}

void mouse_wait_reset() {
    uint8_t value = 0;
    while (value != 0xAA) {
        mouse_wait(true);
        value = inb(0x60);
    }
}

void init_mouse() {
    // flush buffer
    while ((inb(0x64) >> 0) & 1) inb(0x60);

    // disable interrupts
    mouse_wait(false);
    outb(0x64, 0x20);
    mouse_wait(true);
    uint8_t status = inb(0x60);
    status &= 0b01000100;
    mouse_wait(false);
    outb(0x64, 0x60);
    mouse_wait(false);
    outb(0x60, status);
    mouse_wait(true);
    inb(0x60); // setting the compaq byte *might* generate a reply?

    // disable keyboard
    mouse_wait(false);
    outb(0x64, 0xAD);
    mouse_wait(true);
    inb(0x60);

    // enable mouse
    mouse_wait(false);
    outb(0x64, 0xA8);
    mouse_wait(true);
    inb(0x60);

    // reset mouse
    mouse_wait(false);
    outb(0x64, 0xD4);
    mouse_wait(false);
    outb(0x60, 0xFF);
    mouse_wait_reset();

    // set mouse defaults
    mouse_wait(false);
    outb(0x64, 0xD4);
    mouse_wait(false);
    outb(0x60, 0xF6);
    mouse_wait_ack();

    // enable data reporting
    mouse_wait(false);
    outb(0x64, 0xD4);
    mouse_wait(false);
    outb(0x60, 0xF4);
    mouse_wait_ack();

    // re-enable keyboard
    mouse_wait(false);
    outb(0x64, 0xAE);

    // enable keyboard and mouse interrupts
    mouse_wait(false);
    outb(0x64, 0x20);
    mouse_wait(true);
    status = inb(0x60);
    status |= 1 << 0;
    status |= 1 << 1;
    status &= ~(1 << 5);
    mouse_wait(false);
    outb(0x64, 0x60);
    mouse_wait(false);
    outb(0x60, status);
    mouse_wait(true);
    inb(0x60); // setting the compaq byte *might* generate a reply?

    // flush buffer
    while ((inb(0x64) >> 0) & 1) inb(0x60);

    state = mouse_state_0;

    pic_unmask(2);
    pic_unmask(12);
    install_interrupt_handler(12, mouse_interrupt_handler);
}

void mouse_interrupt_handler(uint8_t irq, trap_frame_t *trap_frame, uint32_t error) {
    (void) irq;
    (void) trap_frame;
    (void) error;

    if ((inb(0x64) & 0x20) == 0) return;

    switch (state) {
        case mouse_state_0: {
            state = mouse_state_1;
            byte_0 = inb(0x60);
            break;
        }
        case mouse_state_1: {
            state = mouse_state_2;
            byte_1 = inb(0x60);
            break;
        }
        case mouse_state_2: {
            state = mouse_state_0;
            byte_2 = inb(0x60);
            update_mouse();
            break;
        }
    }
}
