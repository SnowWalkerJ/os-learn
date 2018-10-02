C_SOURCES = $(wildcard drivers/*.c kernel/*.c)
HEADERS = $(wildcard drivers/*.h kernel/*.c)
OBJ = $(C_SOURCES:.c=.o)

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
	i386-elf-gcc -ffreestanding -c $< -o $@

%.bin: %.asm
	nasm -f bin $< -o $@

