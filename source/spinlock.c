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

    // well this is a very shoddy lock ...
    if(holding(lk)) {
        // cprintf("acquire: cpu %d already holding\n", get_cpunum());
        return;
        // panic("acquire: already holding");
    }

#if 1
    // The xchg is atomic.
    // It also serializes, so that reads after acquire are not
    // reordered before it.
    // while(xchg(&lk->locked, 1) != 0)
    //     ;

    // arch_spin_lock() from linux kernel
    uint tmp;

    asm (
    "1: LDREX   %0, [%1]        \n" /* exclusively load lock into %0, i.e. tmp */
    "   TEQ     %0, #0          \n" /* test equality against value 0 */
    "   WFENE                   \n" /* wait for event if negative. special care for Thumb-2 WFE instructions */
    "   STREXEQ %0, %2, [%1]    \n" /* exclusively store 1 into %0 of lock */
    "   TEQEQ   %0, #0          \n" /* test if store succeeded. test equality of %0, i.e. tmp against 0 value */
    "   BNE     1b                " /* branch on negative to 1 to try again */
    : "=&r" (tmp)                   /* tmp: output, referred to by %0; r: use register to store*/
    : "r" (&lk->locked), "r" (1)    /* inputs, %1, %2 */
    : "cc", "memory");              /* clobbered register cc (condition code) will be modified */

#else
    lk->locked = 1;	// set the lock status to make the kernel happy
#endif

    // Record info about lock acquisition for debugging.
    lk->cpu = curr_cpu;
    // getcallerpcs(get_fp(), lk->pcs);
    asm("DMB" ::: "memory");        /* SMP memory barrier to protect the &lk->locked */

}

// Release the lock.
void release(struct spinlock *lk)
{
    asm("DMB" ::: "memory");        /* SMP memory barrier to protect the &lk->locked */

    // well this is a very shoddy lock ...
    if(!holding(lk)) {
        // cprintf("release: cpu %d not holding\n", get_cpunum());
        popcli();
        return;
        // panic(lk->name);
    }

    lk->pcs[0] = 0;
    lk->cpu = curr_cpu;

#if 1
    // The xchg serializes, so that reads before release are
    // not reordered after it.  The 1996 PentiumPro manual (Volume 3,
    // 7.2) says reads can be carried out speculatively and in
    // any order, which implies we need to serialize here.
    // But the 2007 Intel 64 Architecture Memory Ordering White
    // Paper says that Intel 64 and IA-32 will not move a load
    // after a store. So lock->locked = 0 would work here.
    // The xchg being asm volatile ensures gcc emits it after
    // the above assignments (and after the critical section).
    // xchg(&lk->locked, 0);

    // arch_spin_unlock() from linux kernel
    lk->locked = 0; // set the lock state to keep the kernel happy
    // asm(
    // "   str %1, [%0]\n"             /* store 0 into &lock->lock, i.e. unlock */
    // :                               /* output: none */
    // : "r" (&lk->locked), "r" (0)    /* input %0, %1 */
    // : "cc");                        /* clobbered register cc (condition code) will be set/modified */

    asm("DSB" ::: "memory");        /* Data Synchronization Barrier */
    asm("SEV" ::: "cc");            /* Send event */

#else
    lk->locked = 0; // set the lock state to keep the kernel happy
#endif

    popcli();
}


// Check whether this cpu is holding the lock.
int holding(struct spinlock *lock)
{
    return lock->locked && lock->cpu == curr_cpu;
}
