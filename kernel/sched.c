#include <kernel/defs.h>
#include <kernel/isr.h>
#include <kernel/kmemory.h>
#include <kernel/port.h>
#include <kernel/sched.h>
#include <kernel/x86.h>
#include <libs/stdlib.h>
#include <stdint.h>

struct timer {
    long jiffies;
    void (*fn)();
    struct timer *next;
} timer_list[NR_TIMERS], *next_timer = NULL;

union task_union {
    struct task_struct task;
    char stack[PAGE_SIZE];
};

extern uint32_t *page_directory;

#define INIT_TASK                                                              \
    {                                                                          \
        /* state */ 0, 15, 15, 0, /* sigaction */                              \
            {                                                                  \
                {},                                                            \
            },                                                                 \
            0, 0, /* start_code */ 0, 0, 0, 0, 0, /* pid */ 0, -1, 0, 0, 0,    \
            /* uid */ 0, 0, 0, 0, 0, 0, /* alarm */ 0, 0, 0, 0, 0, 0,          \
            /* math */ 0, /* fs info */ -1, 0022, NULL, NULL, NULL,            \
            0, /* flip */                                                      \
            {                                                                  \
                NULL,                                                          \
            }, /* ldt */                                                       \
            {                                                                  \
                {0, 0, 0, 0, 0, 0},                                            \
                {0xffff, 0, 0, make_access(1, 3, 1, 0, 1), 0b11001111, 0},     \
                {0xffff, 0, 0, make_access(1, 3, 0, 0, 1), 0b11001111, 0},     \
            },                                                                 \
            /* tss */ {0,       PAGE_SIZE + (uint32_t)&init_task,              \
                       0x10,    0,                                             \
                       0,       0,                                             \
                       0,       0,                                             \
                       0,       0,                                             \
                       0,       0,                                             \
                       0,       0,                                             \
                       0,       0,                                             \
                       0,       0,                                             \
                       0x17,    0x17,                                          \
                       0x17,    0x17,                                          \
                       0x17,    0x17,                                          \
                       _LDT(0), 0x80000000,                                    \
                       {}},                                                    \
    }

// This causes a warning on x86_64 platform because addresses take 64 bits
// and cannot be contained by uint32_t. Just ignore it.
static union task_union init_task = {
    INIT_TASK,
};
volatile long jiffies = 0;
long startup_time;
struct task_struct *current        = &(init_task.task);
struct task_struct *task[NR_TASKS] = {
    &(init_task.task),
};

struct timer timer_list[NR_TIMERS];

uint32_t user_stack[PAGE_SIZE >> 2];

static void schedule(void);

static inline void _set_tssldt_descriptor(int n, uint32_t addr, uint8_t type) {
    gdt_gate_t *descriptor = &gdt_gates[n];
    descriptor->base0_15   = (uint16_t)(addr & 0xffff);
    descriptor->base16_23  = (uint8_t)((addr >> 16) & 0xff);
    descriptor->base24_31  = (uint8_t)((addr >> 24) & 0xff);
    descriptor->access     = type;
    descriptor->flags      = 0;
    descriptor->limit      = 104;
}

#define set_tss_descriptor(n, addr) (_set_tssldt_descriptor(_TSS(n), (uint32_t)addr, 0x89))
#define set_ldt_descriptor(n, addr) (_set_tssldt_descriptor(_LDT(n), (uint32_t)addr, 0x82))

