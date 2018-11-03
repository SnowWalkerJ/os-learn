#include <kernel/spinlock.h>
#include <kernel/x86.h>
#ifdef NOT_COMPLETE

/* private functions declared here */
static int holding(struct spinlock*);


/* public functions declared here */
void init_spinlock(struct spinlock* lock, char* name) {
    lock->locked = 0;
    lock->name = name;
    lock->cpu = 0xFFFFFFFF;
}

void acquire_spinlock(struct spinlock* lock) {
    cli(); // pushcli
    if (holding(lock)) panic("Already holding the spinlock");

    // wait till the lock is acquired
    while (xchg((uint32_t*)&lock->locked, 1) == 1) ;

    lock->cpu = cpu();
}

void release_spinlock(struct spinlock* lock) {
    if (!holding(lock)) panic("Releasing a lock that's not held");
    
    lock->cpu = 0xFFFFFF;
    xchg((uint32_t*)&lock->locked, 0);
    sti();    // popcli
}

/* private functions defined here */
static int holding(struct spinlock* lock) {
    return lock->locked && lock->cpu == cpu();
}

#endif
