AS = ~/opt/cross/bin/i686-elf-as
CC = ~/opt/cross/bin/i686-elf-gcc

CFILES = src/main.c \
		src/bus.c \
		src/cpu.c \
		src/disk.c \
		src/framebuffer.c \
		src/keyboard.c \
		src/mmu.c \
		src/mouse.c

OBJS = $(addsuffix .o, $(basename $(CFILES)))
OBJS += \
	fennecos/build/user/crt0.o \
	fennecos/build/user/framebuffer.o \
	fennecos/build/user/keyboard.o \
	fennecos/build/user/user.o \
	fennecos/build/libc/fox/alloc.o \
	fennecos/build/libc/fox/string.o \
	fennecos/build/libc/stdio/getchar.o \
	fennecos/build/libc/stdio/gets.o \
	fennecos/build/libc/stdio/printf.o \
	fennecos/build/libc/stdio/putchar.o \
	fennecos/build/libc/stdio/puts.o \
	fennecos/build/libc/stdio/vprintf.o \
	fennecos/build/libc/stdlib/itoa.o \
	fennecos/build/libc/stdlib/utoa.o \
	fennecos/build/libc/string/memcmp.o \
	fennecos/build/libc/string/memcpy.o \
	fennecos/build/libc/string/memmove.o \
	fennecos/build/libc/string/memset.o \
	fennecos/build/libc/string/strcat.o \
	fennecos/build/libc/string/strchr.o \
	fennecos/build/libc/string/strcmp.o \
	fennecos/build/libc/string/strcpy.o \
	fennecos/build/libc/string/strlen.o

.PHONY: all
all: fennecos_build fox32.elf fennecos_image

fennecos_build:
	cd fennecos && ./build.sh

fennecos_image:
	cp fox32.elf fennecos/base_image/bin/
	cp fox32.rom fennecos/base_image/bin/
	cd fennecos && sudo ./image.sh
	mv fennecos/boot.img hdd.img
	cd fennecos && sudo ./image-floppy.sh
	mv fennecos/floppy.img fdd.img

fox32.elf: $(OBJS)
	$(CC) -o $@ $(OBJS) -I fennecos/kernel/include -I fennecos/libc/include -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -lgcc

%.o: %.c $(FOX32ROM_OUT)
	$(CC) -o $@ -c $< -I fennecos/kernel/include -I fennecos/libc/include -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf fox32.elf fox32rom.h $(OBJS)
