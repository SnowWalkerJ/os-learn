#ifndef SCHED_H
#define SCHED_H
#include <kernel/console.h>
void sleep_on(void*) {
    panic("sleep_on: this shouldn't happen at all!");
}

#endif /* SCHED_H */