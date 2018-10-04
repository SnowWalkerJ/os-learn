#include <stdint.h>
#include "../drivers/screen.h"
#include "memory.h"
#include "../libs/string.h"
#define UNUSED(x) (void)(x)
void main(){
	initMemTable();
	clearScreen();
	print("Kernel entered.\n");
	char* x = (char*)malloc(10);
	char* tmp = int_to_hex((uint32_t)x);
	print(tmp);
	free(tmp);
	x[0] = 'a';
	x[1] = '\n';
	x[2] = 'b';
	x[3] = '\n';
	x[4] = 'c';
	x[5] = 0;
	print(x);
	int t = free(x);
	if (t != 0) {
		print("Error");
	}
}
