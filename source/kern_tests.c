#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "rpi_gpio.h"
#include "rpi_aux.h"

// extern void set_test_pgtbl();
// static uint32 *kernel_pgtbl = &_kernel_pgtbl;
// static uint32 *user_pgtbl = &_user_pgtbl;

static void _flush_all (void)
{
    uint val = 0;

    // flush all TLB
    asm("MCR p15, 0, %[r], c8, c7, 0" : :[r]"r" (val):);

    // invalid and clean entire data and instruction cache
    asm ("MCR p15, 0, %[r], c7, c5, 0": :[r]"r" (val):);
    asm ("MCR p15, 0, %[r], c7, c14, 1": :[r]"r" (val):);
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
    uint *pde, *pte;
    uint pa = 0x00100000;
    uint va = 0x80100000;
    uint *kpgdir = &_kernel_pgtbl;
    uint *upgdir = &_user_pgtbl;
    uint *pgtab = (uint*)0x00010000;
    int ap = AP_KO;

    pde = &kpgdir[PDE_IDX(va)];
    // *pde = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
    *pde = (uint)pgtab | UPDE_TYPE;
    pte = &pgtab[PTE_IDX(va)];
    *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;

    pde = &upgdir[PDE_IDX(pa)];
    // *pde = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
    *pde = (uint)pgtab | UPDE_TYPE;
    pte = &pgtab[PTE_IDX(pa)];
    *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
    _flush_all();
}

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

void test_paging() {
    cprintf("test_paging start\n");

    set_test_pgtbl();
    test_rw();

    cprintf("test_paging done\n");
}

void
test_main(void)
{
    cprintf("test_main start\n");

    test_paging();

    cprintf("test_main done\n");
}
