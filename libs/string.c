#include "../kernel/memory.h"

char* int_to_hex(uint32_t value) {
	char *x = (char*)malloc(9*sizeof(char));
	memcpy("00000000", x, 9*sizeof(char));
	unsigned char* v = (unsigned char*)&value;
	for (int i = 0; i < 4; i++) {
		char c = v[i];
		char h = ((c>>4) & 0xf) + '0';
		if (h > '9') h += 'A'-'9'-1;
		char l = (c & 0xf) + '0';
		if (l > '9') l += 'A'-'9'-1;
		*(x+2*(3-i)) = h;
		*(x+2*(3-i)+1) = l;
	}
	return x;
}

unsigned int strlen(char* string) {
	unsigned int i;
	for (i = 0; string[i] != 0; i++);
	return i;
}

