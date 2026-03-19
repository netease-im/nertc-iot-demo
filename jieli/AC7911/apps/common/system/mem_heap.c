/**
 * @file
 * Dynamic memory manager
 *
 * This is a lightweight replacement for the standard C library malloc().
 */

#include "generic/printf.h"

#ifdef SUPPORT_MS_EXTENSIONS
/* #pragma bss_seg(	".mem_heap_bss") */
/* #pragma data_seg(	".mem_heap_data") */
#pragma const_seg(	".mem_heap_const")
#pragma code_seg(	".mem_heap_code")
#endif

static inline void *sbrk(int incr);

typedef unsigned char u8_t;
typedef signed char err_t;
typedef unsigned int u32_t, mem_ptr_t;
typedef unsigned int mem_size_t;

#define MEM_HEAP_NO_SYS                      0	 //0:使用操作系统,1:不使用操作系统

#define MEM_HEAP_ERROR_CHECK                 1
#define MEM_HEAP_CLR_MAGIC                   (u8_t)0xff
#define MEM_HEAP_OVERFLOW_MAGIC              0x53555aca

#define SBRK_PIECE_SIZE (20*1024*1024)

/** All allocated blocks will be MEM_HEAP_MIN_SIZE bytes big, at least!
 * MEM_HEAP_MIN_SIZE can be overridden to suit your needs. Smaller values save space,
 * larger values could prevent too small blocks to fragment the RAM too much. */
#define MEM_HEAP_MIN_SIZE                    4

#define MEM_HEAP_STATS                       1   //统计功能


/** print debug message only if debug message type is enabled...
 */
#define MEM_HEAP_PLATFORM_ASSERT(x) \
     ASSERT(x)
/* ASSERT("\"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__); */

#define MEM_HEAP_PLATFORM_DIAG(x)  do {printf x;} while(0)

//#define MEM_HEAP_NOASSERT

/** flag for LWIP_DEBUGF to enable that debug message */
#define MEM_HEAP_DBG_ON                     0x80U
/** flag for LWIP_DEBUGF to disable that debug message */
#define MEM_HEAP_DBG_OFF                    0x00U
/** flag for LWIP_DEBUGF to halt after printing this debug message */
#define MEM_HEAP_DBG_HALT                   0x08U

#define MEM_HEAP_DEBUG                      (MEM_HEAP_DBG_ON)

/*
   ------------------------------------
   ---------- MEM_HEAP options ----------
   ------------------------------------
*/
/**
 * MEM_HEAP_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_HEAP_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_HEAP_ALIGNMENT 2
 */
#define MEM_HEAP_ALIGNMENT                   4

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#define MEM_HEAP_START_SIZE                0//8*1024*1024

#ifdef MEM_HEAP_DEBUG

#define MEM_HEAP_DEBUGF(debug, message) do { \
                               if ((debug) & MEM_HEAP_DBG_ON) { \
                                 MEM_HEAP_PLATFORM_DIAG(message); \
                                 if ((debug) & MEM_HEAP_DBG_HALT) { \
                                   while(1); \
                                 } \
                               } \
                             } while(0)
#else  /* MEM_HEAP_DEBUG */
#define MEM_HEAP_DEBUGF(debug, message)
#endif /* MEM_HEAP_DEBUG */

#ifndef MEM_HEAP_NOASSERT
#define MEM_HEAP_ASSERT(message, assertion) do { if(!(assertion)) \
  MEM_HEAP_PLATFORM_ASSERT(message); } while(0)
#else  /* MEM_HEAP_NOASSERT */
#define MEM_HEAP_ASSERT(message, assertion)
#endif /* MEM_HEAP_NOASSERT */


#if MEM_HEAP_STATS

struct stats_mem {
#ifdef MEM_HEAP_DEBUG
    const char *name;
#endif /* MEM_HEAP_DEBUG */
    mem_size_t avail;
    mem_size_t used;
    mem_size_t max;
    u32_t err;
    u32_t illegal;
};

struct stats_ {
    struct stats_mem mem;
};

#define STATS_INC(x) ++mem_heap.mem_stats.x
#define STATS_DEC(x) --mem_heap.mem_stats.x
#define STATS_INC_USED(x, y) do { mem_heap.mem_stats.x.used += y; \
                                if (mem_heap.mem_stats.x.max < mem_heap.mem_stats.x.used) { \
                                    mem_heap.mem_stats.x.max = mem_heap.mem_stats.x.used; \
                                } \
                             } while(0)

#define MEM_STATS_AVAIL(x, y) mem_heap.mem_stats.mem.x = y
#define MEM_STATS_INC(x) STATS_INC(mem.x)
#define MEM_STATS_INC_USED(x, y) STATS_INC_USED(mem, y)
#define MEM_STATS_DEC_USED(x, y) mem_heap.mem_stats.mem.x -= y

#else
#define MEM_STATS_AVAIL(x, y)
#define MEM_STATS_INC(x)
#define MEM_STATS_INC_USED(x, y)
#define MEM_STATS_DEC_USED(x, y)
#endif


#if !MEM_HEAP_NO_SYS

/*#include "os/os_api.h"
*/
#include "system/spinlock.h"

/*typedef OS_MUTEX sys_mutex_t; // type of mutex
*/

