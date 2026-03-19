#include "volc_thread.h"

#include <unistd.h>
#include "pthread.h"
#include "os/os_api.h"
#include "volc_errno.h"
#include "volc_memory.h"
#include "volc_time.h"
#include "volc_type.h"

volc_thread_local_t g_thread_local = NULL;

static void _volc_thread_local_delete_key(volc_thread_local_t local)
{
    // volc_debug("%s\n", __FUNCTION__);
    (void)local;
}

volc_tid_t volc_thread_get_id(void)
{
    // volc_debug("%s\n", __FUNCTION__);
    return (volc_tid_t)get_cur_thread_pid();
}


uint32_t volc_thread_set_name(const char *name)
{
    volc_debug("%s, %s\n", __FUNCTION__, name);
    (void)name;
    return VOLC_SUCCESS;
}

#include "spinlock.h"

DEFINE_SPINLOCK(volc_lock);
static int volc_thread_pid[8];

int *get_volc_thread_pid(void)
{
    spin_lock(&volc_lock);
    int i = 0;
    for (i = 0; i < 8; i++) {
        if (volc_thread_pid[i] == 0) {
            volc_thread_pid[i]  = !0;
            break;
        }
    }
    if (i == 8) {

        spin_unlock(&volc_lock);
        return NULL;
    }
    spin_unlock(&volc_lock);
    return  &volc_thread_pid[i];

}

uint32_t volc_thread_create(volc_tid_t *thread, const volc_thread_param_t *param, void *(*start_routine)(void *), void *args)
{
    int ret = 0;
    int stack_size = 0;
    int priority = 0;

    if (NULL == thread || NULL == start_routine) {
        return VOLC_FAILED;
    }

    volc_debug("%s %d", __FUNCTION__, __LINE__);
    if (NULL != param) {
        if (param->priority == -1) {
            priority = 5;
        } else {
            priority = param->priority;
        }
        if (param->stack_size == -1) {
            stack_size = 8192;
        } else {
            stack_size = param->stack_size;
        }
    } else {
        stack_size = 8192;
        priority = 3;
    }
    volc_debug("%s %d", __FUNCTION__, __LINE__);
    volc_debug("name:%s prio:%d, stack size:%d\n", param->name, priority, stack_size);
    int *handle = NULL;
    handle = get_volc_thread_pid();
    ret = thread_fork(param->name, priority, (stack_size) / sizeof(int), 0, handle, (void (*)(void *))start_routine, args);
    *thread = handle;
    if (0 != ret) {
        volc_debug("%s %d", __FUNCTION__, __LINE__);
        return VOLC_FAILED;
    }
    return VOLC_SUCCESS;
}


void volc_thread_destroy(volc_tid_t thread)
{
    // volc_debug("%s\n", __FUNCTION__);
    if (NULL == thread) {
        return;
    }
}

void volc_thread_exit(volc_tid_t thread)
{
    if (NULL == thread) {
        return;
    }
    thread_kill(thread, KILL_WAIT);
}

void volc_thread_sleep(uint64_t time)
{
    // volc_debug("%s\n", __FUNCTION__);
    uint64_t remaining_time = time / VOLC_HUNDREDS_OF_NANOS_IN_A_MICROSECOND;

    while (0 != remaining_time) {
        if (remaining_time <= VOLC_MAX_UINT32) {
            usleep(remaining_time);
            remaining_time = 0;
        } else {
            usleep(VOLC_MAX_UINT32);
            remaining_time -= (uint64_t)VOLC_MAX_UINT32;
        }
    }

}

void volc_thread_sleep_until(uint64_t time)
{
    // volc_debug("%s\n", __FUNCTION__);
    uint64_t cur_time = volc_get_time_ms() * VOLC_HUNDREDS_OF_NANOS_IN_A_MILLISECOND;
    if (time > cur_time) {
        volc_thread_sleep(time - cur_time);
    }
}

uint32_t volc_thread_join(volc_tid_t thread, void *ret)
{
    // volc_debug("%s\n", __FUNCTION__);
    (void)thread;
    (void)ret;
    return VOLC_SUCCESS;
}


uint32_t volc_thread_detach(volc_tid_t thread)
{
    // volc_debug("%s\n", __FUNCTION__);
    (void)thread;
    return VOLC_SUCCESS;
}

volc_thread_local_t volc_thread_local_create(void)
{
    // // volc_debug("%s\n", __FUNCTION__);
//    return NULL;
    pthread_key_t *key = (pthread_key_t *)volc_calloc(1, sizeof(pthread_key_t));
    if (NULL == key) {
        return NULL;
    }
    pthread_key_create(key, _volc_thread_local_delete_key);
    volc_debug("%s key:0x%x \n", __FUNCTION__, key);
    return (volc_thread_local_t)key;
}

uint32_t volc_thread_local_destroy(volc_thread_local_t local)
{
    // volc_debug("%s\n", __FUNCTION__);
    return VOLC_FAILED;
    if (NULL == local) {
        return VOLC_FAILED;
    }
    pthread_key_delete(*(pthread_key_t *)local);
    volc_free(local);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_local_set(volc_thread_local_t local, void *data)
{
    // volc_debug("%s\n", __FUNCTION__);
    if (NULL == local || NULL == data) {
        return VOLC_FAILED;
    }
    pthread_setspecific(*(pthread_key_t *)local, data);
    return VOLC_SUCCESS;
    // return VOLC_FAILED;
}

uint32_t volc_thread_local_clean(volc_thread_local_t local)
{
    // volc_debug("%s\n", __FUNCTION__);
    if (NULL == local) {
        return VOLC_FAILED;
    }
    pthread_setspecific(*(pthread_key_t *)local, NULL);
    return VOLC_SUCCESS;
    // return VOLC_FAILED;
}

void *volc_thread_local_get(volc_thread_local_t local)
{
    // volc_debug("%s\n", __FUNCTION__);
    return NULL;
    if (NULL == local) {
        return NULL;
    }
    return pthread_getspecific(*(pthread_key_t *)local);
}
