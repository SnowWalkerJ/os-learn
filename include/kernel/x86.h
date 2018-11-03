#ifndef X86_H
#define X86_H

#include<stdint.h>

static inline uint32_t read_eflags(void) {
    uint32_t eflags;
    __asm__ __volatile__("pushfl; popl %0" : "=r" (eflags));
    return eflags;
}

static inline void write_eflags(uint32_t eflags) {
    asm volatile("pushl %0; popfl" : : "r" (eflags));
}

static inline uint32_t xchg(volatile uint32_t* addr, uint32_t new_val) {
    /* atomic execute read -> modify -> write */
    uint32_t result;
    asm volatile("lock"
        "xchgl %0, %1"
        : "+m" (*addr)
        , "=a" (result)
        : "1" (new_val)
        : "cc");
    return result;
}

static inline void cli(void) {
    asm volatile("cli");
}

static inline void sti(void) {
    asm volatile("sti");
}

#endif   /* X86_H */