void add_timer(long jiffies, void (*fn)(void)) {
    struct timer *p;
    if (!fn)
        return;
    cli();

    if (jiffies <= 0) {
        (fn)();
    } else {
        // Find one free timer struct
        for (p = timer_list; p < timer_list + NR_TIMERS; p++) {
            if (!p->fn) {
                break;
            }
        }
        // If not find free timer
        if (p >= timer_list + NR_TIMERS)
            panic("Out of timers");

        // Else
        p->jiffies = jiffies;
        p->fn      = fn;
        p->next    = next_timer;
        next_timer = p;

        if (p->next && p->next->jiffies >= p->jiffies) {
            p->next->jiffies -= p->jiffies;
        } else {
            while (p->next && p->next->jiffies < p->jiffies) {
                // Swap p and p->next
                p->fn            = p->next->fn;
                p->next->fn      = fn;
                jiffies          = p->jiffies;
                p->jiffies       = p->next->jiffies;
                p->next->jiffies = jiffies - p->jiffies;
                p                = p->next;
            }
        }
    }
    sti();
}

static void do_timer(int cpl) {
    if (cpl) {
        current->utime++;
    } else {
        current->stime++;
    }
    if (next_timer) {
        next_timer->jiffies--;
        while (next_timer && next_timer->jiffies <= 0) {
            void (*fn)(void);
            fn             = next_timer->fn;
            next_timer->fn = NULL;
            next_timer     = next_timer->next;
            (fn)();
        }
    }
    if ((--current->counter) > 0)
        return;
    current->counter = 0;
    if (!cpl)
        return;
    schedule();
}

static void timer_callback(registers_t *regs) {
    jiffies++;
    int cpl = regs->cs & 3; // Priviledge level
    do_timer(cpl);
}

static void init_timer() {
    uint32_t freq = CLOCK / HZ;
    uint8_t low   = (uint8_t)(freq & 0xFF);
    uint8_t high  = (uint8_t)((freq >> 8) & 0xFF);
    register_interrupt_handler(IRQ0, &timer_callback);
    outb(0x43, 0x36);
    outb(0x40, low);
    outb(0x40, high);
}

void init_sched() {
    // Clear all TSS and LDT descriptors
    memsetb(gdt_gates + FIRST_TSS_ENTRY, 0, sizeof(gdt_gate_t) * 2 * NR_TASKS);
    // Set TSS and LDT for task 0
    set_tss_descriptor(0, &(init_task.task.tss));
    set_ldt_descriptor(0, &(init_task.task.ldt));
    init_timer();
}

inline int sys_getpid(void) { return current->pid; }

inline int sys_getppid(void) { return current->father; }

inline int sys_getuid(void) { return current->uid; }

inline int sys_geteuid(void) { return current->euid; }

inline int sys_getgid(void) { return current->gid; }

inline int sys_getegid(void) { return current->egid; }

inline int sys_nice(long increment) {
    if (current->priority - increment > 0)
        current->priority -= increment;
    return 0;
}

static void schedule(void) {
    int c, i, next;
    while (1) {
        c    = -1;
        next = 0;
        for (i = 0; i < NR_TASKS; i++) {
            struct task_struct *p = task[i];
            if (p && p->state == TASK_RUNNING && p->counter > c) {
                next = i;
                c    = p->counter;
            }
        }
        if (c >= 0)
            break; // Found a runnable task

        for (i = 0; i < NR_TASKS; i++) {
            struct task_struct *p = task[i];
            if (p)
                p->counter = (p->counter >> 1) + (p->priority);
        }
    }
    switch_to(next);
}

void switch_to(int n) {
    struct task_struct *p = task[n];
    if (p == current)
        return;
    uint32_t tmp[2];
    tmp[1] = _TSS(n);
    asm("xchgl %%ecx, current;"
        "ljmp *%0;" ::"m"(tmp),
        "c"((uint32_t)p));
}

void wake_up(struct task_struct **p) {
    if (p && *p) {
        (*p)->state = TASK_RUNNING;
        *p          = NULL;
    }
}

void sleep_on(struct task_struct **p) {
    if (!p)
        return;
    if (current == &(init_task.task))
        panic("task 0 trying to sleep");
    struct task_struct *tmp;
    tmp            = *p;
    *p             = current;
    current->state = TASK_UNINTERRUPTIBLE;
    schedule();
    wake_up(&tmp);
}