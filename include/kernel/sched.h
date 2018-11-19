#ifndef SCHED_H
#define SCHED_H

#define UNUSED(x) (void)(x)
#include <kernel/console.h>
void sleep_on(void* link) {
    panic("sleep_on: this shouldn't happen at all!");
    UNUSED(link);
}

void wake_up(void* link) {
    UNUSED(link);
}

#endif /* SCHED_H */