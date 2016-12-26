/*****************************************************************
*       main.c
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/


#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "arm.h"
// #include "rpi_mailbox.h"

extern void* end; // first address after kernel loaded from ELF file
extern FBI fbinfo;

struct cpu	cpus[NCPU];
struct cpu	*curr_cpu;

void OkLoop()
{
    setgpiofunc(16, 1); // gpio 16, set as an output
    while(1){
        setgpioval(16, 0);
        delay(1000000);
        setgpioval(16, 1);
        delay(1000000);
    }
}

void NotOkLoop()
{
    setgpiofunc(16, 1); // gpio 16, set as an output
    while(1){
        setgpioval(16, 0);
        delay(100000);
        setgpioval(16, 1);
        delay(100000);
    }
}

int cmain()
{
    // mailboxinit();
    // create_request(mailbuffer, MPI_TAG_GET_ARM_MEMORY, 8, 0, 0);
    // writemailbox((uint *)mailbuffer, 8);
    // readmailbox(8);
    // if(mailbuffer[1] != 0x80000000)
    //     cprintf("new error readmailbox\n");
    // else
    //     cprintf("ARM memory is %x %x\n",
    //         mailbuffer[MB_HEADER_LENGTH + TAG_HEADER_LENGTH],
    //         mailbuffer[MB_HEADER_LENGTH + TAG_HEADER_LENGTH + 1]
    //     );
    uint vectbl;

    curr_cpu = &cpus[0];

    uartinit(BAUDRATE);

    vectbl = P2V_WO ((HVECTORS & PDE_MASK) + PA_START);

    init_vmm ();
    kpt_freerange (align_up(&end, PT_SZ), vectbl);
    kpt_freerange (vectbl + PT_SZ, P2V_WO(INIT_KERNMAP));
    paging_init (INIT_KERNMAP, PHYSTOP);

    kmem_init ();
    kmem_init2(P2V(INIT_KERNMAP), P2V(PHYSTOP));

    trap_init ();               // vector table and stacks for models

    consoleinit ();             // console

    pinit();
    cprintf("it is ok after pinit\n");
    binit();
    cprintf("it is ok after binit\n");
    fileinit();
    cprintf("it is ok after fileinit\n");
    iinit();
    cprintf("it is ok after iinit\n");
    ideinit();
    cprintf("it is ok after ideinit\n");
    timer3init();

    sti ();
    userinit();
    cprintf("it is ok after userinit\n");
    scheduler();

    NotOkLoop();

    return 0;
}
