#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "rpi_gpio.h"
#include "rpi_aux.h"

// extern void set_test_pgtbl();
// extern uint32 *kernel_pgtbl;
static uint32 *kernel_pgtbl = &_kernel_pgtbl;
// static uint32 *kernel_pgtbl = (uint32*)V2P(&_kernel_pgtbl);
// static uint32 *kernel_pgtbl = (uint32*)0x4000;
// static uint32 *user_pgtbl = (uint32*)V2P(&_user_pgtbl);

void test_rw() {
    int *addr;
    int *addr2;
    cprintf("test_rw start\n");

    cprintf("test 1:\n");
    addr = (int*)0x000ffff0;
    addr2 = (int*)(KERNBASE + (uint)addr);
    *addr = 111;
    *addr2 = 222;
    cprintf("addr1: %d\n", *addr);
    cprintf("addr2: %d\n", *addr2);

    cprintf("test 2:\n");
    addr = (int*)0x00100010;
    addr2 = (int*)(KERNBASE + (uint)addr);
    *addr = 333;
    *addr2 = 444;
    cprintf("addr1: %d\n", *addr);
    cprintf("addr2: %d\n", *addr2);

    // addr = (int*)0x00200000;
    // addr2 = (int*)(KERNBASE + (uint)addr);
    // *addr2 = 0;
    // *addr = 5;
    // if (*addr2 == 5) {
    //     led_flash(500000, 5);
    // } else {
    //     led_flash(100000, 10);
    // }
    cprintf("test_rw done\n");
}

// void test_cpsr() {
//     uint	val;
//     // asm("MRC p15, 0, %[r], c1, c1, 0": [r]"=r" (val)::);
//     asm("MRS %[r], CPSR": [r]"=r" (val)::);
//     if ((val & MODE_MASK) == SVC_MODE) {
//         led_flash(500000, 5);
//     } else {
//         led_flash(100000, 10);
//     }
// }
static void set_test_pgtbl (void)
{
    uint pa = 0x00100000;
    uint va = 0x80100000;
    // uint *kpgdir = (uint*)V2P(&_kernel_pgtbl);
    uint *kpgdir = kernel_pgtbl;
    // uint32 *kpgdir = (uint32*)0x4000;
    // uint *upgdir = V2P_WO(&_user_pgtbl);
    // uint *pgtab = (uint*)0x00010000;
    // int ap = AP_KO;

    uint *pde = &kpgdir[PDE_IDX(va)];
    *pde = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
    // *pde = (uint)pgtab | UPDE_TYPE;
    // uint *pte = &pgtab[PTE_IDX(va)];
    // *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;

    uint *pde2 = &kpgdir[PDE_IDX(pa)];
    *pde2 = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
    // *pde2 = (uint)pgtab | UPDE_TYPE;
    // uint *pte2 = &pgtab[PTE_IDX(pa)];
    // *pte2 = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
}

void test_paging() {
    cprintf("test_paging start\n");

    set_test_pgtbl();
    test_rw();

    cprintf("test_paging done\n");
}

void test_addr() {
    cprintf("test_addr start\n");
    cprintf("_kernel_pgtbl: %x\n", _kernel_pgtbl);
    cprintf("&_kernel_pgtbl: %x\n", &_kernel_pgtbl);
    cprintf("kernel_pgtbl: %x\n", kernel_pgtbl);
    cprintf("V2P(&_kernel_pgtbl): %x\n", V2P(&_kernel_pgtbl));
    cprintf("test_addr start\n");
}

void
test_main(void)
{
    cprintf("test_main start\n");

    test_addr();
    test_paging();

    cprintf("test_main done\n");
}