/* Protect the heap only by using a semaphore */
#define MEM_HEAP_WAIT_MUTEX_TIMEOUT    0
#define MEM_HEAP_DECL_PROTECT()   u16   oserr
#define MEM_HEAP_PROTECT()     spin_lock(&mem_heap.lock) //os_mutex_pend(&mem_heap.mem_mutex,MEM_HEAP_WAIT_MUTEX_TIMEOUT)//os_mutex_pend(&mem_heap.mem_mutex,MEM_HEAP_WAIT_MUTEX_TIMEOUT,0)
#define MEM_HEAP_UNPROTECT()   spin_unlock(&mem_heap.lock) //os_mutex_post(&mem_heap.mem_mutex)

#else /* !MEM_HEAP_NO_SYS */
#define sys_mutex_new()   0
#define MEM_HEAP_DECL_PROTECT()
#define MEM_HEAP_PROTECT()
#define MEM_HEAP_UNPROTECT()
#endif

/**
 * The heap is made up as a list of structs of this type.
 * This does not have to be aligned since for getting its size,
 * we only use the macro SIZEOF_STRUCT_MEM, which automatically alignes.
 */
#define MEM_USED   0x5a5aeaae
#define MEM_UNUSED 0xa5abcdef
struct mem {
#if MEM_HEAP_ERROR_CHECK == 1
    u32_t check_before;
#endif
    /** index of the next struct */
    struct mem *next;
    /** index of the previous struct */
    struct mem *prev;
    /** MEM_USED: this area is used; MEM_UNUSED: this area is unused */
    u32_t used;
    u32 rets_addr;
#if MEM_HEAP_ERROR_CHECK == 1
    u32_t check_after;
#endif
};

/* some alignment macros: we define them here for better source code layout */
#define MIN_SIZE_ALIGNED     MEM_HEAP_ALIGN_SIZE(MEM_HEAP_MIN_SIZE)
#define SIZEOF_STRUCT_MEM    MEM_HEAP_ALIGN_SIZE(sizeof(struct mem))
#define MEM_SIZE_ALIGNED     MEM_HEAP_ALIGN_SIZE(MEM_HEAP_SIZE)

/** Calculate memory size for an aligned buffer - returns the next highest
 * multiple of MEM_HEAP_ALIGNMENT (e.g. MEM_HEAP_ALIGN_SIZE(3) and
 * MEM_HEAP_ALIGN_SIZE(4) will both yield 4 for MEM_HEAP_ALIGNMENT == 4).
 */
#define MEM_HEAP_ALIGN_SIZE(size) (((size) + MEM_HEAP_ALIGNMENT - 1) & ~(MEM_HEAP_ALIGNMENT-1))

/** Align a memory pointer to the alignment defined by MEM_HEAP_ALIGNMENT
 * so that ADDR % MEM_HEAP_ALIGNMENT == 0
 */
#define MEM_HEAP_ALIGN(addr) ((void *)(((mem_ptr_t)(addr) + MEM_HEAP_ALIGNMENT - 1) & ~(mem_ptr_t)(MEM_HEAP_ALIGNMENT-1)))

struct _mem_heap {
    /** pointer to the heap (ram_heap): for alignment, ram is now a pointer instead of an array */
    u8_t *ram;
    /** the last entry, always unused! */
    struct mem *ram_end;
    /** pointer to the lowest free block, this is used for faster search */
    struct mem *lfree;
    /** record the last sbrk end*/
    u8_t *sbrk_end;
    /** record total mem_heap size */
    mem_ptr_t mem_heap_size;
#if !MEM_HEAP_NO_SYS
    /*sys_mutex_t mem_mutex;*/
    spinlock_t lock;
#endif
#if MEM_HEAP_STATS
    struct stats_ mem_stats;
#endif
    u8_t mem_init;
};
static struct _mem_heap mem_heap = {
    .mem_init = 0,
};

extern u32 MALLOC_SIZE[];
extern u32 HEAP_BEGIN;
extern u32 HEAP_END;

#if MEM_HEAP_STATS
static void mem_stats_init(void)
{
#ifdef MEM_HEAP_DEBUG
    mem_heap.mem_stats.mem.name = "MEM_HEAP";
#endif
}
void malloc_stats(void)
{
    MEM_HEAP_PLATFORM_DIAG(("\nMEM_HEAP %s\n", mem_heap.mem_stats.mem.name));
    MEM_HEAP_PLATFORM_DIAG(("total: %u\n", (u32)MALLOC_SIZE));
    MEM_HEAP_PLATFORM_DIAG(("left: %u\n", ((u32)MALLOC_SIZE - (u32)mem_heap.mem_stats.mem.used)));
    /*MEM_HEAP_PLATFORM_DIAG(("avail: %u\n", (u32)mem_heap.mem_stats.mem.avail));
    MEM_HEAP_PLATFORM_DIAG(("used: %u\n", (u32)mem_heap.mem_stats.mem.used));*/
    MEM_HEAP_PLATFORM_DIAG(("max: %u\n", (u32)mem_heap.mem_stats.mem.max));
    /*MEM_HEAP_PLATFORM_DIAG(("err: %u\n", (u32)mem_heap.mem_stats.mem.err));*/
}
#endif

u32 get_free_heap_size(void)
{
#if MEM_HEAP_STATS
    return (u32)MALLOC_SIZE - (u32)mem_heap.mem_stats.mem.used;
#else
    return 0;
#endif
}

