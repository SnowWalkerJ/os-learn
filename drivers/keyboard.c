#include <drivers/keyboard.h>
#include <drivers/ps2.h>
#include <kernel/port.h>
#include <kernel/isr.h>
#include <drivers/screen.h>
#define UNUSED(x) (void)(x)

#define BACKSPACE 0x0E
#define ENTER 0x1C

#define BUFFER_SIZE 256

static char input_mask = 0;
static char key_buffer[BUFFER_SIZE];
static unsigned short key_buffer_cursor = 0;

#define SC_MAX 57
const char *sc_name[] = { "ERROR", "Esc", "1", "2", "3", "4", "5", "6", 
    "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E", 
	"R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl", 
	"A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", 
	"LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", 
    "/", "RShift", "Keypad *", "LAlt", "Spacebar"};
const char sc_ascii[] = { '?', 0, '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y', 
    'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 
    'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '};

void append(char key) {
	if (key_buffer_cursor >= BUFFER_SIZE) return;
	key_buffer[key_buffer_cursor++] = key;
}

void show(char key) {
	char data[2] = {key, 0};
	if (input_mask != 0) {
		data[0] = input_mask;
	}
	kprint(data);
}

void keyboard_callback(registers_t* regs) {
	uint8_t scan_code = inb(PS2_DATA);
	if (scan_code > SC_MAX) return;
	if (scan_code == BACKSPACE) {
		if (key_buffer_cursor > 0) {
			backspace();
			key_buffer_cursor--;
		}
	} else {
		char key = sc_ascii[scan_code];
		append(key);
		show(key);
	}
	UNUSED(regs);
}

void set_input_mask(char mask) {
	input_mask = mask;
}
