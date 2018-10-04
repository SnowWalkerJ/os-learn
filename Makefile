C_SOURCES = $(wildcard drivers/*.c kernel/*.c libs/*.c)
HEADERS = $(wildcard drivers/*.h kernel/*.h libs/*.h)
OBJ = $(C_SOURCES:.c=.o)
CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
	     -Wall -Wextra -Werror

all: run

run: os-image
	qemu-system-i386 -curses -fda $<

clean:
	rm *.bin os-image
	rm kernel/*.o drivers/*.o boot/*.bin

os-image: boot/boot.bin kernel.bin
	cat $^ > $@

boot/kernel_entry.o: boot/kernel_entry.asm
	nasm -f elf $< -o $@

kernel.bin: boot/kernel_entry.o $(OBJ)
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -ffreestanding -c $< -o $@

%.bin: %.asm
	nasm -f bin $< -o $@

