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
    @ initialize stack pointers for svc modes
    MOV     r0, #(PSR_MODE_SVC | PSR_DISABLE_IRQ | PSR_DISABLE_FIQ )
    MSR     cpsr, r0
    LDR     sp, =V2P_WO(svc_stktop)

    @ smp cores
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
