// #include "param.h"
#include "types.h"
#include "defs.h"
// #include "arm.h"
// #include "memlayout.h"
// #include "mmu.h"
// #include "proc.h"
// #include "spinlock.h"
// #include "elf.h"

/*
* Write the level and type you want to Cache Size Selection Register(CSSELR)
* to get size details from Current Cache Size ID Register(CCSIDR)
*/
// static void set_csselr(u32 level, u32 type)
// {	u32 csselr = level << 1 | type;
//     /* Write to Cache Size Selection Register(CSSELR) */
//     asm("mcr p15, 2, %0, c0, c0, 0" : : "r" (csselr));
// }
//
// static u32 get_ccsidr(void)
// {
//     u32 ccsidr;
//     /* Read current CP15 Cache Size ID Register */
//     asm("mrc p15, 1, %0, c0, c0, 0" : "=r" (ccsidr));
//     return ccsidr;
// }
//
// static u32 get_clidr(void)
// {
//     u32 clidr;
//     /* Read current CP15 Cache Level ID Register */
//     asm("mrc p15,1,%0,c0,c0,1" : "=r" (clidr));
//     return clidr;
// }
//
// static void v7_clean_inval_dcache_level_setway(uint32 level, uint32 num_sets,
//                                                uint32 num_ways, uint32 way_shift,
//                                                uint32 log2_line_len)
// {
//     int way, set, setway;
//     /*
//     * For optimal assembly code:
//     *	a. count down
//     *	b. have bigger loop inside
//     */
//     for (way = num_ways - 1; way >= 0 ; way--) {
//         for (set = num_sets - 1; set >= 0; set--) {
//             setway = (level << 1) | (set << log2_line_len) |
//             (way << way_shift);
//             /*
//             * Clean & Invalidate data/unified
//             * cache line by set/way
//             */
//             asm("mcr p15, 0, %0, c7, c14, 2": : "r" (setway));
//         }
//     }
//     /* DSB to make sure the operation is complete */
//     CP15DSB;
// }
//
// static void v7_maint_dcache_level_setway(uint32 level)
// {
//     uint32 ccsidr;
//     uint32 num_sets, num_ways, log2_line_len, log2_num_ways;
//     uint32 way_shift;
//
//     set_csselr(level, ARMV7_CSSELR_IND_DATA_UNIFIED);
//
//     ccsidr = get_ccsidr();
//
//     log2_line_len = ((ccsidr & CCSIDR_LINE_SIZE_MASK) >>
//             CCSIDR_LINE_SIZE_OFFSET) + 2;
//     /* Converting from words to bytes */
//     log2_line_len += 2;
//
//     num_ways  = ((ccsidr & CCSIDR_ASSOCIATIVITY_MASK) >>
//             CCSIDR_ASSOCIATIVITY_OFFSET) + 1;
//     num_sets  = ((ccsidr & CCSIDR_NUM_SETS_MASK) >>
//             CCSIDR_NUM_SETS_OFFSET) + 1;
//     /*
//     * According to ARMv7 ARM number of sets and number of ways need
//     * not be a power of 2
//     */
//     log2_num_ways = log_2_n_round_up(num_ways);
//
//     way_shift = (32 - log2_num_ways);
//
//     v7_clean_inval_dcache_level_setway(level, num_sets, num_ways, way_shift, log2_line_len);
// }

void flush_dcache_all(void)
{
    // uint32 level, cache_type, level_start_bit = 0;
    //
    // uint32 clidr = get_clidr();
    //
    // for (level = 0; level < 7; level++) {
    //     cache_type = (clidr >> level_start_bit) & 0x7;
    //     if ((cache_type == ARMV7_CLIDR_CTYPE_DATA_ONLY) ||
    //         (cache_type == ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA) ||
    //         (cache_type == ARMV7_CLIDR_CTYPE_UNIFIED))
    //             v7_maint_dcache_level_setway(level, operation);
    //     level_start_bit += 3;
    // }
    clean_inval_dcache_all();
    /* Full system DSB - make sure that the invalidation is complete */
    asm("DSB" ::: "memory");
}

void invalidate_icache_all(void)
{
    /*
     * Invalidate all instruction caches to PoU.
     * Also flushes branch target cache.
     */
    asm("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));
    /* Invalidate entire branch predictor array */
    asm("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));
    /* Full system DSB - make sure that the invalidation is complete */
    asm("DSB" ::: "memory");
    /* Full system ISB - make sure the instruction stream sees it */
    asm("ISB" ::: "memory");
}

// invalid all TLB
void invalidate_tlb (void)
{
    /* Invalidate entire unified TLB */
    asm("MCR p15, 0, %0, c8, c7, 0" : : "r" (0));
    // cprintf("invalidate unified TLB\n");
    /* Invalidate entire data TLB */
    asm("MCR p15, 0, %0, c8, c6, 0" : : "r" (0));
    // cprintf("invalidate data TLB\n");
    /* Invalidate entire instruction TLB */
    asm("MCR p15, 0, %0, c8, c5, 0" : : "r" (0));
    // cprintf("invalidate insturction TLB\n");
    /* Full system DSB - make sure that the invalidation is complete */
    asm("DSB" ::: "memory");
    /* Full system ISB - make sure the instruction stream sees it */
    asm("ISB" ::: "memory");
    // cprintf("done!\n");
}
