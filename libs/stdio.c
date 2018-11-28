#include <drivers/screen.h>
#include <kernel/memory.h>
#include <libs/stdio.h>
#include <libs/string.h>
#include <stdarg.h>

void printf(const char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    int i          = 0;
    int state      = 0;
    int fmt_length = 0;
    char ch;
    char c;
    int num;
    unsigned int hex;
    char *str;
    while ((ch = fmt[i++]) != 0) {
        if (state == 0) {
            if (ch == '%') {
                state = 1;
            } else {
                kput_char(ch);
            }
        } else {
            switch (ch) {
            case 's':
                str = va_arg(arg, char *);
                kprint(str);
                state = 0;
                break;
            case 'c':
                c = (char)va_arg(arg, int);
                kput_char(c);
                fmt_length = 0;
                break;
            case 'd':
                num       = va_arg(arg, int);
                char *oct = itoa(num, fmt_length);
                kprint(oct);
                free(oct);
                fmt_length = 0;
                state      = 0;
                break;
            case 'x':
                hex = va_arg(arg, unsigned int);
                kprint_hex(hex);
                state = 0;
                break;
            default:
                if (ch >= '0' && ch <= '9') {
                    fmt_length = fmt_length * 10 + ch - '0';
                }
            }
        }
    }
    va_end(arg);
}
