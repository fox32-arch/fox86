.phony: all clean

all: base_image/boot/fox86.elf boot.img
floppy: base_image/boot/fox86.elf floppy.img

qemu: boot.img
	qemu-system-i386 -hda boot.img -serial stdio -m 256M

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
KERNEL_FILES := $(call rwildcard,src/x86,*.c)
KERNEL_OUT_FILES := src/x86/boot.o ${KERNEL_FILES:.c=.o}
LIBK_FILES := $(call rwildcard,src/x86/libk,*.c)
LIBK_OUT_FILES := ${LIBK_FILES:.c=.o}
FOX32_FILES := $(call rwildcard,src/fox32,*.c)
FOX32_OUT_FILES := ${FOX32_FILES:.c=.o}

src/x86/boot.o: src/x86/boot.s
	@echo '[ASM] $<' && \
	i686-elf-as src/x86/boot.s -o src/x86/boot.o

%.o: %.c
	@echo '[CC] $<' && \
	i686-elf-gcc -c $< -o $@ -g -std=gnu99 -ffreestanding -O3 -ffast-math -Wall -Wextra -Isrc/x86/include/ -Isrc/x86/libk/include/ -Isrc/

base_image/boot/fox86.elf: $(KERNEL_OUT_FILES) $(LIBK_OUT_FILES) $(FOX32_OUT_FILES)
	@echo '[LINK] $@' && \
	i686-elf-gcc -T src/x86/linker.ld -o $@ -ffreestanding -O3 -ffast-math -nostdlib $^ -lgcc

boot.img: base_image/boot/fox86.elf
	sudo bash image.sh

floppy.img: base_image/boot/fox86.elf
	sudo bash image-floppy.sh

FORCE: ;
clean:
	rm -f boot.img floppy.img $(KERNEL_OUT_FILES) $(LIBK_OUT_FILES) $(FOX32_OUT_FILES)
