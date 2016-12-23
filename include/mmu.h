// Definition for ARM MMU
#ifndef MMU_INCLUDE
#define MMU_INCLUDE

// align_up/down: al must be of power of 2
#define align_up(sz, al) (((uint)(sz)+ (uint)(al)-1) & ~((uint)(al)-1))
#define align_dn(sz, al) ((uint)(sz) & ~((uint)(al)-1))
//
// Since ARMv6, you may use two page tables, one for kernel pages (TTBR1),
// and one for user pages (TTBR0). We use this architecture. Memory address
// lower than UVIR_BITS^2 is translated by TTBR0, while higher memory is
// translated by TTBR1.
// Kernel pages are create statically during system initialization. It use
// 1MB page mapping. User pages use 4K pages.
//


// access permission for page directory/page table entries.
#define AP_NA       0x00    // no access
#define AP_KO       0x01    // privilaged access, kernel: RW, user: no access
#define AP_KUR      0x02    // no write access from user, read allowed
#define AP_KU       0x03    // full access

// domain definition for page table entries
#define DM_NA       0x00    // any access causing a domain fault
#define DM_CLIENT   0x01    // any access checked against TLB (page table)
#define DM_RESRVED  0x02    // reserved
#define DM_MANAGER  0x03    // no access check

#define PE_CACHE    (1 << 3)// cachable
#define PE_BUF      (1 << 2)// bufferable

#define PE_TYPES    0x03    // mask for page type
#define KPDE_TYPE   0x02    // use "section" type for kernel page directory
#define UPDE_TYPE   0x01    // use "coarse page table" for user page directory
#define PTE_TYPE    0x02    // executable user page(subpage disable)

// 1st-level or large (1MB) page directory (always maps 1MB memory)
#define PDE_SHIFT   20                      // shift how many bits to get PDE index
#define PDE_SZ      (1 << PDE_SHIFT)
#define PDE_MASK    (PDE_SZ - 1)            // offset for page directory entries
#define PDE_IDX(v)  ((uint)(v) >> PDE_SHIFT) // index for page table entry

// 2nd-level page table
#define PTE_SHIFT   12                  // shift how many bits to get PTE index
#define PTE_IDX(v)  (((uint)(v) >> PTE_SHIFT) & (NUM_PTE - 1))
#define PTE_SZ      (1 << PTE_SHIFT)
#define PTE_ADDR(v) align_dn (v, PTE_SZ)
#define PTE_AP(pte) (((pte) >> 4) & 0x03)

// size of two-level page tables
#define UADDR_BITS  28                  // maximum user-application memory, 256MB
#define UADDR_SZ    (1 << UADDR_BITS)   // maximum user address space size

// must have NUM_UPDE == NUM_PTE
#define NUM_UPDE    (1 << (UADDR_BITS - PDE_SHIFT)) // # of PDE for user space
#define NUM_PTE     (1 << (PDE_SHIFT - PTE_SHIFT))  // how many PTE in a PT

#define PT_SZ       (NUM_PTE << 2)                  // user page table size (1K)
#define PT_ADDR(v)  align_dn(v, PT_SZ)              // physical address of the PT
#define PT_ORDER    10

/*=================================================*/

#define MBYTE		0x100000
#define K_PDX_BASE	0x4000
#define K_PTX_BASE	0x3000

#define CACHELINESIZE   32

/*
 * page table entries.
*/

#define UNMAPPED	0x00000000

#define COARSE		(0<<4|1)
#define SECTION		(0<<4|2)

#define LARGE		0x00000001
#define SMALL		0x00000002
#define BUFFERED	0x00000004
#define CACHED		0x00000008
#define DOMAIN0		0

#define NOACCESS	0
#define K_RW		1
#define U_AP		2
#define U_RW		3

#define ACCESS_PERM(n, v)	(((v) & 3) << (((n) * 2) + 4))
#define PDX_AP(ap)		(ACCESS_PERM(3, (ap)))
#define PTX_AP(ap) 		(ACCESS_PERM(3, (ap)) | ACCESS_PERM(2, (ap)) \
				| ACCESS_PERM(1, (ap)) | ACCESS_PERM(0, (ap)))

#define HVECTORS        0xffff0000

// A virtual address 'la' has a three-part structure as follows:
//
// +--------12------+-------8--------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

// page directory index
#define PDX(va)         (((uint)(va) >> PDXSHIFT) & 0xFFF)

// page table index
#define PTX(va)         (((uint)(va) >> PTXSHIFT) & 0xFF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Address in page table or page directory entry
#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PGSHIFT         12      // log2(PGSIZE)
#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        20      // offset of PDX in a linear address


#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

#define PGDIR_BASE	P2V(K_PDX_BASE)

#define KVMPDXATTR       DOMAIN0|PDX_AP(U_RW)|SECTION|CACHED|BUFFERED

#define UVMPDXATTR 	DOMAIN0|COARSE
#define UVMPTXATTR	PTX_AP(U_RW)|CACHED|BUFFERED|SMALL

#endif
