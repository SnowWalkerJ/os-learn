#include <stddef.h>
#include <kernel/port.h>
#include <kernel/memory.h>
#include <libs/stdlib.h>
#include <libs/string.h>
#include <drivers/screen.h>


// Declaration of private functions

static unsigned int getCursorOffset();
static void setCursorOffset(unsigned int);
static int putCharAt(char, int, int, char);
static inline unsigned int getOffset(int, int);
static inline int getOffsetRow(unsigned int);
static inline int getOffsetCol(unsigned int);
static void setEmpty();
static void handleScroll();

static uint8_t current_style = WHITE_ON_BLACK;

// Public API functions

void kprintAt(const char* message, int row, int col) {
	unsigned int offset;
	if (row >= 0 && col >= 0) {
		offset = getOffset(row, col);
	} else {
		offset = getCursorOffset(row, col);
		row = getOffsetRow(offset);
		col = getOffsetCol(offset);
	}
	int i = 0;
	while (message[i] != 0) {
		offset = putCharAt(message[i++], row, col, current_style);
		row = getOffsetRow(offset);
		col = getOffsetCol(offset);
	}
}

void kprint(const char* message) {
	kprintAt(message, -1, -1);
}

void kprint_hex(uint32_t value) {
    char* hex = int_to_hex(value);
    kprint(hex);
    free(hex);
}

void kput_char(char ch) {
	putCharAt(ch, -1, -1, current_style);
}

void clearScreen() {
	setEmpty(0, MAX_COLS*MAX_ROWS);
	setCursorOffset(0);
}

void set_style(uint8_t style) {
	current_style = style;
}

// Private functions

static inline unsigned int getOffset(int row, int col) {
	return (row * MAX_COLS + col) * 2;
}

static unsigned int getCursorOffset() {
	outb(REG_SCREEN_CTRL, 14);
	unsigned int offset = (unsigned int)(inb(REG_SCREEN_DATA) << 8);
	outb(REG_SCREEN_CTRL, 15);
	offset += inb(REG_SCREEN_DATA);
	return offset * 2;
}

static void setCursorOffset(unsigned int offset) {
	offset /= 2;
	outb(REG_SCREEN_CTRL, 14);
	outb(REG_SCREEN_DATA, (unsigned char)((offset >> 8) & 0xff));
	outb(REG_SCREEN_CTRL, 15);
	outb(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

static inline int getOffsetRow(unsigned int offset) {
	return offset / 2 / MAX_COLS;
}

static inline int getOffsetCol(unsigned int offset) {
	return (offset / 2) % MAX_COLS;
}

static int putCharAt(char c, int row, int col, char style) {
	unsigned char* address = (unsigned char*)VIDEO_ADDRESS;
	if (!style) style = WHITE_ON_BLACK;

	//Error handling
	if (row >= MAX_ROWS || col >= MAX_COLS) {
		/*
		address[2*MAX_COLS*MAX_ROWS-2] = 'E';
		address[2*MAX_COLS*MAX_ROWS-1] = RED_ON_BLACK;
		return getOffset(row, col);
		*/
		handleScroll();
		row = MAX_ROWS - 1;
		col = 0;
	}

	unsigned int offset;
	if (row >= 0 && col >= 0) {
		offset = getOffset(row, col);
	} else {
		offset = getCursorOffset();
	}
	if (c == '\n') {
		int row = getOffsetRow(offset);
		offset = getOffset(row + 1, 0);
	} else {
		address[offset] = c;
		address[offset + 1] = style;
		offset += 2;
	}
	setCursorOffset(offset);
	return offset;
}

static void setEmpty(unsigned int begin, size_t length) {
	unsigned char* addr = (unsigned char*)VIDEO_ADDRESS;
	char tmp[2] = {' ', WHITE_ON_BLACK};
	memsetw(addr+begin, *(uint16_t*)tmp, length);
}

static void handleScroll(){
	unsigned int offset = 2 * MAX_COLS;
	memcpy((unsigned char*)VIDEO_ADDRESS, (unsigned char*)(VIDEO_ADDRESS+offset), MAX_COLS*(MAX_ROWS-1)*2);
	offset = MAX_COLS*(MAX_ROWS-1)*2;
	setEmpty(offset, MAX_COLS);
	setCursorOffset(offset);
}

void backspace() {
	int offset = getCursorOffset() - 2;
	setCursorOffset(offset);
	*(char*)(VIDEO_ADDRESS+offset) = ' ';
}
