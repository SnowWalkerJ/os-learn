#ifndef DEFS_H
#define DEFS_H

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

#define UNUSED(x) ((void)x)

#define NR_OPEN 32 // Maximum number of open files each process
#define NR_TASKS 64 // Maximum number of tasks the system can hold
#define NR_TIMERS 32 // Maximum number of timers

#define CLOCK 1193180

#define HZ 100 // Time of clock

#endif /* DEFS_H */