#ifndef DEFS_H
#define DEFS_H

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct stat;
struct superblock;
struct trapframe;

void OkLoop(void);
void NotOkLoop(void);

// arm.c
uint            inw(uint addr);
void            outw(uint addr, uint data);
void            cli (void);
void            sti (void);
uint            readcpsr(void);
uint            cpsr_usr(void);
int             int_enabled();
void            pushcli(void);
void            popcli(void);
void            getcallerpcs(void *, uint*);
void            show_callstk(char *);
// asm.S
void            set_stk(uint mode, uint addr);
void*           get_fp(void);
void*           get_sp(void);
uint            get_cpunum(void);
void            enable_interrupts(void);
// void            invalidate_dcache_all(void);
void            clean_inval_dcache_all(void);

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);

// buddy.c
void            kmem_init (void);
void            kmem_init2(void *vstart, void *vend);
void*           kmalloc (int order);
void            kfree (void *mem, int order);
void            free_page(void *v);
void*           alloc_page (void);
void            kmem_test_b (void);
int             get_order (uint32 v);

// cache.c
void            flush_dcache_all(void);
void            invalidate_icache_all(void);
void            invalidate_tlb (void);

// console.c
void            consoleinit(void);
void            cprintf(char*, ...);
void            consoleintr(int(*)(void));
void            panic(char*) __attribute__((noreturn));
void            drawcharacter(uint8, uint, uint);
void            gpuputc(uint);


// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);


// fs.c
void            readsb(int dev, struct superblock *sb);
int             dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit(void);
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, uint, uint);

// kalloc.c
// char*           kalloc(void);
// void            kfree(char*);
// void            kinit1(void*, void*);
// void            kinit2(void*, void*);
// void            kmem_init (void);

// log.c
void            initlog(void);
void            log_write(struct buf*);
void            begin_trans();
void            commit_trans();

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, char*, int);
int             pipewrite(struct pipe*, char*, int);

//PAGEBREAK: 16
// proc.c
struct proc*    copyproc(struct proc*);
void            exit(void);
int             fork(void);
int             growproc(int);
int             kill(int);
void            pinit(void);
void            procdump(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(void);
void            wakeup(void*);
void            yield(void);
void            dump_context (struct context *c);
void            procdump(void);

// swtch.S
void            swtch(struct context**, struct context*);

// syscall.c
int             argint(int, int*);
int             argptr(int, char**, int);
int             argstr(int, char**);
int             fetchint(uint, int*);
int             fetchstr(uint, char**);
void            syscall(void);

void            kvmalloc(void);

int             UsbInitialise(void);
void            KeyboardUpdate(void);
char            KeyboardGetChar(void);
uint            KeyboardCount(void);
uint            KeyboardGetAddress(uint);
struct          KeyboardLeds KeyboardGetLedSupport(uint);

// spinlock.c
void            acquire(struct spinlock*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
void*           memcpy(void *dst, const void *src, uint n);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);
uint            div(uint n, uint d);

// syscall.c
int             argint(int, int*);
int             argptr(int, char**, int);
int             argstr(int, char**);
int             fetchint(uint, int*);
int             fetchstr(uint, char**);
void            syscall(void);

// timer.c
void            timer3init(void);
void            timer3intr(void);
void            delay(uint);
extern struct   spinlock tickslock;
extern uint     ticks;

// trap.c
void            trap_init(void);
void            stack_init(void);
void            dump_trapframe (struct trapframe *tf);

// trap_asm.S
void            trap_reset(void);
void            trap_und(void);
void            trap_swi(void);
void            trap_iabort(void);
void            trap_dabort(void);
void            trap_na(void);
void            trap_irq(void);
void            trap_fiq(void);

// uart.c
void            uartinit(int);
void            miniuartintr(void);
void            uartputc(uint);
void            setgpiofunc(uint, uint);
void            setgpioval(uint, uint);
void            led_flash(int, int);
void            led_flash_no_map(int, int);

// vm.c
int             allocuvm(pde_t*, uint, uint);
int             deallocuvm(pde_t*, uint, uint);
void            freevm(pde_t*);
void            inituvm(pde_t*, char*, uint);
int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
pde_t*          copyuvm(pde_t*, uint);
void            switchuvm(struct proc*);
int             copyout(pde_t*, uint, void*, uint);
void            clearpteu(pde_t *pgdir, char *uva);
void*           kpt_alloc(void);
void            init_vmm (void);
void            kpt_freerange (uint32 low, uint32 hi);
void            paging_init (uint phy_low, uint phy_hi);
void            map_vectors (uint vector_start);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

#endif