void malloc_dump()
{
    struct mem *mem;
    struct mem *sbrk_lfree = 0;

    MEM_HEAP_PROTECT();
    u32 max_free = 0;
    for (sbrk_lfree ? (mem = sbrk_lfree) : (mem = mem_heap.lfree);
         mem != mem_heap.ram_end; mem = mem->next) {

        if (mem->used == MEM_USED) {
            printf("mem_used: %x - %x, size=%x, call_from=%x\n", (u32)mem,
                   (u32)mem->next, (u32)mem->next - (u32)mem,
                   mem->rets_addr);
        } else {
            printf("mem_free: %x - %x, size=%x\n",
                   (u32)mem, (u32)mem->next, (u32)mem->next - (u32)mem);
            if (max_free < ((u32)mem->next - (u32)mem)) {
                max_free = (u32)mem->next - (u32)mem;
            }
        }
    }
    MEM_HEAP_UNPROTECT();

#if MEM_HEAP_STATS
    u32 last_free = (u8 *)&HEAP_END - (u8 *)mem_heap.ram_end;
    if (max_free < last_free) {
        max_free = last_free;
    }

    float fragment = 100 - 100.0 * max_free / ((u32)MALLOC_SIZE - (u32)mem_heap.mem_stats.mem.used);
    printf("fragment: %f \r\n", fragment);
#endif
}

#if !MEM_HEAP_NO_SYS
static err_t sys_mutex_new(void)
{
    spin_lock_init(&mem_heap.lock);
    /*u16 err;
    err =os_mutex_create(&mem_heap.mem_mutex) ;

    MEM_HEAP_ASSERT("sys_mutex_new ",err == OS_ERR_NONE );
    if(err)
        return err;*/

    return 0;
}
#endif

/**
 * "Plug holes" by combining adjacent empty struct mems.
 * After this function is through, there should not exist
 * one empty struct mem pointing to another empty struct mem.
 *
 * @param mem this points to a struct mem which just has been freed
 * @internal this function is only called by mem_free() and mem_trim()
 *
 * This assumes access to the heap is protected by the calling function
 * already.
 */

static struct mem *
plug_holes(struct mem *mem)
{
    struct mem *mem2 = mem;
    struct mem *nmem;
    struct mem *pmem;

//  MEM_HEAP_ASSERT("plug_holes: mem >= ram", (u8_t *)mem >= mem_heap.ram);
//  MEM_HEAP_ASSERT("plug_holes: mem < ram_end", (u8_t *)mem < (u8_t *)mem_heap.ram_end);
    MEM_HEAP_ASSERT("plug_holes: mem->used == MEM_UNUSED", mem->used == MEM_UNUSED);

    /* plug hole forward */
//  MEM_HEAP_ASSERT("plug_holes: mem->next <= ram_end", mem->next <= mem_heap.ram_end);

    nmem = mem->next;
    if (mem != nmem && nmem->used == MEM_UNUSED && (u8_t *)nmem != (u8_t *)mem_heap.ram_end) {
        /* if mem->next is unused and not end of ram, combine mem and mem->next */
        if (mem_heap.lfree == nmem) {
            mem_heap.lfree = mem;
        }
        mem->next = nmem->next;
        nmem->next->prev = mem;

        MEM_STATS_DEC_USED(used, SIZEOF_STRUCT_MEM);
    }

    /* plug hole backward */
    pmem = mem->prev;
    /* printf("pmem : 0x%x, 0x%x", pmem, mem->rets_addr); */
    if (pmem && pmem != mem && pmem->used == MEM_UNUSED) {
        /* if mem->prev is unused, combine mem and mem->prev */
        if (mem_heap.lfree == mem) {
            mem_heap.lfree = pmem;
        }
        pmem->next = mem->next;
        mem->next->prev = pmem;
        mem2 = pmem;
        MEM_STATS_DEC_USED(used, SIZEOF_STRUCT_MEM);
    }
    return mem2;
}

/**
 * Zero the heap and initialize start, end and lowest-free
 */
static void mem_heap_init(void)
{
    u8_t *sbrk_start;
    struct mem *mem;

    if (mem_heap.mem_init) {
        return;
    }

#if MEM_HEAP_STATS
    mem_stats_init();
#endif
    MEM_HEAP_ASSERT("Sanity check alignment",
                    (SIZEOF_STRUCT_MEM & (MEM_HEAP_ALIGNMENT - 1)) == 0);

    mem_heap.mem_heap_size = MEM_HEAP_START_SIZE;

    sbrk_start = sbrk(MEM_HEAP_START_SIZE + (2 * SIZEOF_STRUCT_MEM) + MEM_HEAP_ALIGNMENT);
    /* align the heap */
    mem_heap.ram = (u8_t *)MEM_HEAP_ALIGN(sbrk_start);
    /* initialize the start of the heap */
    mem = (struct mem *)mem_heap.ram;
    mem->next = (struct mem *)&mem_heap.ram[mem_heap.mem_heap_size + SIZEOF_STRUCT_MEM];
    mem->prev = mem;
    mem->used = MEM_UNUSED;
    /* initialize the end of the heap */
    mem_heap.ram_end = (struct mem *)(void *)&mem_heap.ram[mem_heap.mem_heap_size + SIZEOF_STRUCT_MEM];
    mem_heap.ram_end->used = MEM_USED;
    mem_heap.ram_end->next = mem_heap.ram_end;
    mem_heap.ram_end->prev = (struct mem *)mem_heap.ram;
#if MEM_HEAP_ERROR_CHECK == 1
    mem->check_before = MEM_HEAP_OVERFLOW_MAGIC;
    mem->check_after = MEM_HEAP_OVERFLOW_MAGIC;
    mem_heap.ram_end->check_before = MEM_HEAP_OVERFLOW_MAGIC;
    mem_heap.ram_end->check_after = MEM_HEAP_OVERFLOW_MAGIC;
#endif
    mem_heap.sbrk_end = sbrk_start + MEM_HEAP_START_SIZE + (2 * SIZEOF_STRUCT_MEM) + MEM_HEAP_ALIGNMENT;

    /* initialize the lowest-free pointer to the start of the heap */
    mem_heap.lfree = (struct mem *)mem_heap.ram;

    MEM_STATS_AVAIL(avail, mem_heap.mem_heap_size);

    if (sys_mutex_new() != 0) {
        MEM_HEAP_ASSERT("failed to create mem_mutex", 0);
    }

    /*malloc_stats();*/

    mem_heap.mem_init = 1;
}

