#include "arm.h"

.text
.code 32

@ set the stack for IRQ mode, we can use r0-r3 for free
@ input: r0 - #mode, r1 - stk
.global set_stk
set_stk:
    @ switch to the mode stocked in r0
    MRS     r2, cpsr
    BIC     r2, r2, #PSR_MASK
    ORR     r2, r2, r0
    MSR     cpsr_cxsf, r2

    @ set the stack pointer, sp register is banked, so we need to switch mode
    MOV     sp, r1

    @ switch back to the SVC mode
    BIC     r2, r2, #PSR_MASK
    ORR     r2, r2, #PSR_MODE_SVC
    MSR     cpsr_cxsf, r2

    @ return
    bx      lr

@ return the frame pointer for the current function
.global get_fp
get_fp:
    MOV     r0, fp
    bx      lr

.global enable_interrupts
enable_interrupts:
    MRS     r0, cpsr
    BIC     r0, r0, #(PSR_DISABLE_IRQ | PSR_DISABLE_FIQ)
    MSR     cpsr_c, r0
    bx      lr

.global flush_dcache_all
flush_dcache_all:
    MOV     r0, #0
    MCR     p15, 0, r0, c7, c10, 4 /* dsb */
    MCR     p15, 0, r0, c7, c14, 0 /* invalidate d-cache */
    bx      lr
