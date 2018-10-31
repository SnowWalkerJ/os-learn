#include <kernel/console.h>
#include <drivers/screen.h>
#include <libs/stdio.h>
#include <stddef.h>
char* test_paging();
char* test_memcpy();

void run_one_test(char* func(), char* name, int fatal) {
    char* result = func();
    if (result != NULL) {
        printf("[%s]....\t", name);
        set_style(RED_ON_BLACK);
        kprint("failed");
        set_style(WHITE_ON_BLACK);
        printf(" (%s)\n", result);
        if (fatal) panic("");
    } else {
        printf("[%s]....\t", name);
        set_style(GREEN_ON_BLACK);
        kprint("passed");
        set_style(WHITE_ON_BLACK);
        kprint("\n");
    }
}

char* test_fake_error() {
    // This test fails intentionally
    return "Error as expected";
}

void run_tests() {
    run_one_test(test_paging, "Memory Paging", 1);
    run_one_test(test_memcpy, "Memcpy", 1);
}