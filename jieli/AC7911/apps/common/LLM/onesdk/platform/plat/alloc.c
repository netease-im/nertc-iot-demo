#include <string.h>
#include "alloc.h"

static void *s_non_aligned_malloc(struct aws_allocator *allocator, size_t size)
{
    (void)allocator;
    void *result = malloc(size);
    return result;
}

static void s_non_aligned_free(struct aws_allocator *allocator, void *ptr)
{
    (void)allocator;
    free(ptr);
}

static void *s_non_aligned_realloc(struct aws_allocator *allocator, void *ptr, size_t oldsize, size_t newsize)
{
    (void)allocator;
    (void)oldsize;

    if (newsize <= oldsize) {
        return ptr;
    }

    /* newsize is > oldsize, need more memory */
    void *new_mem = s_non_aligned_malloc(allocator, newsize);

    if (ptr) {
        memcpy(new_mem, ptr, oldsize);
        s_non_aligned_free(allocator, ptr);
    }

    return new_mem;
}

static void *s_non_aligned_calloc(struct aws_allocator *allocator, size_t num, size_t size)
{
    (void)allocator;
    void *mem = calloc(num, size);
    return mem;
}


static struct aws_allocator _allocator = {
    .mem_acquire = s_non_aligned_malloc,
    .mem_release = s_non_aligned_free,
    .mem_realloc = s_non_aligned_realloc,
    .mem_calloc = s_non_aligned_calloc,
};

struct aws_allocator *plat_aws_alloc(void)
{
    return &_allocator;
}
