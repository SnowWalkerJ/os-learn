#include "port.h"
#include "screen.h"
#include "../kernel/memory.h"


// Declaration of private functions

unsigned int getCursorOffset();
void setCursorOffset(unsigned int);
int putChar(char, int, int, char);
unsigned int getOffset(int, int);
int getOffsetRow(unsigned int);
int getOffsetCol(unsigned int);
void setEmpty();
void handleScroll();

// Public API functions

void printAt(char* message, int row, int col) {
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
		offset = putChar(message[i++], row, col, WHITE_ON_BLACK);
		row = getOffsetRow(offset);
		col = getOffsetCol(offset);
		setCursorOffset(offset);
	}
}

void print(char* message) {
	printAt(message, -1, -1);
}

void clearScreen() {
	setEmpty(0, MAX_COLS*MAX_ROWS);
	setCursorOffset(0);
}

// Private functions

unsigned int getOffset(int row, int col) {
	return (row * MAX_COLS + col) * 2;
}

unsigned int getCursorOffset() {
	portByteOut(REG_SCREEN_CTRL, 14);
	unsigned int offset = (unsigned int)(portByteIn(REG_SCREEN_DATA) << 8);
	portByteOut(REG_SCREEN_CTRL, 15);
	offset += portByteIn(REG_SCREEN_DATA);
	return offset * 2;
}

void setCursorOffset(unsigned int offset) {
	offset /= 2;
	portByteOut(REG_SCREEN_CTRL, 14);
	portByteOut(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
	portByteOut(REG_SCREEN_CTRL, 15);
	portByteOut(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

int getOffsetRow(unsigned int offset) {
	return offset / 2 / MAX_COLS;
}

int getOffsetCol(unsigned int offset) {
	return (offset / 2) % MAX_COLS;
}

int putChar(char c, int row, int col, char format) {
	unsigned char* address = (unsigned char*)VIDEO_ADDRESS;
	if (!format) format = WHITE_ON_BLACK;

	//Error handling
	if (row >= MAX_ROWS || col >= MAX_COLS) {
		/*
		address[2*MAX_COLS*MAX_ROWS-2] = 'E';
		address[2*MAX_COLS*MAX_ROWS-1] = RED_ON_WHITE;
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
		address[offset + 1] = format;
		offset += 2;
	}
	return offset;
}

void setEmpty(unsigned int begin, unsigned int length) {
	unsigned char* addr = (unsigned char*)VIDEO_ADDRESS;
	for (int i = 0; i < length; i++) {
		addr[begin + i * 2] = ' ';
		addr[begin + i * 2 + 1] = WHITE_ON_BLACK;
	}
}

void handleScroll(){
	unsigned int offset = 2 * MAX_COLS;
	memcpy((unsigned char*)(VIDEO_ADDRESS+offset), (unsigned char*)VIDEO_ADDRESS, MAX_COLS*(MAX_ROWS-1)*2);
	offset = MAX_COLS*(MAX_ROWS-1)*2;
	setEmpty(offset, MAX_COLS);
	setCursorOffset(offset);
}

