// Mutual exclusion lock.
struct spinlock {
#if 1
    uint        locked;     // Is the lock held?
#else
// ticket-based lock
    union {
        uint        locked;     // Is the lock held?
        struct __raw_tickets {
            uint16 owner;
            uint16 next;
        } tickets;
    };
#endif
    // For debugging:
    char        *name;      // Name of lock.
    struct cpu  *cpu;       // The cpu holding the lock.
    uint        pcs[10];    // The call stack (an array of program counters)
                            // that locked the lock.
};
