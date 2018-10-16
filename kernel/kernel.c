#include <stdint.h>
#include "../drivers/screen.h"
#include "memory.h"
#include "../libs/string.h"
#include "../drivers/keyboard.h"
#include "../cpu/isr.h"
#define UNUSED(x) (void)(x)
extern void isr_install();


void main(){
	initMemTable();
	clearScreen();
	print("Kernel entered.\n");
	isr_install();
	__asm__ __volatile__("sti");
	init_keyboard();
	print("MyOS> ");
}
