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

extern void test_main(void);

void mpmain(int cpunum)
{
    static int mpwait = 1;

    if (cpunum == 0) {
        mpwait = 0;
        return;
    }

    while (mpwait)
        ;

    // make sure they are off at first
    cpus[cpunum].enabled = 0;

    // under development
    delay(50 * cpunum);
    // for(;;)
    //     ;

    // cprintf("cpu%d: starting\n", cpunum);

    stack_init();
    enable_interrupts();

    // test_main();

    scheduler();

    NotOkLoop();
}

void kmain (int cpunum)
{
    // curr_cpu = &cpus[0];
    if (cpunum) {
        mpmain(cpunum);
    }

    // main core should be on
    cpus[0].enabled = 1;

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

int cpuutil(int cpunum, int mode, int enabled)
{
    int i, at_least_one_cpu_is_on;

    switch(mode) {
    case 0: // get enabled
        return cpus[cpunum].enabled;

    case 1: // set enabled
        if (cpunum < 0 || cpunum >= NCPU) {
            cprintf("setcore: cpu %d not available.\n", cpunum);
            return -1;
        }

        if (enabled == 0) {
            if (cpunum == get_cpunum()) {
                cprintf("setcore: cpu %d is running this process.\n", cpunum);
                cprintf("         It will turned off after this process.\n");
            }

            for(i = 0, at_least_one_cpu_is_on = 0; i < NCPU; i++) {
                if (i != cpunum) {
                    at_least_one_cpu_is_on += cpus[NCPU].enabled;
                }
            }

            if (!at_least_one_cpu_is_on) {
                cprintf("setcore: cpu %d is the last running core!\n", cpunum);
                return -1;
            }
        }

        if (enabled != 0 && enabled != 1) {
            cprintf("setcore: invalid argument %d\n", cpunum);
            return -1;
        }

        cpus[cpunum].enabled = enabled;
        cprintf("setcore: set core[%d].enabled to %d\n", cpunum, enabled);

        return 0;
    default:
        cprintf("setcore: invalid mode %d\n", cpunum);
        return -1;
    }
}
