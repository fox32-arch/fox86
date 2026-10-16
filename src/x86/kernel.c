#include <kernel/allocator.h>
#include <kernel/event.h>
#include <kernel/floppy.h>
#include <kernel/framebuffer.h>
#include <kernel/gdt.h>
#include <kernel/ide.h>
#include <kernel/idt.h>
#include <kernel/io.h>
#include <kernel/mouse.h>
#include <kernel/multiboot.h>
#include <kernel/pic.h>
#include <kernel/pit.h>
#include <kernel/ps2.h>
#include <kernel/serial.h>
#include <kernel/timer.h>

#include <fox32/bus.h>
#include <fox32/cpu.h>
#include <fox32/disk.h>
#include <fox32/framebuffer.h>
#include <fox32/keyboard.h>
#include <fox32/mouse.h>

#include "../fox32rom.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

multiboot_info_t copied_multiboot_struct;

char boot_disk_char;

fox32_vm_t vm;

// timer runs at 100 Hz
#define FPS 50
#define TPF 1
#define TPS (FPS * TPF)

extern bool bus_requests_exit;
extern disk_controller_t disk_controller;
extern uint8_t *hardware_framebuffer;

extern uint16_t mouse_x;
extern uint16_t mouse_y;

uint32_t tick_start;
uint32_t tick_end;
int tick = 0;
bool done = false;

//time_t rtc_time;
uint32_t rtc_uptime;

void main_loop(void);

void kernel_main(multiboot_info_t *multiboot_struct) {
    init_gdt();
    init_serial();
    init_pit();
    init_pic();
    init_ide();
    init_timer();
    init_ps2_keyboard();
    init_mouse();
    memcpy(&copied_multiboot_struct, multiboot_struct, sizeof(multiboot_info_t));
    init_allocator(copied_multiboot_struct.mmap_addr, copied_multiboot_struct.mmap_length);
    init_floppy_1();
    init_idt();
    init_floppy_2();

    bool booting_from_floppy = (copied_multiboot_struct.boot_device >> 24) == 0x00 ? true : false;
    boot_disk_char = booting_from_floppy ? '2' : '1';

    // initialize the framebuffer
    init_framebuffer(
        copied_multiboot_struct.framebuffer_addr,
        copied_multiboot_struct.framebuffer_width,
        copied_multiboot_struct.framebuffer_height,
        copied_multiboot_struct.framebuffer_pitch,
        copied_multiboot_struct.framebuffer_bpp,
        0x444444
    );

    fox32_init(&vm);
    vm.io_read = bus_io_read;
    vm.io_write = bus_io_write;
    vm.halted = false;
    vm.debug = false;
    vm.headless = false;

    hardware_framebuffer = (uint8_t *) (uintptr_t) copied_multiboot_struct.framebuffer_addr;

    memcpy(vm.memory_rom, fox32rom, fox32rom_len);

    while (!done && !bus_requests_exit) {
        tick_start = get_timer_value();
        main_loop();
        tick_end = get_timer_value();
        kprintf("main loop took %d ticks\n", tick_end - tick_start);

        /*tick_end = get_timer_value();
        int delay = 1000/TPS - (tick_end - tick_start);
        if (delay > 0) {
            //kprintf("sleeping %d\n", delay);
            //sleep(delay);
        } else {
            //kprintf("time overrun %d\n", delay);
        }*/
    }

    abort();
}

void main_loop(void) {
    //int dt = get_timer_value() - tick_start;
    //tick_start = get_timer_value();
    //if (!dt)
    //    dt = 1;

    int dt = 1;

    int cycles_per_tick = FOX32_CPU_HZ / TPS / dt;
    int extra_cycles = FOX32_CPU_HZ / TPS - (cycles_per_tick * dt);

    fox32_err_t error = FOX32_ERR_OK;

    for (int i = 0; i < 1; i++) {
        rtc_uptime += 1;
        //rtc_time = time(NULL);

        int cycles_left = cycles_per_tick;

        if (i == dt - 1)
            cycles_left += extra_cycles;

        while (cycles_left > 0) {
            uint32_t executed = 0;

            //printf("starting, vm.pointer_instr: 0x%X\n", vm.pointer_instr);
            //printf("rom[0] = %X, rom[1] = %X\n", vm.memory_rom[0], vm.memory_rom[1]);
            error = fox32_resume(&vm, cycles_left, &executed);
            //printf("executed %d instructions, vm.pointer_instr: 0x%X\n", executed, vm.pointer_instr);
            //printf("rom[0] = %X, rom[1] = %X\n", vm.memory_rom[0], vm.memory_rom[1]);
            if (error != FOX32_ERR_OK) {
                //if (vm.debug) puts(fox32_strerr(error));
                error = fox32_recover(&vm, error);
                if (error != FOX32_ERR_OK) {
                    printf("error: %d\n", error);
                }
            }

            cycles_left -= executed;
        }
    }

    mouse_moved(mouse_x, mouse_y);

    event_t event;
    if (get_next_event(&event)) {
        if (event.type == KEY_DOWN) {
            if (event.arg0 == 87) // F11
                done = true;
            else
                key_pressed(event.arg0);
        } else if (event.type == KEY_UP) {
            key_released(event.arg0);
        } else if (event.type == MOUSE_DOWN) {
            mouse_pressed(0);
        } else if (event.type == MOUSE_UP) {
            mouse_released(0);
        }
    }

    if ((tick % TPF) == 0) {
        if (!vm.headless)
            draw_framebuffer();
        fox32_raise(&vm, VSYNC_INTERRUPT_VECTOR);
        vm.halted = false;
    }

    //done = ScreenProcessEvents();

    tick++;
}