/**
 * Put a struct mem back on the heap
 *
 * @param rmem is the data portion of a struct mem as returned by a previous
 *             call to mem_malloc()
 */
void free(void *rmem)
{
    struct mem *mem;
    MEM_HEAP_DECL_PROTECT();

    /*u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    printf("??????????????????????????free: %x, %x\n", rmem, rets_addr);*/

    if (rmem == (void *)0) {
//    MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("mem_free(p == NULL) was called.\n"));
        return;
    }
    MEM_HEAP_ASSERT("mem_free: sanity check alignment", (((mem_ptr_t)rmem) & (MEM_HEAP_ALIGNMENT - 1)) == 0);

//  MEM_HEAP_ASSERT("mem_free: legal memory", (u8_t *)rmem >= (u8_t *)mem_heap.ram &&
//    (u8_t *)rmem < (u8_t *)mem_heap.ram_end);

//  if ((u8_t *)rmem < (u8_t *)mem_heap.ram || (u8_t *)rmem >= (u8_t *)mem_heap.ram_end) {
//    MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("mem_free: illegal memory\n"));
//    /* protect mem stats from concurrent access */
//    MEM_HEAP_PROTECT();
//    MEM_STATS_INC(illegal);
//    MEM_HEAP_UNPROTECT();
//    return;
//  }
    /* protect the heap from concurrent access */
    MEM_HEAP_PROTECT();
    /* Get the corresponding struct mem ... */
    mem = (struct mem *)(void *)((u8_t *)rmem - SIZEOF_STRUCT_MEM);
#if MEM_HEAP_ERROR_CHECK == 1
    MEM_HEAP_ASSERT("mem_free: mem->check_before", mem->check_before == MEM_HEAP_OVERFLOW_MAGIC);
    MEM_HEAP_ASSERT("mem_free: mem->check_after", mem->check_after == MEM_HEAP_OVERFLOW_MAGIC);
    memset(rmem, MEM_HEAP_CLR_MAGIC, (u8_t *)mem->next - (u8_t *)rmem);
#endif
    /* ... which has to be in a used state ... */
    MEM_HEAP_ASSERT("mem_free: mem->used", mem->used == MEM_USED);
    /* ... and is now unused. */
    mem->used = MEM_UNUSED;

    if (mem < mem_heap.lfree) {
        /* the newly freed struct is now the lowest */
        mem_heap.lfree = mem;
    }

    MEM_STATS_DEC_USED(used, ((mem_size_t)mem->next - (mem_size_t)mem - SIZEOF_STRUCT_MEM));

    /* finally, see if prev or next are free also */
    mem = plug_holes(mem);

    MEM_HEAP_UNPROTECT();
}

void *get_malloc_caddr(u8 *addr, u32 offset)
{
#if MEM_HEAP_ERROR_CHECK
    struct mem *mem2 = (struct mem *)(addr - SIZEOF_STRUCT_MEM);

    return (void *)(&mem2->check_before);
#else
    return NULL;
#endif
}

static u8 call_from_zalloc = 0;

/**
 * Adam's mem_malloc() plus solution for bug #17922
 * Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @param size is the minimum size of the requested block in bytes.
 * @return pointer to allocated memory or NULL if no free memory was found.
 *
 * Note that the returned value will always be aligned (as defined by MEM_HEAP_ALIGNMENT).
 */
static u32 rets_addr;

