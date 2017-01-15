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

.global get_sp
get_sp:
    MOV     r0, sp
    bx      lr

.global get_cpunum
get_cpunum:
    MRC     p15, 0, r0, c0, c0, 5   @ MPIDR
    AND     r0, r0, #0xFF
    bx      lr

.global enable_interrupts
enable_interrupts:
    MRS     r0, cpsr
    BIC     r0, r0, #(PSR_DISABLE_IRQ | PSR_DISABLE_FIQ)
    MSR     cpsr_c, r0
    bx      lr

.global invalidate_dcache_all
invalidate_dcache_all:
    MRC     p15, 1, r0, c0, c0, 0       @ Read Cache Size ID.
    LDR     r3, =#0x1ff
    AND     r0, r3, r0, LSR #13         @ r0 = no. of sets - 1.

    MOV     r1, #0                      @ r1 = way counter way_loop.
way_loop:
    MOV     r3, #0                      @ r3 = set counter set_loop.
set_loop:
    MOV     r2, r1, LSL #30
    ORR     r2, r3, LSL #5              @ r2 = set/way cache operation format.
    MCR     p15, 0, r2, c7, c6, 2       @ Invalidate the line described by r2.
    ADD     r3, r3, #1                  @ Increment set counter.
    CMP     r0, r3                      @ Last set reached yet?
    BGT     set_loop                    @ If not, iterate set_loop,
    ADD     r1, r1, #1                  @ else, next.
    CMP     r1, #4                      @ Last way reached yet?
    BNE     way_loop                    @ if not, iterate way_loop.

    bx      lr

.global clean_inval_dcache_all
clean_inval_dcache_all:
    MRC     p15, 1, r0, c0, c0, 0       @ Read Cache Size ID.
    LDR     r3, =#0x1ff
    AND     r0, r3, r0, LSR #13         @ r0 = no. of sets - 1.

    MOV     r1, #0                      @ r1 = way counter way_loop.
way_loop_1:
    MOV     r3, #0                      @ r3 = set counter set_loop.
set_loop_1:
    MOV     r2, r1, LSL #30
    ORR     r2, r3, LSL #5              @ r2 = set/way cache operation format.
    MCR     p15, 0, r2, c7, c14, 2      @ Clean and Invalidate the line described by r2.
    ADD     r3, r3, #1                  @ Increment set counter.
    CMP     r0, r3                      @ Last set reached yet?
    BGT     set_loop_1                  @ If not, iterate set_loop,
    ADD     r1, r1, #1                  @ else, next.
    CMP     r1, #4                      @ Last way reached yet?
    BNE     way_loop_1                  @ if not, iterate way_loop.

    bx      lr
