#include <drivers/screen.h>
int panicked = 0;

void panic(char *info) {
    kprint(info);
    panicked = 1;
    while (1)
        ;
}
