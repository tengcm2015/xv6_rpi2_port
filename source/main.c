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
#include "rpi_gpio.h"
#include "rpi_mailbox.h"
#include "rpi_mailbox-interface.h"

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
    // uint vectbl;

    curr_cpu = &cpus[0];

    uartinit(BAUDRATE);

    consoleinit ();             // console

    led_flash(500000, 1); // debug

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
    RPI_PropertyInit();
    RPI_PropertyAddTag( TAG_GET_ARM_MEMORY );
    RPI_PropertyProcess();
    rpi_mailbox_property_t* mp;
    mp = RPI_PropertyGet( TAG_GET_ARM_MEMORY );

    /* reference:
        https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
     */
    if ( mp ) {
        cprintf( "MEMORY:\r\n" );
        cprintf( "Base address: 0x%x\r\n", mp->data.buffer_32[0] );
        cprintf( "Size: %d byte\r\n", mp->data.buffer_32[1] );
    } else {
        cprintf( "Mailbox error\r\n" );
    }

    // interrrupt vector table is in the middle of first 1MB. We use the left
    // over for page tables
    // vectbl = P2V_WO ((HVECTORS & PDE_MASK) + PA_START);

    init_vmm ();
    // kpt_freerange (align_up(&end, PT_SZ), vectbl);
    // kpt_freerange (vectbl + PT_SZ, P2V_WO(INIT_KERNMAP));
    kpt_freerange (align_up(&end, PT_SZ), P2V_WO(INIT_KERNMAP));
    paging_init (INIT_KERNMAP, PA_STOP);

    kmem_init ();
    cprintf("it is ok after kmem_init\n");
    kmem_init2(P2V(INIT_KERNMAP), P2V(PA_STOP));
    cprintf("it is ok after kmem_init2\n");

    trap_init ();               // vector table and stacks for models
    cprintf("it is ok after trap_init\n");

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
    cprintf("it is ok after timer3init\n");

    sti ();
    userinit();
    cprintf("it is ok after userinit\n");
    scheduler();

    NotOkLoop();

    return 0;
}
