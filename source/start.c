#include "types.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "defs.h"

// this code mainly runs in low address,
// and since we linked whole code at high address,
// we cannot use global variables here because GVs
// use absolute reference in asm

// setup the boot page table: dev_mem whether it is device memory
void set_bootpgtbl (uint32 virt, uint32 phy, uint len, int dev_mem )
{
    uint32 *kernel_pgtbl = (uint32*)V2P(&_kernel_pgtbl);
    uint32 *user_pgtbl = (uint32*)V2P(&_user_pgtbl);

    uint32 pde;
    int idx;

    // convert all the parameters to indexes
    virt >>= PDE_SHIFT;
    phy  >>= PDE_SHIFT;
    len  >>= PDE_SHIFT;

    for (idx = 0; idx < len; idx++) {
        pde = (phy << PDE_SHIFT);

        if (!dev_mem) {
            // normal memory, make it kernel-only, cachable, bufferable
            pde |= (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
        } else {
            // device memory, make it non-cachable and non-bufferable
            pde |= (AP_KO << 10) | KPDE_TYPE;
        }

        // use different page table for user/kernel space
        if (virt < NUM_UPDE) {
            user_pgtbl[virt] = pde;
        } else {
            kernel_pgtbl[virt] = pde;
        }

        virt++;
        phy++;
    }
}

static void _flush_all (void)
{
    uint val = 0;

    // flush all TLB
    asm("MCR p15, 0, %[r], c8, c7, 0" : :[r]"r" (val):);

    // invalid entire data and instruction cache
    // asm ("MCR p15,0,%[r],c7,c5,0": :[r]"r" (val):);
    // asm ("MCR p15,0,%[r],c7,c6,0": :[r]"r" (val):);
}

void load_pgtlb (uint32* kern_pgtbl, uint32* usr_pgtbl)
{
    uint	val;
    // we need to check the cache/tlb etc., but let's skip it for now

    // set domain access control: all domain will be checked for permission
    val = 0x55555555;
    asm("MCR p15, 0, %[v], c3, c0, 0": :[v]"r" (val):);

    // set the page table base registers. We use two page tables: TTBR0
    // for user space and TTBR1 for kernel space
    val = 32 - UADDR_BITS;
    asm("MCR p15, 0, %[v], c2, c0, 2": :[v]"r" (val):);

    // set the kernel page table
    //      0x08 RGN=b01  (outer cacheable write-back cached, write allocate)
    //      S=0      (translation table walk to non-shared memory)
    //      0x40 IRGN=b01 (inner cacheability for the translation table walk
    //                      is Write-back Write-allocate)
    // val = (uint)kernel_pgtbl | 0x08 | 0x40;
    val = (uint)kern_pgtbl;
    asm("MCR p15, 0, %[v], c2, c0, 1": :[v]"r" (val):);

    // set the user page table
    // val = (uint)user_pgtbl | 0x08 | 0x40;
    val = (uint)usr_pgtbl;
    asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);

    // ok, enable paging using read/modify/write
    asm("MRC p15, 0, %[r], c1, c0, 0": [r]"=r" (val)::);

    // enable MMU, cache, write buffer, high vector tbl,
    // disable subpage
    // val |= 0x80300D;
    val |= 1 | (1 << 2) | (1 << 12) | (1 << 13);

    // led_flash_no_map(500000, 1);
    asm("MCR p15, 0, %[r], c1, c0, 0": :[r]"r" (val):);

    _flush_all();
}

extern void * edata;
extern void * end;
// clear the BSS section for the main kernel, see kernel.ld
void clear_bss (void)
{
    memset(&edata, 0x00, (uint)&end-(uint)&edata);
}

extern void jump_stack (void);
extern int cmain(void);
void start (void)
{
    uint32 *kernel_pgtbl = (uint32*)V2P(&_kernel_pgtbl);
    uint32 *user_pgtbl = (uint32*)V2P(&_user_pgtbl);
    // uint32  vectbl;

    // double map the low memory, required to enable paging
    // we do not map all the physical memory
    set_bootpgtbl(PA_START, PA_START, INIT_KERN_SZ, 0);
    set_bootpgtbl(KERNBASE+PA_START, PA_START, INIT_KERN_SZ, 0);

    // vector table is in the middle of first 1MB (0xF000)
    // vectbl = P2V_WO ((VEC_TBL & PDE_MASK) + PHY_START);
    //
    // if (vectbl <= (uint)&end) {
    //     _puts("error: vector table overlap and cprintf() is 0x00000\n");
    //     cprintf ("error: vector table overlaps kernel\n");
    // }
    // V, P, len, is_mem
    // set_bootpgtbl(VEC_TBL, PHY_START, 1 << PDE_SHIFT, 0); // V, P, SZ, ISDEV
    // set_bootpgtbl(HVECTORS, PA_START, 1 << PDE_SHIFT, 0); // V, P, SZ, ISDEV

    set_bootpgtbl(GPUMEMBASE, GPUMEMBASE, GPUMEMSIZE, 1); // V, P, SZ, ISDEV

    set_bootpgtbl(DEVSPACE, PHYSIO, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV

    // set_test_pgtbl();
    load_pgtlb (kernel_pgtbl, user_pgtbl);

    // We can now call normal kernel functions at high memory
    clear_bss ();
}
