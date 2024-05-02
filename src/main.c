#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "cpu.h"
#include "disk.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "mouse.h"
#include "serial.h"

#include "../fox32rom.h"

#define FPS 60
#define TPF 1
#define TPS (FPS * TPF)

fox32_vm_t vm;

extern bool bus_requests_exit;
extern disk_controller_t disk_controller;

uint32_t tick_start;
uint32_t tick_end;
int ticks = 0;
bool done = false;

//time_t rtc_time;
uint32_t rtc_uptime;

void main_loop(void);

int main() {
    fox32_init(&vm);
    vm.io_read = bus_io_read;
    vm.io_write = bus_io_write;
    vm.halted = false;
    vm.debug = false;
    vm.headless = false;

    memcpy(vm.memory_rom, fox32rom, sizeof(fox32rom));

    //tick_start = SDL_GetTicks();
    //tick_end = SDL_GetTicks();

    new_disk("disk0.img", 0);

    while (!done && !bus_requests_exit) {
        main_loop();

        //tick_end = SDL_GetTicks();
        int delay = 1000/TPS - (tick_end - tick_start);
        if (delay > 0) {
            //sleep(delay);
        } else {
            //printf("time overrun %d\n", delay);
        }
    }

    return 0;
}

void main_loop(void) {
    //int dt = SDL_GetTicks() - tick_start;
    //tick_start = SDL_GetTicks();
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
                    yield();
                    while (true);
                }
            }

            cycles_left -= executed;
        }
    }

    if ((ticks % TPF) == 0) {
        if (!vm.headless)
            draw_framebuffer();
        fox32_raise(&vm, VSYNC_INTERRUPT_VECTOR);
        vm.halted = false;
    }

    //done = ScreenProcessEvents();

    ticks++;
}
