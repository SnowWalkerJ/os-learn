#include <kernel/console.h>
#include <drivers/screen.h>
#include <libs/stdio.h>
#include <stddef.h>
char* test_paging();
char* test_memcpy();


// FIXME: string constants not properly initialized

static void run_one_test(char* (*func)(), char* name, int fatal) {
    char* result = (*func)();
    if (result != NULL) {
        printf("[%s]....", name);
        set_style(RED_ON_BLACK);
        kprint("failed");
        set_style(WHITE_ON_BLACK);
        printf(" (%s)\n", result);
        if (fatal) panic("");
    } else {
        printf("[%s]....", name);
        set_style(GREEN_ON_BLACK);
        kprint("passed\n");
        set_style(WHITE_ON_BLACK);
    }
}


void run_tests() {
    run_one_test(&test_paging, "Memory Paging", 1);
    run_one_test(&test_memcpy, "Memcpy", 1);
}