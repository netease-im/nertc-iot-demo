#include "volc_mutex.h"

//#include <pthread.h>
#include "os/os_api.h"
#include "volc_errno.h"
#include "volc_memory.h"
#include "volc_type.h"

/* 最好不要依赖可重入锁 */
volc_mutex_t volc_mutex_create(bool reentrant)
{
//    volc_debug("%s\n", __FUNCTION__);
    OS_MUTEX *mutex = (OS_MUTEX *)malloc(sizeof(OS_MUTEX));;
    int ret = os_mutex_create(mutex);
    if (ret != 0) {
        os_mutex_free(mutex);
        volc_debug("mutex create error!");
        return NULL;
    }
    return (volc_mutex_t)mutex;
}

void volc_mutex_lock(volc_mutex_t mutex)
{
//    volc_debug("%s\n", __FUNCTION__);
    os_mutex_pend((OS_MUTEX *)mutex, 0);
}

bool volc_mutex_trylock(volc_mutex_t mutex)
{
//    volc_debug("%s\n", __FUNCTION__);
    os_mutex_accept((OS_MUTEX *)mutex);
}

void volc_mutex_unlock(volc_mutex_t mutex)
{
//    volc_debug("%s\n", __FUNCTION__);
    os_mutex_post((OS_MUTEX *) mutex);
}

void volc_mutex_destroy(volc_mutex_t mutex)
{
//    volc_debug("%s\n", __FUNCTION__);
    OS_MUTEX *p_mutex = (OS_MUTEX *)mutex;
    if (NULL == p_mutex) {
        return;
    }
    os_mutex_del(p_mutex, OS_DEL_ALWAYS);
    free(p_mutex);
}
