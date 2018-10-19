#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 24
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#include<stdint.h>

void kprint(char*);
void kprint_hex(uint32_t);
void kprintAt(char*, int, int);
void backspace();
void clearScreen();