void *malloc(unsigned long size)
{
    struct mem *mem, *mem2;
    struct mem *sbrk_lfree = 0;

    MEM_HEAP_DECL_PROTECT();

    if (call_from_zalloc == 0) {
        __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    }

    if (size == 0) {
        return (void *)0;
    }
    mem_heap_init();

    /* Expand the size of the allocated memory region so that we can
       adjust for alignment. */
    size = MEM_HEAP_ALIGN_SIZE(size);

    if (size < MIN_SIZE_ALIGNED) {
        /* every data block must be at least MIN_SIZE_ALIGNED long */
        size = MIN_SIZE_ALIGNED;
    }
    MEM_HEAP_PROTECT();

    if (size > mem_heap.mem_heap_size) {
        /*MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("malloc 0x%x > mem_heap_size(0x%x).\n", size, mem_heap.mem_heap_size));*/
        /* protect the heap from concurrent access */
        goto SBRK_MEM;
    }

MALLOC:
    /* Scan through the heap searching for a free block that is big enough,
     * beginning with the lowest free block.
     */
    for (sbrk_lfree ? (mem = sbrk_lfree) : (mem = mem_heap.lfree); mem != mem_heap.ram_end; mem = mem->next) {
        if ((mem->used == MEM_UNUSED) &&
            ((mem_ptr_t)(mem->next) - ((mem_ptr_t)mem + SIZEOF_STRUCT_MEM)) >= size) {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - ((mem_ptr_t)mem + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

            if ((mem_ptr_t)(mem->next) - ((mem_ptr_t)mem + SIZEOF_STRUCT_MEM) >= (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED)) {
                /* (in addition to the above, we test if another struct mem (SIZEOF_STRUCT_MEM) containing
                 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
                 * -> split large block, create empty remainder,
                 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
                 * mem->next - ((mem_ptr_t)mem + (2*SIZEOF_STRUCT_MEM)) == size,
                 * struct mem would fit in but no data between mem2 and mem2->next
                 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
                 *       region that couldn't hold data, but when mem->next gets freed,
                 *       the 2 regions would be combined, resulting in more free memory
                 */
                mem2 = (struct mem *)((mem_ptr_t)mem + SIZEOF_STRUCT_MEM + size);
                /* create mem2 struct */
                mem2->used = MEM_UNUSED;
                mem2->next = mem->next;
                mem2->prev = mem;
#if MEM_HEAP_ERROR_CHECK == 1
                mem2->check_after = MEM_HEAP_OVERFLOW_MAGIC;
                mem2->check_before = MEM_HEAP_OVERFLOW_MAGIC;
#endif
                /* and insert it between mem and mem->next */
                mem->next = mem2;
                mem->used = MEM_USED;
                mem2->next->prev = mem2;
                MEM_STATS_INC_USED(used, (size + SIZEOF_STRUCT_MEM));
            } else {
                /* (a mem2 struct does no fit into the user data space of mem and mem->next will always
                 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
                 * take care of this).
                 * -> near fit or excact fit: do not split, no mem2 creation
                 * also can't move mem->next directly behind mem, since mem->next
                 * will always be used at this point!
                 */
                mem->used = MEM_USED;

                MEM_STATS_INC_USED(used, ((mem_ptr_t)mem->next - (mem_ptr_t)mem - SIZEOF_STRUCT_MEM));
            }
            if (mem == mem_heap.lfree) {
                struct mem *cur = mem_heap.lfree;
                /* Find next free block after mem and update lowest free pointer */
                while ((cur->used == MEM_USED) && cur != mem_heap.ram_end) {
                    cur = cur->next;
                }
                mem_heap.lfree = cur;
                MEM_HEAP_ASSERT("mem_malloc: !mem_heap.lfree->used", ((mem_heap.lfree == mem_heap.ram_end) || (mem_heap.lfree->used == MEM_UNUSED)));
            }
            MEM_HEAP_UNPROTECT();
//        MEM_HEAP_ASSERT("mem_malloc: allocated memory not above ram_end.",
//         (mem_ptr_t)mem + SIZEOF_STRUCT_MEM + size <= (mem_ptr_t)mem_heap.ram_end);
            MEM_HEAP_ASSERT("mem_malloc: allocated memory properly aligned.",
                            ((mem_ptr_t)mem + SIZEOF_STRUCT_MEM) % MEM_HEAP_ALIGNMENT == 0);
            MEM_HEAP_ASSERT("mem_malloc: sanity check alignment",
                            (((mem_ptr_t)mem) & (MEM_HEAP_ALIGNMENT - 1)) == 0);

            /* printf("malloc:0x%x size:0x%x\n",(u8_t *)mem  + SIZEOF_STRUCT_MEM,size); */
            /*if (call_from_zalloc == 0) {
                printf("??????????????????????????malloc: %d, %x, %x\n", size, rets_addr,
                    (u8 *)mem + SIZEOF_STRUCT_MEM);
            }*/
            mem->rets_addr = rets_addr;
            /*printf("malloc: %x\n", (u8 *)mem + SIZEOF_STRUCT_MEM);*/
            return (u8_t *)mem + SIZEOF_STRUCT_MEM;
        }
    }

SBRK_MEM:
#define SBRK_PTR_ALIGN_FORWARD(ptr) (struct mem *)(((mem_ptr_t)ptr) & ~(mem_ptr_t)(MEM_HEAP_ALIGNMENT-1))
#define SBRK_PTR_ALIGN_BACKWARD(ptr) (struct mem *)(((mem_ptr_t)ptr + MEM_HEAP_ALIGNMENT - 1) & ~(mem_ptr_t)(MEM_HEAP_ALIGNMENT-1))

//    MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, (" +SBRK_MEM\r\n"));
    ;
    u8_t ram_end_prev_unused_flag;
    u8_t *sbrk_ptr;
    struct mem *sbrk_head = (struct mem *) - 1;
    struct mem *new_ram_end;
    mem_size_t mem_heap_incr_size;
    mem_size_t remain_size, need_size;

    need_size = size + 2 * SIZEOF_STRUCT_MEM + 2 * MEM_HEAP_ALIGNMENT;
    if (mem_heap.ram_end->prev->used == MEM_UNUSED) {
        ram_end_prev_unused_flag = 1;

        sbrk_ptr = sbrk(0);//detect sbrk_ptr
        if (mem_heap.sbrk_end != sbrk_ptr) { //discontinuous
            remain_size = 0;
        } else {
            remain_size = (mem_size_t)mem_heap.ram_end - ((mem_size_t)(mem_heap.ram_end->prev) + SIZEOF_STRUCT_MEM);
        }

        mem_heap_incr_size = need_size - remain_size;

    } else {
        ram_end_prev_unused_flag = 0;
        mem_heap_incr_size = need_size;
    }

    sbrk_ptr = sbrk(mem_heap_incr_size);
    if (sbrk_ptr == (void *) - 1) {
        MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("1sbrk ret == NULL,sbrk_malloc: could not allocate %d bytes\n", (mem_size_t)mem_heap_incr_size));
        MEM_STATS_INC(err);
        MEM_HEAP_UNPROTECT();

        malloc_dump();

        return (void *)0;
    }
    if (mem_heap.sbrk_end != sbrk_ptr) { //discontinuous
        sbrk_head = SBRK_PTR_ALIGN_BACKWARD(sbrk_ptr);

        sbrk_head->prev = mem_heap.ram_end;
        sbrk_head->used = MEM_UNUSED;
#if MEM_HEAP_ERROR_CHECK == 1
        sbrk_head->check_after = MEM_HEAP_OVERFLOW_MAGIC;
        sbrk_head->check_before = MEM_HEAP_OVERFLOW_MAGIC;
#endif
        mem_heap.ram_end->next = sbrk_head;
    }

    mem_heap.sbrk_end = sbrk_ptr + mem_heap_incr_size;

    new_ram_end = (struct mem *)((u8_t *)(SBRK_PTR_ALIGN_FORWARD(mem_heap.sbrk_end)) - SIZEOF_STRUCT_MEM);
    new_ram_end->used = MEM_USED;
    new_ram_end->next = new_ram_end;
#if MEM_HEAP_ERROR_CHECK == 1
    new_ram_end->check_after = MEM_HEAP_OVERFLOW_MAGIC;
    new_ram_end->check_before = MEM_HEAP_OVERFLOW_MAGIC;
#endif
    if (sbrk_head == (struct mem *) - 1) { //continuous
        if (ram_end_prev_unused_flag) {
            new_ram_end->prev = mem_heap.ram_end->prev;
            mem_heap.ram_end->prev->next = new_ram_end;
        } else {
            new_ram_end->prev = mem_heap.ram_end;
            mem_heap.ram_end->next = new_ram_end;
            mem_heap.ram_end->used = MEM_UNUSED;
        }
    } else {
        new_ram_end->prev = sbrk_head;
        sbrk_head->next = new_ram_end;
    }
    mem_heap.ram_end = new_ram_end;

    mem_heap.mem_heap_size += mem_heap_incr_size;
    MEM_STATS_AVAIL(avail, mem_heap.mem_heap_size);

    sbrk_lfree = new_ram_end->prev;

    goto MALLOC;
}

