#ifndef SCHED_H
#define SCHED_H

#include <kernel/console.h>
#include <kernel/defs.h>
#include <kernel/gdt.h>
#include <stdint.h>

enum task_status {
    TASK_RUNNING,
    TASK_INTERRUPTIBLE,
    TASK_UNINTERRUPTIBLE,
    TASK_ZOMBIE,
    TASK_STOPPED
};

struct i387_struct {
    uint32_t cwd;
    uint32_t swd;
    uint32_t twd;
    uint32_t fip;
    uint32_t fcs;
    uint32_t foo;
    uint32_t fos;
    uint32_t st_space[20]; /* 8*10 bytes for each FP-reg = 80 bytes */
};

struct tss_struct {
    uint32_t back_link; /* 16 high bits zero */
    uint32_t esp0;
    uint32_t ss0; /* 16 high bits zero */
    uint32_t esp1;
    uint32_t ss1; /* 16 high bits zero */
    uint32_t esp2;
    uint32_t ss2; /* 16 high bits zero */
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;           /* 16 high bits zero */
    uint32_t cs;           /* 16 high bits zero */
    uint32_t ss;           /* 16 high bits zero */
    uint32_t ds;           /* 16 high bits zero */
    uint32_t fs;           /* 16 high bits zero */
    uint32_t gs;           /* 16 high bits zero */
    uint32_t ldt;          /* 16 high bits zero */
    uint32_t trace_bitmap; /* bits: trace 0, bitmap 16-31 */
    struct i387_struct i387;
};

struct sigaction {
    uint32_t signal;
};

struct task_struct {
    /* these are hardcoded - don't touch */
    uint32_t state; /* -1 unrunnable, 0 runnable, >0 stopped */
    int counter;
    uint32_t priority;
    uint32_t signal;
    struct sigaction sigaction[32];
    uint32_t blocked; /* bitmap of masked signals */
                  /* various fields */
    int exit_code;
    uint32_t start_code, end_code, end_data, brk, start_stack;
    int pid, father, pgrp, session, leader;
    uint16_t uid, euid, suid;
    uint16_t gid, egid, sgid;
    uint32_t alarm;
    uint32_t utime, stime, cutime, cstime, start_time;
    uint16_t used_math;
    /* file system info */
    int tty; /* -1 if no tty, so it must be signed */
    uint16_t umask;
    struct m_inode *pwd;
    struct m_inode *root;
    struct m_inode *executable;
    uint32_t close_on_exec;
    struct file *filp[NR_OPEN];
    /* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
    gdt_gate_t ldt[3];
    /* tss for this task */
    struct tss_struct tss;
};

extern struct task_struct *task[NR_TASKS];
extern struct task_struct *current;
extern long volatile jiffies;
extern long startup_time;

#define CURRENT_TIME (startup_time + jiffies / HZ)

void add_timer(long jiffies, void (*fn)(void));
void sleep_on(struct task_struct **p);
void wake_up(struct task_struct **p);
void switch_to(int n);

#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY + 1)
#define _TSS(n) ((((uint32_t) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((uint32_t) n)<<4)+(FIRST_LDT_ENTRY<<3))
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

void init_sched();

#endif /* SCHED_H */