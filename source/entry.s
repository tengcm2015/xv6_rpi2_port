#include "arm.h"
#include "memlayout.h"

.section .vectors
.global vectors
vectors:
    ldr pc, reset_handler
    ldr pc, undefintr_handler
    ldr pc, swi_handler
    ldr pc, prefetch_handler
    ldr pc, data_handler
    ldr pc, unused_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler
reset_handler:
    .word V2P_WO(_start)       /* reset, in svc mode already */
undefintr_handler:
    .word trap_und             /* undefined instruction */
swi_handler:
    .word trap_swi             /* SWI & SVC */
prefetch_handler:
    .word trap_iabort          /* prefetch abort */
data_handler:
    .word trap_dabort          /* data abort */
unused_handler:
    .word trap_na              /* reserved */
irq_handler:
    .word trap_irq             /* IRQ */
fiq_handler:
    .word trap_fiq             /* FIQ */

.section .text
.global _start
_start:
    @==================================================================
    @ Disable caches, MMU and branch prediction in case they were left enabled from an earlier run
    @ This does not need to be done from a cold reset
    @==================================================================

    @MRC     p15, 0, r0, c1, c0, 0       @ Read CP15 System Control register
    @BIC     r0, r0, #(0x1 << 12)        @ Clear I bit 12 to disable I Cache
    @BIC     r0, r0, #(0x1 <<  2)        @ Clear C bit  2 to disable D Cache
    @BIC     r0, r0, #0x1                @ Clear M bit  0 to disable MMU
    @BIC     r0, r0, #(0x1 << 11)        @ Clear Z bit 11 to disable branch prediction
    @MCR     p15, 0, r0, c1, c0, 0       @ Write value back to CP15 System Control register

    @===================================================================
    @ ACTLR.SMP Enables coherent requests to the processor.
    @ You must ensure this bit is set to 1 before the caches and MMU are enabled,
    @ or any cache and TLB maintenance operations are performed.
    @===================================================================
    @MRC     p15, 0, r0, c1, c0, 1   @ Read CP15 ACTLR
    @ORR     r0, r0, #(1 << 6)       @ set ACTLR.SMP bit
    @MCR     p15, 0, r0, c1, c0, 1   @ Write CP15 ACTLR

    @===================================================================
    @ Initialize Supervisor Mode Stack
    @===================================================================
    MOV     r0, #(PSR_MODE_SVC | PSR_DISABLE_IRQ | PSR_DISABLE_FIQ )
    MSR     cpsr, r0
    LDR     sp, =V2P_WO(svc_stktop)

    @===================================================================
    @ Set Vector Base Address Register (VBAR) to point to
    @ this application's vector table
    @===================================================================
    @LDR     r0, =vectors
    @MCR     p15, 0, r0, c12, c0, 0

    @==================================================================
    @ Cache Invalidation code for Cortex-A7
    @ NOTE: Neither Caches, nor MMU, nor BTB need post-reset invalidation on Cortex-A7,
    @ but forcing a cache invalidation, makes the code more portable to other CPUs (e.g. Cortex-A9)
    @==================================================================
    @ Invalidate L1 Instruction Cache
    @MRC     p15, 1, r0, c0, c0, 1      @ Read Cache Level ID Register (CLIDR)
    @TST     r0, #0x3                   @ Harvard Cache?
    @MOV     r0, #0                     @ SBZ
    @MCRNE   p15, 0, r0, c7, c5, 0      @ ICIALLU - Invalidate instruction cache and flush branch target cache

    @ Invalidate Data/Unified Caches
@    MRC     p15, 1, r0, c0, c0, 1      @ Read CLIDR
@    ANDS    r3, r0, #0x07000000        @ Extract coherency level
@    MOV     r3, r3, LSR #23            @ Total cache levels << 1
@    BEQ     Finished                   @ If 0, no need to clean
@
@    MOV     r10, #0                    @ R10 holds current cache level << 1
@Loop1:
@    ADD     r2, r10, r10, LSR #1       @ R2 holds cache "Set" position
@    MOV     r1, r0, LSR r2             @ Bottom 3 bits are the Cache-type for this level
@    AND     r1, r1, #7                 @ Isolate those lower 3 bits
@    CMP     r1, #2
@    BLT     Skip                       @ No cache or only instruction cache at this level
@
@    MCR     p15, 2, r10, c0, c0, 0     @ Write the Cache Size selection register
@    ISB                                @ ISB to sync the change to the CacheSizeID reg
@    MRC     p15, 1, r1, c0, c0, 0      @ Reads current Cache Size ID register
@    AND     r2, r1, #7                 @ Extract the line length field
@    ADD     r2, r2, #4                 @ Add 4 for the line length offset (log2 16 bytes)
@    LDR     r4, =0x3FF
@    ANDS    r4, r4, r1, LSR #3         @ R4 is the max number on the way size (right aligned)
@    CLZ     r5, r4                     @ R5 is the bit position of the way size increment
@    LDR     r7, =0x7FFF
@    ANDS    r7, r7, r1, LSR #13        @ R7 is the max number of the index size (right aligned)
@
@Loop2:
@    MOV     r9, r4                     @ R9 working copy of the max way size (right aligned)
@Loop3:
@    ORR     r11, r10, r9, LSL r5       @ Factor in the Way number and cache number into R11
@    ORR     r11, r11, r7, LSL r2       @ Factor in the Set number
@    MCR     p15, 0, r11, c7, c6, 2     @ Invalidate by Set/Way
@    SUBS    r9, r9, #1                 @ Decrement the Way number
@    BGE     Loop3
@    SUBS    r7, r7, #1                 @ Decrement the Set number
@    BGE     Loop2
@Skip:
@    ADD     r10, r10, #2               @ increment the cache number
@    CMP     r3, r10
@    BGT     Loop1
@Finished:

    @===================================================================
    @ smp cores
    @===================================================================
    MRC     p15, 0, r0, c0, c0, 5   @ MPIDR
    MOV     r1, #0xFF
    ANDS    r1, r1, r0
    BNE     smp

    BL      start
    BL      NotOkLoop

@===================================================================
@ during startup, kernel stack uses user address,
@ now switch it to kernel addr
@===================================================================
.global jump_stack
jump_stack:
    MOV     r0, sp
    ADD     r0, r0, #KERNBASE
    MOV     sp, r0
    MOV     pc, lr

@===================================================================
@ smp cores
@===================================================================
smp:
    B       .