/**
 * Contiguously allocates enough space for count objects that are size bytes
 * of memory each and returns a pointer to the allocated memory.
 *
 * The allocated memory is filled with bytes of value zero.
 *
 * @param count number of objects to allocate
 * @param size size of the objects to allocate
 * @return pointer to allocated memory / NULL pointer if there is an error
 */
void *calloc(unsigned long count, unsigned long size)
{
    void *p;

    /* allocate 'count' objects of size 'size' */
    p = malloc(count * size);
    if (p) {
        /* zero the memory */
        memset(p, 0, count * size);
    }
    return p;
}

void *zalloc(unsigned long size)
{
    void *p;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));

    call_from_zalloc = 1;
    p = malloc(size);
    /*printf("??????????????????????????malloc: %d, %x, %x\n", size, rets_addr, p);*/
    call_from_zalloc = 0;
    if (p) {
        memset(p, 0, size);
    }
    return p;
}

void *realloc(void *rmem, unsigned long newsize)
{
    mem_size_t size;
    struct mem *new_mem;
    struct mem *mem, *mem2;
    void *new_rmem = (void *)0;
    /* use the FREE_PROTECT here: it protects with sem OR SYS_ARCH_PROTECT */
    MEM_HEAP_DECL_PROTECT();

    /* Expand the size of the allocated memory region so that we can
       adjust for alignment. */
    newsize = MEM_HEAP_ALIGN_SIZE(newsize);

    if (newsize < MIN_SIZE_ALIGNED) {
        /* every data block must be at least MIN_SIZE_ALIGNED long */
        newsize = MIN_SIZE_ALIGNED;
    }

    mem_heap_init();

    if (rmem == (void *)0) {
        rmem = malloc(newsize);
        return rmem;
    }

//  MEM_HEAP_ASSERT("realloc: legal memory", ((u8_t *)rmem >= (u8_t *)mem_heap.ram &&
//   (u8_t *)rmem < (u8_t *)mem_heap.ram_end));
//
//  if ((u8_t *)rmem < (u8_t *)mem_heap.ram || (u8_t *)rmem >= (u8_t *)mem_heap.ram_end) {
//    MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("realloc: illegal memory\n"));
//    /* protect mem stats from concurrent access */
//    MEM_HEAP_PROTECT();
//    MEM_STATS_INC(illegal);
//    MEM_HEAP_UNPROTECT();
//    return rmem;
//  }
#define USE_FIX_MALLOC 1

    /* protect the heap from concurrent access */
    MEM_HEAP_PROTECT();

    /* Get the corresponding struct mem ... */
    mem = (struct mem *)(void *)((u8_t *)rmem - SIZEOF_STRUCT_MEM);
    /* ... and its offset pointer */
    size = (mem_size_t)(mem->next) - (mem_size_t)mem - SIZEOF_STRUCT_MEM;
#if USE_FIX_MALLOC == 0
    MEM_HEAP_UNPROTECT();
#endif
    if (newsize > size) {
#if USE_FIX_MALLOC == 1
        if ((mem->next->used == MEM_UNUSED) && ((mem_size_t)(mem->next->next) - (mem_size_t)mem - SIZEOF_STRUCT_MEM >= newsize)) {
            new_mem = (struct mem *)((mem_size_t)mem + SIZEOF_STRUCT_MEM + newsize);
            if ((mem_size_t)(mem->next->next) - (mem_size_t)new_mem  >= SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED) {
                if (mem_heap.lfree == mem->next) {
                    mem_heap.lfree = new_mem;
                }
                if (mem_heap.ram_end->prev == mem->next) {
                    mem_heap.ram_end->prev = new_mem;
                }
                mem->next->next->prev = new_mem;
                new_mem->next = mem->next->next;
                new_mem->prev = mem;
                new_mem->used = MEM_UNUSED;
#if MEM_HEAP_ERROR_CHECK == 1
                new_mem->check_after = MEM_HEAP_OVERFLOW_MAGIC;
                new_mem->check_before = MEM_HEAP_OVERFLOW_MAGIC;
#endif
                mem->next = new_mem;

                MEM_STATS_INC_USED(used, (newsize - size));
            } else {
                MEM_STATS_INC_USED(used, ((mem_size_t)mem->next->next - (mem_size_t)mem->next - SIZEOF_STRUCT_MEM));
                if (mem_heap.lfree == mem->next) {
                    struct mem *cur = mem->next->next;
                    while ((cur->used == MEM_USED) && cur != mem_heap.ram_end) {
                        cur = cur->next;
                    }
                    mem_heap.lfree = cur;
                }
                if (mem_heap.ram_end->prev == mem->next) {
                    mem_heap.ram_end->prev = mem;
                }
                mem->next->next->prev = mem;
                mem->next = mem->next->next;
            }

            MEM_HEAP_UNPROTECT();
            return rmem;
        } else {
            MEM_HEAP_UNPROTECT();
            new_rmem = malloc(newsize);
            if (new_rmem != (void *)0) {
                memmove(new_rmem, rmem, size);
                free(rmem);
                rmem = new_rmem;
                return rmem;
            } else {
                MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("mem_realloc: could not allocate %d bytes\n", (mem_size_t)newsize));
                return (void *)0;
            }
        }
#else
        new_rmem = malloc(newsize);
        if (new_rmem != (void *)0) {
            memmove(new_rmem, rmem, size);
            free(rmem);
            rmem = new_rmem;
            return rmem;
        } else {
            MEM_HEAP_DEBUGF(MEM_HEAP_DEBUG, ("mem_realloc: could not allocate %d bytes\n", (mem_size_t)newsize));
            return (void *)0;
        }
#endif
    } else if (newsize < size) {
#if USE_FIX_MALLOC == 1
        mem2 = mem->next;
        if (mem2->used == MEM_UNUSED) {
            /* The next struct is unused, we can simply move it at little */
            struct mem *next;
            /* remember the old next pointer */
            next = mem2->next;
            /* create new struct mem which is moved directly after the shrinked mem */
            new_mem = (struct mem *)((mem_size_t)mem + SIZEOF_STRUCT_MEM + newsize);
            if (mem_heap.lfree == mem2) {
                mem_heap.lfree = new_mem;
            }
            if (mem_heap.ram_end->prev == mem2) {
                mem_heap.ram_end->prev = new_mem;
            }
#if MEM_HEAP_ERROR_CHECK == 1
            new_mem->check_after = MEM_HEAP_OVERFLOW_MAGIC;
            new_mem->check_before = MEM_HEAP_OVERFLOW_MAGIC;
#endif
            mem2 = new_mem;
            mem2->used = MEM_UNUSED;
            /* restore the next pointer */
            mem2->next = next;
            /* link it back to mem */
            mem2->prev = mem;
            /* link mem to it */
            mem->next = mem2;
            /* last thing to restore linked list: as we have moved mem2,
             * let 'mem2->next->prev' point to mem2 again. but only if mem2->next is not
             * the end of the heap */
            if (mem2->next != mem_heap.ram_end) { //imposible?
                mem2->next->prev = mem2;
            }

            MEM_STATS_DEC_USED(used, (size - newsize));
            /* no need to plug holes, we've already done that */
        } else if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED <= size) {
            /* Next struct is used but there's room for another struct mem with
             * at least MIN_SIZE_ALIGNED of data.
             * Old size ('size') must be big enough to contain at least 'newsize' plus a struct mem
             * ('SIZEOF_STRUCT_MEM') with some data ('MIN_SIZE_ALIGNED').
             * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
             *       region that couldn't hold data, but when mem->next gets freed,
             *       the 2 regions would be combined, resulting in more free memory */
            new_mem = (struct mem *)((mem_size_t)mem + SIZEOF_STRUCT_MEM + newsize);
#if MEM_HEAP_ERROR_CHECK == 1
            new_mem->check_after = MEM_HEAP_OVERFLOW_MAGIC;
            new_mem->check_before = MEM_HEAP_OVERFLOW_MAGIC;
#endif
            mem2 = new_mem;
            if (mem2 < mem_heap.lfree) {
                mem_heap.lfree = mem2;
            }
            mem2->used = MEM_UNUSED;
            mem2->next = mem->next;
            mem2->prev = mem;
            mem->next = mem2;
            if (mem2->next != mem_heap.ram_end) {
                mem2->next->prev = mem2;
            }
            if (mem_heap.ram_end->prev == mem) {
                mem_heap.ram_end->prev = mem2;
            }
            MEM_STATS_DEC_USED(used, (size - newsize - SIZEOF_STRUCT_MEM));
            /* the original mem->next is used, so no need to plug holes! */
        }
        /* else {
          next struct mem is used but size between mem and mem2 is not big enough
          to create another struct mem
          -> don't do anyhting.
          -> the remaining space stays unused since it is too small
        } */
        MEM_HEAP_UNPROTECT();
        return rmem;
