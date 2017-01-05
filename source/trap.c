// The ARM UART, a memory mapped device
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "arm.h"
#include "traps.h"

/** @brief The BCM2835 Interupt controller peripheral at it's base address */
static rpi_irq_controller_t* rpiIRQController =
        (rpi_irq_controller_t*)RPI_INTERRUPT_CONTROLLER_BASE;

/**
    @brief Return the IRQ Controller register set
*/
rpi_irq_controller_t* RPI_GetIrqController( void )
{
    return rpiIRQController;
}

// void enable_intrs(void)
// {
//     rpiIRQController->Enable_IRQs_1 |= 1 << 29;     // enable the miniuart through Aux
//     // rpiIRQController->Enable_IRQs_2 |= 1 << 25;     // enable uart
//     rpiIRQController->Enable_Basic_IRQs |= 1 << 0;  // enable the system timer
// }
//
// void disable_intrs(void)
// {
//     int disable = ~0;
//     rpiIRQController->Enable_IRQs_1 = disable;
//     rpiIRQController->Enable_IRQs_2 = disable;
//     rpiIRQController->Enable_Basic_IRQs = disable;
//     rpiIRQController->FIQ_control = 0;
// }

// trap routine
void swi_handler (struct trapframe *r)
{
    curr_proc->tf = r;
    syscall ();
}

// trap routine
void irq_handler (struct trapframe *r)
{
    // curr_proc points to the current process. If the kernel is
    // running scheduler, curr_proc is NULL.
    if (curr_proc != NULL) {
        curr_proc->tf = r;
    }

    while ( rpiIRQController->IRQ_basic_pending
         || rpiIRQController->IRQ_pending_1
         || rpiIRQController->IRQ_pending_2
          )
    {
        if(rpiIRQController->IRQ_pending_1 & (1 << IRQ_TIMER3)) {
            timer3intr();
        }
        if(rpiIRQController->IRQ_pending_1 & (1 << IRQ_MINIUART)) {
            miniuartintr();
        }
    }
}

// trap routine
void reset_handler (struct trapframe *r)
{
    cli();
    cprintf ("reset at: 0x%x \n", r->pc);
}

// trap routine
void und_handler (struct trapframe *r)
{
    cli();
    cprintf ("und at: 0x%x \n", r->pc);
}

// trap routine
void dabort_handler (struct trapframe *r)
{
    uint dfs, fa;
    extern void show_callstk (char *s);
    cli();

    // read data fault status register
    asm("MRC p15, 0, %[r], c5, c0, 0": [r]"=r" (dfs)::);

    // read the fault address register
    asm("MRC p15, 0, %[r], c6, c0, 0": [r]"=r" (fa)::);

    cprintf ("data abort: instruction 0x%x, fault addr 0x%x, reason 0x%x \n",
             r->pc, fa, dfs);

    dump_trapframe (r);
    show_callstk("Stack dump for data exception.");
}

// trap routine
void iabort_handler (struct trapframe *r)
{
    uint ifs;

    // read fault status register
    asm("MRC p15, 0, %[r], c5, c0, 0": [r]"=r" (ifs)::);

    cli();
    cprintf ("prefetch abort at: 0x%x (reason: 0x%x)\n", r->pc, ifs);
    dump_trapframe (r);
}

// trap routine
void na_handler (struct trapframe *r)
{
    cli();
    cprintf ("n/a at: 0x%x \n", r->pc);
}

// trap routine
void fiq_handler (struct trapframe *r)
{
    cli();
    cprintf ("fiq at: 0x%x \n", r->pc);
}

// low-level init code: in real hardware, lower memory is usually mapped
// to flash during startup, we need to remap it to SDRAM
void trap_init (void)
{
    volatile uint32 *ram_start;
    char *stk;
    int i;
    uint modes[] =
    { PSR_MODE_FIQ
    , PSR_MODE_IRQ
    , PSR_MODE_ABT
    , PSR_MODE_UND
    };

    // map interrupt vectors
    map_vectors(PA_START);

    // set the reset vector to point to real reset handler
    ram_start = (uint32*)HVECTORS;
    ram_start[8] = (uint32)trap_reset;

    // initialize the stacks for different mode
    for (i = 0; i < sizeof(modes)/sizeof(uint); i++) {
        stk = alloc_page ();

        if (stk == NULL) {
            panic("failed to alloc memory for irq stack");
        }

        set_stk (modes[i], (uint)stk);
    }

    enable_interrupts();
}

void dump_trapframe (struct trapframe *tf)
{
    cprintf ("r14_svc: 0x%x\n", tf->r14_svc);
    cprintf ("   spsr: 0x%x\n", tf->spsr);
    cprintf ("     r0: 0x%x\n", tf->r0);
    cprintf ("     r1: 0x%x\n", tf->r1);
    cprintf ("     r2: 0x%x\n", tf->r2);
    cprintf ("     r3: 0x%x\n", tf->r3);
    cprintf ("     r4: 0x%x\n", tf->r4);
    cprintf ("     r5: 0x%x\n", tf->r5);
    cprintf ("     r6: 0x%x\n", tf->r6);
    cprintf ("     r7: 0x%x\n", tf->r7);
    cprintf ("     r8: 0x%x\n", tf->r8);
    cprintf ("     r9: 0x%x\n", tf->r9);
    cprintf ("    r10: 0x%x\n", tf->r10);
    cprintf ("    r11: 0x%x\n", tf->r11);
    cprintf ("    r12: 0x%x\n", tf->r12);
    cprintf ("     pc: 0x%x\n", tf->pc);
}
