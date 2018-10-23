#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 24
#define MAX_COLS 80

#define MAKE_STYLE(fore_color, back_color) (((back_color)<<4) | (fore_color))
#define WHITE 0xf
#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define GREEN2 0x3
#define RED   0x4
#define PURPLE 0x5
#define YELLOW 0x6
#define GRAY 0x7
#define WHITE_ON_BLACK MAKE_STYLE(WHITE, BLACK)
#define GREEN_ON_BLACK   MAKE_STYLE(GREEN, BLACK)
#define RED_ON_BLACK   MAKE_STYLE(RED, BLACK)

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#include<stdint.h>

void kprint(char*);
void kprint_hex(uint32_t);
void kprintAt(char*, int, int);
void backspace();
void clearScreen();
void set_style(uint8_t);
uint8_t get_style();

