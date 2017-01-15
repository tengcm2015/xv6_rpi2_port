// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "arm.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

void initlock(struct spinlock *lk, char *name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

// For single CPU systems, there is no need for spinlock.
// Add the support when multi-processor is supported.


// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void acquire(struct spinlock *lk)
{
    pushcli();		// disable interrupts to avoid deadlock.

    // if(holding(lk)) {
    //     // cprintf("acquire: cpu %d already holding\n", get_cpunum());
    //     // return;
    //     panic("acquire: already holding");
    // }

#if 1
    // arch_spin_lock() from linux kernel
    uint tmp;

    asm (
    "1: LDREX   %0, [%1]        \n" /* exclusively load lock into %0, i.e. tmp */
    "   TEQ     %0, #0          \n" /* test equality against value 0 */
    "   WFENE                   \n" /* wait for event if negative. */
    "   STREXEQ %0, %2, [%1]    \n" /* exclusively store 1 into %0 of lock */
    "   TEQEQ   %0, #0          \n" /* test if store succeeded. test equality of %0, i.e. tmp against 0 value */
    "   BNE     1b                " /* branch on negative to 1 to try again */
    : "=&r" (tmp)                   /* tmp: output, referred to by %0; r: use register to store*/
    : "r" (&lk->locked), "r" (1)    /* inputs, %1, %2 */
    : "cc");                        /* clobbered register cc (condition code) will be modified */

    // uint newval;
    // struct spinlock lockval;
    //
    // asm (
    // "1: LDREX   %0, [%3]        \n" /* exclusively load lk->locked into %0, i.e. lockval.locked */
    // "   ADD     %1, %0, %4      \n" /* store lk->tickets.next + 1 to newval */
    // "   STREX   %2, %1, [%3]    \n" /* exclusively store newval into lockval.locked */
    // "   TEQ     %2, #0          \n" /* test if store succeeded. */
    // "   BNE     1b                " /* branch on negative to 1 to try again */
    // : "=&r" (lockval.locked), "=&r" (newval), "=&r" (tmp)   /* %0, %1, %2 */
    // : "r" (&lk->locked), "I" (1 << 16)    /* inputs, %3, %4 */
    // : "cc");                        /* clobbered register cc (condition code) will be modified */
    //
    // while (lockval.tickets.next != lockval.tickets.owner) {
    //     asm("WFE" ::: "memory");
    //     lockval.tickets.owner = lk->tickets.owner;
    // }
#else
    lk->locked = 1;	// set the lock status to make the kernel happy
#endif

    // Record info about lock acquisition for debugging.
    lk->cpu = curr_cpu;
    // getcallerpcs(get_fp(), lk->pcs);
    asm("DMB" ::: "memory");        /* SMP memory barrier to protect the &lk->locked */

    // if(lk->name[0] == 'p')
    //     cprintf("cpu%d acquired lock: %s\n", get_cpunum(), lk->name);
}

// Release the lock.
void release(struct spinlock *lk)
{
    asm("DMB" ::: "memory");        /* SMP memory barrier to protect the &lk->locked */

    // if(!holding(lk)) {
    //     // cprintf("release: cpu %d not holding\n", get_cpunum());
    //     // popcli();
    //     // return;
    //     panic("release: not holding");
    // }

    lk->pcs[0] = 0;
    lk->cpu = 0;

#if 1
    // arch_spin_unlock() from linux kernel
    // asm(
    // "   str %1, [%0]\n"             /* store 0 into &lock->lock, i.e. unlock */
    // :                               /* output: none */
    // : "r" (&lk->locked), "r" (0)    /* input %0, %1 */
    // : "cc");                        /* clobbered register cc (condition code) will be set/modified */

    lk->locked = 0;
    // lk->tickets.owner++;

    asm("DSB" ::: "memory");        /* Data Synchronization Barrier */
    asm("SEV" ::: "memory");        /* Send event */
    // asm("NOP");                     /* No-op */

#else
    lk->locked = 0; // set the lock state to keep the kernel happy
#endif
    // if(lk->name[0] == 'p')
    //     cprintf("cpu%d released lock: %s\n", get_cpunum(), lk->name);

    popcli();
}


// Check whether this cpu is holding the lock.
int holding(struct spinlock *lock)
{
    // return (lock->tickets.next != lock->tickets.owner) && lock->cpu == curr_cpu;
    return lock->locked && lock->cpu == curr_cpu;
}