#else
        return rmem;
#endif
    } else if (newsize == size) {
#if USE_FIX_MALLOC == 1
        MEM_HEAP_UNPROTECT();
#endif
        /* No change in size, simply return */
        return rmem;
    }

    return (void *)0;
}

#if MEM_HEAP_ERROR_CHECK == 1
void mem_heap_check(const char *const func, unsigned int line)
{
    MEM_HEAP_DECL_PROTECT();
    MEM_HEAP_PROTECT();

    struct mem *mem;

    for (mem = (struct mem *)mem_heap.ram; mem != mem_heap.ram_end->prev; mem = mem->next) {
        if (((u32_t)mem > (u32_t)mem_heap.ram + mem_heap.mem_heap_size + SIZEOF_STRUCT_MEM)
            || ((u32_t)mem < (u32_t)mem_heap.ram)
            || ((u32_t)mem->next > (u32_t)mem_heap.ram + mem_heap.mem_heap_size + SIZEOF_STRUCT_MEM)
            || ((u32_t)mem->next < (u32_t)mem_heap.ram)
            || ((u32_t)mem->prev > (u32_t)mem_heap.ram + mem_heap.mem_heap_size + SIZEOF_STRUCT_MEM)
            || ((u32_t)mem->prev < (u32_t)mem_heap.ram)
            || ((mem->used != MEM_USED) && (mem->used != MEM_UNUSED))
            || (mem->check_before != MEM_HEAP_OVERFLOW_MAGIC)
            || (mem->check_after  != MEM_HEAP_OVERFLOW_MAGIC)
           ) {
            printf("{%s, %d", func, line);

            printf("\n\n\n\n\n\n\n\nmem_heap was maliciously modified:\n"\
                   "mem_heap.ram = 0x%x\n"\
                   "mem_heap.mem_heap_size = 0x%x\n"\
                   "mem = 0x%x\n"\
                   "mem->next = 0x%x\n"\
                   "mem->prev = 0x%x\n"\
                   "mem->used = 0x%x\n"\
                   "mem->rets_addr = 0x%x\n"\
                   "mem->check_before = 0x%x\n"\
                   "mem->check_after = 0x%x\n\n\n\n\n\n\n\n\n", \
                   (u32_t)mem_heap.ram, mem_heap.mem_heap_size, (u32_t)mem, (u32_t)mem->next, (u32_t)mem->prev,
                   mem->used, mem->rets_addr, mem->check_before, mem->check_after);
            while (1);
        }
    }

//    if(func)
//    {
//       puts("}");
//    }

    MEM_HEAP_UNPROTECT();
}
#endif

#define SBRK_INTERNAL

#ifdef SBRK_INTERNAL


static void *sbrk(int increment)
{
    static unsigned char *__init_addr = 0;
    unsigned char *ret;

    if (__init_addr == 0) {
        __init_addr = (u8 *)&HEAP_BEGIN;
    }

    if ((__init_addr + increment >= (u8 *)&HEAP_END) && (increment > 0)) {
        return (void *) - 1;
    }

    ret = __init_addr;
    __init_addr += increment;

//    printf("increment = %x sbrk %x %x %x", increment , ret ,&HEAP_BEGIN ,&HEAP_END ) ;

    return ret;
}
#endif

void *pvPortMalloc(unsigned long  xWantedSize)
{
    return malloc(xWantedSize);
}

void vPortFree(void *pv)
{
    free(pv);
}

void *kmalloc(unsigned long size, int flags)
{
    return malloc(size);
}

void *vmalloc(unsigned long size)
{
    return malloc(size);
}

void vfree(void *addr)
{
    free(addr);
}

void *kzalloc(unsigned int len, int a)
{
    return calloc(1, len);
}

void kfree(void *p)
{
    free(p);
}

void memory_init(void)
{

}
