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

FOX32ROM_IN = fox32.rom
FOX32ROM_OUT = fox32rom.h

$(FOX32ROM_OUT): $(FOX32ROM_IN)
	xxd -i $(FOX32ROM_IN) $(FOX32ROM_OUT)
	sed -i -e 's/fox32_rom/fox32rom/' $(FOX32ROM_OUT)

fennecos_build:
	cd fennecos && ./build.sh

fennecos_image:
	cp fox32.elf fennecos/base_image/bin/
	cd fennecos && sudo ./image.sh
	cd fennecos && sudo ./image-floppy.sh
	mv fennecos/boot.img hdd.img
	mv fennecos/floppy.img fdd.img

fox32.elf: $(OBJS)
	$(CC) -o $@ $(OBJS) -I fennecos/kernel/include -I fennecos/libc/include -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdlib -lgcc

%.o: %.c $(FOX32ROM_OUT)
	$(CC) -o $@ -c $< -I fennecos/kernel/include -I fennecos/libc/include -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf fox32.elf fox32rom.h $(OBJS)
