#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "disk.h"

#include <kernel/floppy.h>
#include <kernel/ide.h>

disk_controller_t disk_controller;

extern fox32_vm_t vm;

uint32_t harddisk_sector = 0;
uint32_t floppy_sector = 0;

void new_disk(const char *filename, size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return; }
    printf("mounting %s as disk ID %d\n", filename, (int) id);
    /*disk_controller.disks[id].file = open(filename, MODE_READ | MODE_WRITE);
    if (disk_controller.disks[id].file == (uint32_t) -1) {
        printf("couldn't open disk file\n");
        yield();
        return;
    }
    //fseek(disk_controller.disks[id].file, 0, SEEK_END);
    //disk_controller.disks[id].size = ftell(disk_controller.disks[id].file);
    disk_controller.disks[id].size = 512 * 1024; // TODO: fix this!!!!!!!
    //rewind(disk_controller.disks[id].file);
    yield();*/
}

void insert_disk(disk_t disk, size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return; }
    if (disk_controller.disks[id].size > 0) remove_disk(id);
    printf("mounting disk ID %d\n", (int) id);
    disk_controller.disks[id] = disk;
}

void remove_disk(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return; }
    printf("unmounting disk ID %d\n", (int) id);
    //close(disk_controller.disks[id].file);
    disk_controller.disks[id].size = 0;
}

uint64_t get_disk_size(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return 0; }

    if (id == 0) return 1440 * 1024;      // floppy disk is always shown as 1440 KiB
    //if (id == 1) return 16 * 1024 * 1024; // IDE disk is always shown as 16 MiB
    return 0;
}

void set_disk_sector(size_t id, uint64_t sector) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return; }

    if (id == 0) floppy_sector = sector;
    if (id == 1) harddisk_sector = sector;
    //seek(disk_controller.disks[id].file, sector * 512);
}

size_t read_disk_into_memory(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return 0; }
    switch (id) {
        case 0:
            floppy_read_sector(floppy_sector, &vm.memory_ram[disk_controller.buffer_pointer]);
            return 512;
        case 1:
            read_sector(&vm.memory_ram[disk_controller.buffer_pointer], harddisk_sector);
            return 512;
        default:
            return 0;
    }
}

size_t write_disk_from_memory(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return 0; }
    switch (id) {
        case 0:
            return 0;
        case 1:
            write_sector(&vm.memory_ram[disk_controller.buffer_pointer], harddisk_sector);
            return 512;
        default:
            return 0;
    }
}
