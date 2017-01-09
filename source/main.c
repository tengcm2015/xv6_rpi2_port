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

void mpmain(int cpunum)
{
    static int mpwait = 1;

    if (cpunum == 0) {
        mpwait = 0;
        return;
    }

    while (mpwait)
        ;

    // under development
    if (cpunum != 3) {
        for(;;)
            ;
    }

    cprintf("cpu%d: starting\n", cpunum);

    stack_init();
    enable_interrupts();

    timer3init();

    scheduler();

    NotOkLoop();
}

extern void test_main(void);
void kmain (int cpunum)
{
    // curr_cpu = &cpus[0];
    if (cpunum) {
        mpmain(cpunum);
    }

    uartinit(BAUDRATE);

    consoleinit ();             // console

    // led_flash(500000, 1); // debug

    /* reference:
        https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
     */
    RPI_PropertyInit();
    RPI_PropertyAddTag( TAG_GET_ARM_MEMORY );
    RPI_PropertyProcess();
    rpi_mailbox_property_t* mp;
    mp = RPI_PropertyGet( TAG_GET_ARM_MEMORY );

    if ( mp ) {
        cprintf( "MEMORY:\r\n" );
        cprintf( "Base address: 0x%x\r\n", mp->data.buffer_32[0] );
        cprintf( "Size: %d byte\r\n", mp->data.buffer_32[1] );
    } else {
        cprintf( "Mailbox error\r\n" );
    }

    init_vmm ();
    kpt_freerange (align_up(&end, PT_SZ), P2V_WO(INIT_KERNMAP));
    paging_init (INIT_KERNMAP, PA_STOP);
    // cprintf("it is ok after paging_init\n");

    kmem_init ();
    // cprintf("it is ok after kmem_init\n");
    kmem_init2(P2V(INIT_KERNMAP), P2V(PA_STOP));
    // cprintf("it is ok after kmem_init2\n");

    trap_init ();               // vector table and stacks for models
    // cprintf("it is ok after trap_init\n");

    // test_main();

    pinit();
    // cprintf("it is ok after pinit\n");
    binit();
    // cprintf("it is ok after binit\n");
    fileinit();
    // cprintf("it is ok after fileinit\n");
    iinit();
    // cprintf("it is ok after iinit\n");
    ideinit();
    // cprintf("it is ok after ideinit\n");
    timer3init();
    // cprintf("it is ok after timer3init\n");
    mpmain(0);

    sti ();
    userinit();
    // cprintf("it is ok after userinit\n");
    scheduler();

    NotOkLoop();
}
