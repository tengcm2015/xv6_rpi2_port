#include "types.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "defs.h"

// uint32 *kernel_pgtbl = (uint32*)V2P(&_kernel_pgtbl);
static uint32 *kernel_pgtbl = (uint32*)0x4000;
static uint32 *user_pgtbl = (uint32*)V2P(&_user_pgtbl);
// uint32 *kernel_pgtbl = V2P_WO(&_kernel_pgtbl);
// uint32 *user_pgtbl = V2P_WO(&_user_pgtbl);

// static void set_test_pgtbl (void);

// setup the boot page table: dev_mem whether it is device memory
void set_bootpgtbl (uint32 virt, uint32 phy, uint len, int dev_mem )
{
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

void load_pgtlb (uint32* kern_pgtbl, uint32* user_pgtbl)
{
    uint	ret;
    char	arch;
    uint	val;

    // read the main id register to make sure we are running on ARMv6
    asm("MRC p15, 0, %[r], c0, c0, 0": [r]"=r" (ret)::);

    if (ret >> 24 == 0x41) {
        //_puts ("ARM-based CPU\n");
    }

    arch = (ret >> 16) & 0x0F;

    if ((arch != 7) && (arch != 0xF)) {
        // _puts ("need AARM v6 or higher\n");
    }

    // we need to check the cache/tlb etc., but let's skip it for now

    // set domain access control: all domain will be checked for permission
    val = 0x55555555;
    asm("MCR p15, 0, %[v], c3, c0, 0": :[v]"r" (val):);

    // set the page table base registers. We use two page tables: TTBR0
    // for user space and TTBR1 for kernel space
    val = 32 - UADDR_BITS;
    asm("MCR p15, 0, %[v], c2, c0, 2": :[v]"r" (val):);

    // set the kernel page table
    val = (uint)kernel_pgtbl | 0x00;
    asm("MCR p15, 0, %[v], c2, c0, 1": :[v]"r" (val):);

    // set the user page table
    val = (uint)user_pgtbl | 0x00;
    asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);

    // ok, enable paging using read/modify/write
    asm("MRC p15, 0, %[r], c1, c0, 0": [r]"=r" (val)::);

    // enable MMU, cache, write buffer, high vector tbl,
    // disable subpage
    // val |= 0x80300D;
    val |= 1 | (1 << 2) | (1 << 12) | (1 << 13);

    asm("MCR p15, 0, %[r], c1, c0, 0": :[r]"r" (val):);

    _flush_all();
}

extern void * edata_entry;
extern void * svc_stktop;
extern void jump_stack (void);
int cmain(void);

extern void * edata;
extern void * end;

// clear the BSS section for the main kernel, see kernel.ld
void clear_bss (void)
{
    memset(&edata, 0x00, (uint)&end-(uint)&edata);
}

void start (void)
{
    // uint32  vectbl;
    // _puts("starting xv6 for ARM...\n");

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
    set_bootpgtbl(HVECTORS, PA_START, 1 << PDE_SHIFT, 1); // V, P, SZ, ISDEV
    // set_bootpgtbl(VEC_TBL, PHY_START, 1 << PDE_SHIFT, 0); // V, P, SZ, ISDEV

    // cannot map this for some reason...
    // set_bootpgtbl(GPUMEMBASE, GPUMEMBASE, GPUMEMSIZE, 1); // V, P, SZ, ISDEV

    set_bootpgtbl(DEVSPACE, PHYSIO, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV

    // set_test_pgtbl();
    // led_flash_no_map(500000, 3);
    load_pgtlb (kernel_pgtbl, user_pgtbl);

    jump_stack ();

    // We can now call normal kernel functions at high memory
    clear_bss ();

    cmain ();
}

// static void set_test_pgtbl (void)
// {
//     uint pa = 0x00100000;
//     uint va = 0x80100000;
//     // uint *kpgdir = (uint*)V2P(&_kernel_pgtbl);
//     uint *kpgdir = kernel_pgtbl;
//     // uint32 *kpgdir = (uint32*)0x4000;
//     // uint *upgdir = V2P_WO(&_user_pgtbl);
//     // uint *pgtab = (uint*)0x00010000;
//     // int ap = AP_KO;
//
//     uint *pde = &kpgdir[PDE_IDX(va)];
//     *pde = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
//     // *pde = (uint)pgtab | UPDE_TYPE;
//     // uint *pte = &pgtab[PTE_IDX(va)];
//     // *pte = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
//
//     uint *pde2 = &kpgdir[PDE_IDX(pa)];
//     *pde2 = pa | (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
//     // *pde2 = (uint)pgtab | UPDE_TYPE;
//     // uint *pte2 = &pgtab[PTE_IDX(pa)];
//     // *pte2 = pa | ((ap & 0x3) << 4) | PE_CACHE | PE_BUF | PTE_TYPE;
// }
