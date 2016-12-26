/*****************************************************************
*       arm.h
*       by Zhiyi Huang, hzy@cs.otago.ac.nz
*       University of Otago
*
********************************************************************/

#ifndef ARM_INCLUDE
#define ARM_INCLUDE

#define PSR_MODE_USR    0x00000010
#define PSR_MODE_FIQ    0x00000011
#define PSR_MODE_IRQ    0x00000012
#define PSR_MODE_SVC    0x00000013
#define PSR_MODE_MON    0x00000016
#define PSR_MODE_ABT    0x00000017
#define PSR_MODE_UND    0x0000001B
#define PSR_MODE_SYS    0x0000001F
#define PSR_MASK        0x0000001F
#define USER_MODE       0x0

// cpsr/spsr bits
#define PSR_DISABLE_IRQ     0x00000080
#define PSR_DISABLE_FIQ     0x00000040

#define PSR_V           0x10000000
#define PSR_C           0x20000000
#define PSR_Z           0x40000000
#define PSR_N           0x80000000

// vector table
#define TRAP_RESET      0
#define TRAP_UND        1
#define TRAP_SWI        2
#define TRAP_IABT       3
#define TRAP_DABT       4
#define TRAP_NA         5
#define TRAP_IRQ        6
#define TRAP_FIQ        7

// trap frame: in ARM, there are seven modes. Among the 16 regular registers,
// r13 (sp), r14(lr), r15(pc) are banked in all modes.
// 1. In xv6_a, all kernel level activities (e.g., Syscall and IRQ) happens
// in the SVC mode. CPU is put in different modes by different events. We
// switch them to the SVC mode, by shoving the trapframe to the kernel stack.
// 2. during the context switched, the banked user space registers should also
// be saved/restored.
//
// Here is an example:
// 1. a user app issues a syscall (via SWI), its user-space registers are
// saved on its kernel stack, syscall is being served.
// 2. an interrupt happens, it preempted the syscall. the app's kernel-space
// registers are again saved on its stack.
// 3. interrupt service ended, and execution returns to the syscall.
// 4. kernel decides to reschedule (context switch), it saves the kernel states
// and switches to a new process (including user-space banked registers)
#ifndef __ASSEMBLER__
struct trapframe {
    uint    sp_usr;     // user mode sp
    uint    lr_usr;     // user mode lr
    uint    r14_svc;    // r14_svc (r14_svc == pc if SWI)
    uint    spsr;
    uint    r0;
    uint    r1;
    uint    r2;
    uint    r3;
    uint    r4;
    uint    r5;
    uint    r6;
    uint    r7;
    uint    r8;
    uint    r9;
    uint    r10;
    uint    r11;
    uint    r12;
    uint    pc;         // (lr on entry) instruction to resume execution
};

#endif

#endif
