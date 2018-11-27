#include <stdint.h>

struct registers {
    uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
};

struct task {
    unsigned int id;
    unsigned int priority;
    struct registers regs;
    struct task *next;
};