C_SOURCES = $(wildcard drivers/*.c kernel/*.c libs/*.c cpu/*.c tests/*.c fs/*.c)
S_SOURCES = $(wildcard cpu/*.asm kernel/*.asm)
HEADERS = $(wildcard include/drivers/*.h include/kernel/*.h include/libs/*.h include/tests/*.h include/fs/*.h)
TARGET_DIR = build
DIST_DIR = dist
OBJ = $(addprefix $(TARGET_DIR)/, $(C_SOURCES:.c=.o) $(S_SOURCES:.asm=.o))
GDB = i386-elf-gdb
CC = i386-elf-gcc
CFLAGS = -g -m32 -fno-builtin -ffreestanding -fno-stack-protector -nostartfiles -nodefaultlibs \
	     -Wall -Wextra -Werror -fno-exceptions -Iinclude

.PHONY: all clean run debug makedir build tools
all: makedir run
build: makedir $(DIST_DIR)/os-image

makedir:
	mkdir -p build/boot build/kernel build/fs build/cpu build/drivers build/libs build/tests dist/tools

run: $(DIST_DIR)/os-image
	qemu-system-i386 -curses -fda $< -hda fs.img -no-shutdown -no-reboot

debug: build $(TARGET_DIR)/kernel.elf
	qemu-system-i386 -s -S -curses -fda $(DIST_DIR)/os-image -hda fs.img -d guest_errors,int

clean:
	rm -r build/* dist/*

tools: makedir $(DIST_DIR)/tools/mkfs

$(DIST_DIR)/tools/mkfs: tools/mkfs.c tools/utils/argparse.c fs/bitmap.c
	gcc -m32 $^ -Iinclude -o $@

$(DIST_DIR)/os-image: $(TARGET_DIR)/boot/boot.bin $(TARGET_DIR)/kernel.bin
	cat $^ > $@

$(TARGET_DIR)/boot/kernel_entry.o: boot/kernel_entry.asm
	nasm -f elf $< -o $@

$(TARGET_DIR)/kernel.bin: $(TARGET_DIR)/boot/kernel_entry.o $(OBJ)
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

$(TARGET_DIR)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_DIR)/%.o: %.asm
	nasm $< -f elf -o $@

$(TARGET_DIR)/%.bin: %.asm
	nasm -f bin $< -o $@

$(TARGET_DIR)/kernel.elf: $(TARGET_DIR)/boot/kernel_entry.o $(OBJ)
	i386-elf-ld -o $@ -Ttext 0x1000 $^

