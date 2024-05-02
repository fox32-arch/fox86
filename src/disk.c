#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "disk.h"

disk_controller_t disk_controller;

extern fox32_vm_t vm;

void new_disk(const char *filename, size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return; }
    printf("mounting %s as disk ID %d\n", filename, (int) id);
    disk_controller.disks[id].file = open(filename, MODE_READ | MODE_WRITE);
    if (!disk_controller.disks[id].file) {
        printf("couldn't open disk file\n");
        exit(1);
    }
    //fseek(disk_controller.disks[id].file, 0, SEEK_END);
    //disk_controller.disks[id].size = ftell(disk_controller.disks[id].file);
    disk_controller.disks[id].size = 512 * 1024; // TODO: fix this!!!!!!!
    //rewind(disk_controller.disks[id].file);
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
    close(disk_controller.disks[id].file);
    disk_controller.disks[id].size = 0;
}

uint64_t get_disk_size(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return 0; }
    return disk_controller.disks[id].size;
}

void set_disk_sector(size_t id, uint64_t sector) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return; }
    seek(disk_controller.disks[id].file, sector * 512);
}

size_t read_disk_into_memory(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return 0; }
    return read(disk_controller.disks[id].file, &vm.memory_ram[disk_controller.buffer_pointer], 512);
}

size_t write_disk_from_memory(size_t id) {
    if (id > 3) { puts("attempting to access disk with ID > 3"); return 0; }
    return write(disk_controller.disks[id].file, &vm.memory_ram[disk_controller.buffer_pointer], 512);
}
