// The System Timer peripheral

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"
#include "traps.h"
#include "arm.h"
#include "spinlock.h"

#include "rpi_systimer.h"
#include "rpi_interrupts.h"

#define TIMER_FREQ		10000  // interrupt 100 times/sec.

struct spinlock tickslock;
uint ticks;

static rpi_sys_timer_t* rpiSystemTimer = (rpi_sys_timer_t*)RPI_SYSTIMER_BASE;

rpi_sys_timer_t* RPI_GetSystemTimer(void)
{
    return rpiSystemTimer;
}

void
timer3init(void)
{
    uint v;

    // enabletimer3irq();
    RPI_GetIrqController()->Enable_IRQs_1 |= 1 << IRQ_TIMER3;

    v = rpiSystemTimer->counter_lo;
    rpiSystemTimer->compare3 = v + TIMER_FREQ;
    ticks = 0;
}

void
timer3intr(void)
{
    uint v;
    //cprintf("timer3 interrupt: %x\n", rpiSystemTimer->control_status);
    rpiSystemTimer->control_status = 1 << IRQ_TIMER3; // clear timer3 irq
    ticks++;
    wakeup(&ticks);

    // reset the value of compare3
    v = rpiSystemTimer->counter_lo;
    rpiSystemTimer->compare3 = v + TIMER_FREQ;
}

void
delay(uint m)
{
    volatile uint32 ts = rpiSystemTimer->counter_lo;

    while( ( rpiSystemTimer->counter_lo - ts ) < m )
    {
        /* BLANK */
    }
}
