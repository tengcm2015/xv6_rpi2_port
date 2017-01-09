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
    .word V2P_WO(entry)        /* reset, in svc mode already */
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
.global entry
entry:
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
    MRC     p15, 0, r0, c1, c0, 1   @ Read CP15 ACTLR
    ORR     r0, r0, #(1 << 6)       @ set ACTLR.SMP bit
    MCR     p15, 0, r0, c1, c0, 1   @ Write CP15 ACTLR

    @===================================================================
    @ Initialize Supervisor Mode
    @===================================================================
    MOV     r0, #(PSR_MODE_SVC | PSR_DISABLE_IRQ | PSR_DISABLE_FIQ )
    MSR     cpsr, r0

    @===================================================================
    @ smp cores
    @===================================================================
    @MRC     p15, 0, r1, c0, c0, 5   @ MPIDR
    @MOV     r0, #0xFF
    @ANDS    r0, r0, r1
    @BNE     smp
    MRC     p15, 0, r0, c0, c0, 5   @ MPIDR
    AND     r0, r0, #0xFF

    CMP     r0, #0
    BEQ     core_zero
    CMP     r0, #1
    BEQ     core_one
    CMP     r0, #2
    BEQ     core_two
    CMP     r0, #3
    BEQ     core_three

    B       .

@ set svc stacks
core_zero:
    LDR     sp, =V2P_WO(svc_stktop)
    BL      _start

core_one:
    LDR     sp, =V2P_WO(svc_stktop_1)
    BL      _start

core_two:
    LDR     sp, =V2P_WO(svc_stktop_2)
    BL      _start

core_three:
    LDR     sp, =V2P_WO(svc_stktop_3)

_start:
    BL      start

    @===================================================================
    @ during startup, kernel stack uses user address,
    @ now switch it to kernel addr
    @===================================================================
    MOV     r0, sp
    ADD     r0, r0, #KERNBASE
    MOV     sp, r0

    @===================================================================
    @ start execution at high address
    @===================================================================
    MRC     p15, 0, r0, c0, c0, 5   @ MPIDR
    AND     r0, r0, #0xFF

    LDR     pc, =kmain

    BL      NotOkLoop
