#include "volc_memory.h"
#include <stdlib.h>
#include "volc_type.h"

__attribute__((__always_inline__))
void *volc_malloc(size_t size)
{
    void *ptr = malloc(size);
    return ptr;
}
__attribute__((__always_inline__))
void *volc_align_alloc(size_t size, size_t alignment)
{
    volc_debug("volc align alloc alignment:%d \n", alignment);
    printf("volc align alloc alignment:%d \n", alignment);

    void *ptr = NULL;
    posix_memalign(&ptr, alignment, size);
    return ptr;
}
__attribute__((__always_inline__))
void *volc_calloc(size_t num, size_t size)
{
    size_t total = num * size;
    void *ptr = volc_malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}
__attribute__((__always_inline__))
void *volc_realloc(void *ptr, size_t new_size)
{
    if (ptr == NULL) {
        ptr = malloc(new_size);
        return ptr;
    } else {
        void *new_ptr = realloc(ptr, new_size);
        return new_ptr;
    }

}

__attribute__((__always_inline__))
void volc_free(void *ptr)
{
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    if (!ptr) {
        // printf("??????????????????????????free: %x, %x\n", ptr, rets_addr);
    }

    free(ptr);
}

__attribute__((__always_inline__))
bool volc_memory_check(void *ptr, uint8_t val, size_t size)
{

    if (!ptr || size == 0) {
        return false;
    }

    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        if (p[i] != val) {
            return false;
        }
    }
    return true;
}
