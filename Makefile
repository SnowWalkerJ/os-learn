C_SOURCES = $(wildcard drivers/*.c kernel/*.c libs/*.c cpu/*.c)
S_SOURCES = $(wildcard cpu/*.asm kernel/*.asm)
HEADERS = $(wildcard drivers/*.h kernel/*.h libs/*.h cpu/*.h)
OBJ = $(C_SOURCES:.c=.o) $(S_SOURCES:.asm=.o)
GDB = /usr/local/i386elfgcc/bin/i386-elf-gdb
CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
CFLAGS = -m32 -fno-builtin -ffreestanding -fno-stack-protector -nostartfiles -nodefaultlibs \
	     -Wall -Wextra -Werror -fno-exceptions -I.

.PHONY: all clean run
all: run

run: os-image
	qemu-system-i386 -curses -fda $<

debug: os-image kernel.elf
	qemu-system-i386 -s -curses -fda os-image -d guest_errors,int & $(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

clean:
	rm $(OBJ)
	rm boot/*.bin boot/*.o
	rm *.bin os-image

os-image: boot/boot.bin kernel.bin
	cat $^ > $@

boot/kernel_entry.o: boot/kernel_entry.asm
	nasm -f elf $< -o $@

kernel.bin: boot/kernel_entry.o $(OBJ)
	i386-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm -f bin $< -o $@

kernel.elf: boot/kernel_entry.o $(OBJ)
	i386-elf-ld -o $@ -Ttext 0x1000 $^

