#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 24
#define MAX_COLS 80

#define VGA_MAKE_STYLE(fore_color, back_color) (((back_color)<<4) | (fore_color))
#define VGA_BLACK 0x0
#define VGA_BLUE 0x1
#define VGA_GREEN 0x2
#define VGA_CYAN 0x3
#define VGA_RED   0x4
#define VGA_MAGENTA 0x5
#define VGA_BROWN 0x6
#define VGA_LIGHTGRAY 0x7

#define VGA_LIGHT(color) ((color)+8)

#define VGA_DARKGRAY VGA_LIGHT(VGA_BLACK)
#define VGA_LIGHTBLUE VGA_LIGHT(VGA_BLUE)
#define VGA_LIGHTGREEN VGA_LIGHT(VGA_GREEN)
#define VGA_LIGHTCYAN VGA_LIGHT(VGA_CYAN)
#define VGA_LIGHTRED VGA_LIGHT(VGA_RED)
#define VGA_LIGHTMAGENTA VGA_LIGHT(VGA_MAGENTA)
#define VGA_LIGHTBROWN VGA_LIGHT(VGA_BROWN)
#define VGA_WHITE VGA_LIGHT(VGA_LIGHTGRAY)

#define WHITE_ON_BLACK VGA_MAKE_STYLE(VGA_WHITE, VGA_BLACK)
#define GREEN_ON_BLACK VGA_MAKE_STYLE(VGA_GREEN, VGA_BLACK)
#define RED_ON_BLACK   VGA_MAKE_STYLE(VGA_RED, VGA_BLACK)

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#include<stdint.h>

void kprint(const char*);
void kprint_hex(uint32_t);
void kprintAt(const char*, int, int);
void kput_char(char);
void backspace();
void clearScreen();
void set_style(uint8_t);
uint8_t get_style();

