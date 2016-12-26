/*****************************************************************
*       memlayout.h
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/

#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include "rpi_base.h"

// Memory layout
// we first map 1MB low memory containing kernel code.
#define INIT_KERN_SZ    0x00100000
#define INIT_KERNMAP 	(INIT_KERN_SZ + PA_START)
#define DEV_MEM_SZ      0x01000000

#define EXTMEM          0x00000000      /* start of kernel code */
#define PHYSTOP         0x0C000000      /* assuming 128M RAM; need a fix to find the true RAM size */
#define DEVSPACE        0xFE000000      /* i/o registers */

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE        0x80000000         // First kernel virtual address
#define KERNLINK        (KERNBASE+EXTMEM)  // Address where kernel is linked
#define USERBOUND       0x40000000         // maximum user space due to one page pgd
#define GPUMEMBASE      0x40000000
#define GPUMEMSIZE      (1024*MBYTE)

#define PA_START        0x0
#define PHYSIO          PERIPHERAL_BASE
#define RAMSIZE         0x0C000000
#define IOSIZE          (16*MBYTE)
#define TVSIZE          0x00001000

#ifndef __ASSEMBLER__

static inline uint v2p(void *a) { return ((uint) (a))  - KERNBASE; }
static inline void *p2v(uint a) { return (void *) ((a) + KERNBASE); }

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) (((void *) (a)) + KERNBASE)

#endif

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts

#endif
