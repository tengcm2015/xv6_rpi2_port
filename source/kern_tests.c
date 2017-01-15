#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "rpi_gpio.h"
#include "rpi_aux.h"
#include "spinlock.h"
#include "proc.h"

// struct spinlock test_lock;

// extern void set_test_pgtbl();
// static uint32 *kernel_pgtbl = &_kernel_pgtbl;
// static uint32 *user_pgtbl = &_user_pgtbl;

// static void _flush_all (void)
// {
//     cprintf("invalidate TLBs\n");
//     invalidate_tlb();
//     cprintf("invalidate icaches\n");
//     invalidate_icache_all();
//     cprintf("invalidate dcaches\n");
//     flush_dcache_all();
// }

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
// static void set_test_pgtbl (void)
// {
//     // uint *pde;
//     // uint *pte;
//     // uint pa = 0x00100000;
//     // uint va = 0x80100000;
//     // uint *kpgdir = &_kernel_pgtbl;
//     uint *upgdir = &_user_pgtbl;
//     // uint *upgdir = (uint*)0x00010000;
//     // uint *pgtab = (uint*)0x00020000;
//     // int ap = AP_KO;
//
//     // pde = &kpgdir[PDE_IDX(va)];
//     // // *pde = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
//     // *pde = (uint)pgtab | UPDE_TYPE;
//     // pte = &pgtab[PTE_IDX(va)];
//     // *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
//
//     // pde = &upgdir[PDE_IDX(0)];
//     // *pde = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
//     // *pde = (uint)pgtab | UPDE_TYPE;
//     // pte = &pgtab[PTE_IDX(pa)];
//     // *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
//
//     // pde = &upgdir[PDE_IDX(0)];
//     // *pde = 0 | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
//     // pde = &upgdir[PDE_IDX(pa)];
//     // *pde = (uint)pgtab | UPDE_TYPE;
//     // pte = &pgtab[PTE_IDX(pa)];
//     // *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
//
//     // uint val = (uint) upgdir | 0x00;
//     // asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);
//
//     cprintf("sp:%x\n", get_sp());
//
//     _flush_all();
//
//     upgdir[0] = 0;
//
//     _flush_all();
// }

// void test_rw() {
//     int *addr;
//     int *addr2;
//     cprintf("test_rw start\n");
//
//     cprintf("test 1:\n");
//     addr = (int*)0x000ffff0;
//     addr2 = (int*)(KERNBASE + (uint)addr);
//     *addr = 111;
//     *addr2 = 222;
//     cprintf("addr1: %d\n", *addr);
//     cprintf("addr2: %d\n", *addr2);
//
//     cprintf("test 2:\n");
//     addr = (int*)0x00100010;
//     addr2 = (int*)(KERNBASE + (uint)addr);
//     *addr2 = 444;
//     cprintf("addr2: %d\n", *addr2);
//     *addr = 333;
//     cprintf("addr1: %d\n", *addr);
//
//     // addr = (int*)0x00200000;
//     // addr2 = (int*)(KERNBASE + (uint)addr);
//     // *addr2 = 0;
//     // *addr = 5;
//     // if (*addr2 == 5) {
//     //     led_flash(500000, 5);
//     // } else {
//     //     led_flash(100000, 10);
//     // }
//     cprintf("test_rw done\n");
// }

// void test_paging() {
//     cprintf("test_paging start\n");
//
//     set_test_pgtbl();
//     test_rw();
//
//     cprintf("test_paging done\n");
// }

// void test_acquire(struct spinlock *lk)
// {
//     pushcli();		// disable interrupts to avoid deadlock.
//
//     // if(holding(lk)) {
//     //     // cprintf("acquire: cpu %d already holding\n", get_cpunum());
//     //     // return;
//     //     panic("acquire: already holding");
//     // }
//
//     // arch_spin_lock() from linux kernel
//     uint tmp;
//     uint newval;
//     struct spinlock lockval;
//
//     asm (
//     "1: LDREX   %0, [%3]        \n" /* exclusively load lk->locked into %0, i.e. lockval.locked */
//     "   ADD     %1, %0, %4      \n" /* store lk->tickets.owner + 1 to newval */
//     "   STREX   %2, %1, [%3]    \n" /* exclusively store newval into lk.locked */
//     "   TEQ     %2, #0          \n" /* test if store succeeded. */
//     "   BNE     1b                " /* branch on negative to 1 to try again */
//     : "=&r" (lockval.locked), "=&r" (newval), "=&r" (tmp)   /* %0, %1, %2 */
//     : "r" (&lk->locked), "I" (1 << 16)    /* inputs, %3, %4 */
//     : "cc");                        /* clobbered register cc (condition code) will be modified */
//
//     while (lockval.tickets.next != lockval.tickets.owner) {
//         asm("WFE" ::: "memory");
//         lockval.tickets.owner = lk->tickets.owner;
//     }
//
//     // Record info about lock acquisition for debugging.
//     lk->cpu = curr_cpu;
//     // getcallerpcs(get_fp(), lk->pcs);
//     asm("DMB" ::: "memory");        /* SMP memory barrier to protect the &lk->locked */
//
//     // if(lk->name[0] == 'p')
//     //     cprintf("cpu%d acquired lock: %s\n", get_cpunum(), lk->name);
// }
//
// // Release the lock.
// void test_release(struct spinlock *lk)
// {
//     asm("DMB" ::: "memory");        /* SMP memory barrier to protect the &lk->locked */
//
//     // if(!holding(lk)) {
//     //     // cprintf("release: cpu %d not holding\n", get_cpunum());
//     //     // popcli();
//     //     // return;
//     //     panic("release: not holding");
//     // }
//
//     lk->pcs[0] = 0;
//     lk->cpu = curr_cpu;
//
//     lk->tickets.owner++;
//
//     asm("DSB" ::: "memory");        /* Data Synchronization Barrier */
//     asm("SEV" ::: "memory");        /* Send event */
//     asm("NOP");                     /* No-op */
//
//     popcli();
// }
//
//
// void test_locks() {
//     initlock(&test_lock, "test");
//
//     test_acquire(&test_lock);
//     cprintf("test: acquire \n");
//     test_release(&test_lock);
//     cprintf("test: release \n");
//
//     test_acquire(&test_lock);
//     cprintf("test: acquire \n");
//     test_release(&test_lock);
//     cprintf("test: release \n");
//
//     test_acquire(&test_lock);
//     cprintf("test: acquire \n");
//     test_release(&test_lock);
//     cprintf("test: release \n");
// }

// void
// test_main(void)
// {
//     cprintf("test_main start\n");
//
//     // test_paging();
//     // test_locks();
//
//     cprintf("test_main done\n");
// }
