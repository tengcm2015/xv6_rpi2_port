#include "arm.h"
#include "memlayout.h"

.global _start
_start:
@ initialize stack pointers for svc modes
	MSR     CPSR_cxsf, #(PSR_MODE_SVC|NO_INT)
	LDR     sp, =V2P_WO(svc_stktop)

	MRC     p15, 0, r0, c0, c0, 5 ;@ MPIDR
	MOV     r1, #0xFF
	ANDS    r1, r1, r0
	BNE     smp

	BL      start
	BL      NotOkLoop

@ during startup, kernel stack uses user address, now switch it to kernel addr
.global jump_stack
jump_stack:
	MOV     r0, sp
	ADD     r0, r0, #KERNBASE
	MOV     sp, r0
	MOV     pc, lr

smp:
	B       .

.global dsb_barrier
dsb_barrier:
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4
	bx lr
.global flush_dcache_all
flush_dcache_all:
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 /* dsb */
	mov r0, #0
	mcr p15, 0, r0, c7, c14, 0 /* invalidate d-cache */
	bx lr
.global flush_idcache
flush_idcache:
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 /* dsb */
	mov r0, #0
	mcr p15, 0, r0, c7, c14, 0 /* invalidate d-cache */
	mov r0, #0
	mcr p15, 0, r0, c7, c5, 0 /* invalidate i-cache */
	bx lr
.global flush_tlb
flush_tlb:
	mov r0, #0
	mcr p15, 0, r0, c8, c7, 0
	mcr p15, 0, r0, c7, c10, 4
	bx lr
.global flush_dcache /* flush a range of data cache flush_dcache(va1, va2) */
flush_dcache:
	mcrr p15, 0, r0, r1, c14
	bx lr
.global set_pgtbase /* set the page table base set_pgtbase(base) */
set_pgtbase:
	mcr p15, 0, r0, c2, c0
	bx lr

.global getsystemtime
getsystemtime:
	ldr r0, =0xFE003004 /* addr of the time-stamp lower 32 bits */
	ldrd r0, r1, [r0]
	bx lr
