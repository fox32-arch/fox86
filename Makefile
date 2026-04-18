AS = i686-elf-as
CC = i686-elf-gcc

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
	fennecos/user/crt0.o \
	fennecos/user/framebuffer.o \
	fennecos/user/keyboard.o \
	fennecos/user/user.o \
	fennecos/libc/stdio/getchar.o \
	fennecos/libc/stdio/gets.o \
	fennecos/libc/stdio/printf.o \
	fennecos/libc/stdio/putchar.o \
	fennecos/libc/stdio/puts.o \
	fennecos/libc/stdio/vprintf.o \
	fennecos/libc/stdlib/itoa.o \
	fennecos/libc/stdlib/utoa.o \
	fennecos/libc/string/memcmp.o \
	fennecos/libc/string/memcpy.o \
	fennecos/libc/string/memmove.o \
	fennecos/libc/string/memset.o \
	fennecos/libc/string/strcat.o \
	fennecos/libc/string/strchr.o \
	fennecos/libc/string/strcmp.o \
	fennecos/libc/string/strcpy.o \
	fennecos/libc/string/strlen.o

.PHONY: all
all: fennecos_build fox32.app fennecos_image

fennecos_build:
	$(MAKE) -C fennecos

fennecos_image:
	rm -rf fennecos/base_image/app/*
	cp fox32.app fennecos/base_image/app/
	cp fox32.rom fennecos/base_image/app/
	cd fennecos && sudo ./image.sh
	mv fennecos/boot.img hdd.img
	cd fennecos && sudo ./image-floppy.sh
	mv fennecos/floppy.img fdd.img

fox32.app: $(OBJS)
	$(CC) -o $@.elf $(OBJS) -pie -flinker-output=pie -T fennecos/user/user.ld -ffreestanding -O2 -nostdlib -lgcc -Wl,--no-warn-rwx-segments
	fennecos/elf2app.py $@.elf $@
	rm $@.elf

%.o: %.c $(FOX32ROM_OUT)
	$(CC) -o $@ -c $< -g -std=gnu99 -fpie -T fennecos/user/user.ld -Wl,-no-dynamic-linker -ffreestanding -O2 -Wall -Wextra -Ifennecos/include/ -Ifennecos/libc/include/

clean:
	rm -rf fox32.elf fox32rom.h $(OBJS)
