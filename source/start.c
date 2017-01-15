#include "types.h"
#include "memlayout.h"
#include "mmu.h"
#include "arm.h"
#include "defs.h"

// this code mainly runs in low address,
// and since we linked whole code at high address,
// global variables and static variables use absolute reference in asm
// so we need to convert them if we want use them...
#define get_global_no_map(type, x) (*(type*)V2P(&x))
// maybe swap start code to low addr someday ...

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
    // uint val = 0;

    // flush all TLB
    // asm("MCR p15, 0, %[r], c8, c7, 0" : :[r]"r" (val):);
    invalidate_tlb();

    // invalid entire data and instruction cache
    invalidate_dcache_all();
    invalidate_icache_all();
    // asm ("MCR p15,0,%[r],c7,c5,0": :[r]"r" (val):);
    // asm ("MCR p15,0,%[r],c7,c6,0": :[r]"r" (val):);
}

void load_pgtlb (uint32* kern_pgtbl, uint32* usr_pgtbl)
{
    uint    val;
    // we need to check the cache/tlb etc., but let's skip it for now
    // set SMP bit in ACTLR
    asm("MRC p15, 0, %[r], c1, c0, 1": [r]"=r" (val)::);
    val |= 1 << 6;
    asm("MCR p15, 0, %[v], c1, c0, 1": :[v]"r" (val):);

    // set domain access control: all domain will be checked for permission
    val = 0x55555555;
    asm("MCR p15, 0, %[v], c3, c0, 0": :[v]"r" (val):);

    // set the page table base registers. We use two page tables: TTBR0
    // for user space and TTBR1 for kernel space
    val = 32 - UADDR_BITS;
    asm("MCR p15, 0, %[v], c2, c0, 2": :[v]"r" (val):);

    // set the kernel page table (with Multiprocessing Extensions)
    //      0x08 RGN=b01  (outer cacheable write-back cached, write allocate)
    //      0x02 S=1      (translation table walk to shared memory)
    //      0x40 IRGN=b01 (inner cacheability for the translation table walk
    //                      is Write-back Write-allocate)
    val = (uint)kern_pgtbl | 0x02 | 0x08 | 0x40;
    // val = (uint)kern_pgtbl | 0x0;
    asm("MCR p15, 0, %[v], c2, c0, 1": :[v]"r" (val):);

    // set the user page table
    val = (uint)usr_pgtbl | 0x02 | 0x08 | 0x40;
    // val = (uint)usr_pgtbl | 0x0;
    asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);

    _flush_all();

    // ok, enable paging using read/modify/write
    asm("MRC p15, 0, %[r], c1, c0, 0": [r]"=r" (val)::);

    // enable MMU, D/U-cache, branch prediction, I-cache, high vector tbl
    // val |= 1 | (1 << 2) | (1 << 11) | (1 << 12) | (1 << 13);
    // val |= 0x80300D;
    // for some reason ACTLR.SMP doesn't work with data cache...
    val |= 1 | (1 << 11) | (1 << 12) | (1 << 13);
    // val |= 1 | (1 << 13);

    // led_flash_no_map(500000, 1);
    asm("MCR p15, 0, %[r], c1, c0, 0": :[r]"r" (val):);

    _flush_all();
}

void init_pgtbl(void)
{
    // interrupt vectors are mapped with small page at trapinit

    // double map the low memory, required to enable paging
    // we do not map all the physical memory
    set_bootpgtbl(PA_START, PA_START, INIT_KERN_SZ, 0);
    set_bootpgtbl(KERNBASE+PA_START, PA_START, INIT_KERN_SZ, 0);

    set_bootpgtbl(GPUMEMBASE, GPUMEMBASE, GPUMEMSIZE, 1); // V, P, SZ, ISDEV

    set_bootpgtbl(DEVSPACE, PHYSIO, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV
}

extern void * edata;
extern void * end;
// clear the BSS section for the main kernel, see kernel.ld
void clear_bss (void)
{
    memset(&edata, 0x00, (uint)&end-(uint)&edata);
}

void start (int cpunum)
{
    uint32 *kernel_pgtbl = (uint32*)V2P(&_kernel_pgtbl);
    uint32 *user_pgtbl = (uint32*)V2P(&_user_pgtbl);

    static int mpwait = 1;

    if (cpunum == 0) {
        init_pgtbl();

        // let mpcores enable caches right now
        // because caches needs to be on for every core
        // before any cache maintainence to keep consistency
        get_global_no_map(int, mpwait) = 0;

        load_pgtlb (kernel_pgtbl, user_pgtbl);
        // We can now call normal kernel functions at high memory
        clear_bss ();

    } else {
        // maybe swap start code to low addr someday ...
        while(get_global_no_map(int, mpwait))
            ;

        load_pgtlb (kernel_pgtbl, user_pgtbl);
    }
}
