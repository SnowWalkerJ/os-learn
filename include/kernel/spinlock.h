#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <stdint.h>

struct spinlock {
    char *name;
    uint32_t locked;
    int cpu;
};

void init_spinlock(struct spinlock *, char *);
void acquire_spinlock(struct spinlock *);
void release_spinlock(struct spinlock *);

#endif /* SPINLOCK_H */
