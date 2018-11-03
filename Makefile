C_SOURCES = $(wildcard drivers/*.c kernel/*.c libs/*.c cpu/*.c tests/*.c)
S_SOURCES = $(wildcard cpu/*.asm kernel/*.asm)
HEADERS = $(wildcard drivers/*.h kernel/*.h libs/*.h cpu/*.h tests/*.h)
OBJ = $(C_SOURCES:.c=.o) $(S_SOURCES:.asm=.o)
GDB = gdb
CC = i386-elf-gcc
CFLAGS = -g -m32 -fno-builtin -ffreestanding -fno-stack-protector -nostartfiles -nodefaultlibs \
	     -Wall -Wextra -Werror -fno-exceptions -Iinclude

.PHONY: all clean run debug
all: run

run: os-image
	qemu-system-i386 -curses -fda $<

debug: os-image kernel.elf
	# qemu-system-i386 -s -S -curses -fda os-image -d guest_errors,int & $(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"
	qemu-system-i386 -s -S -curses -fda os-image -d guest_errors,int

clean:
	rm $(OBJ)
	rm boot/*.bin boot/*.o
	rm *.bin os-image
	rm kernel.elf

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

