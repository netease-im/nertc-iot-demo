/**
 * @file lv_os_jieli.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_os.h"

#if LV_USE_OS == LV_OS_CUSTOM

#include <errno.h>
#include "../misc/lv_log.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_result_t lv_thread_init(lv_thread_t *thread, lv_thread_prio_t prio, void (*callback)(void *), size_t stack_size,
                           void *user_data)
{
    size_t os_prio;
    switch (prio) {
    case LV_THREAD_PRIO_LOWEST:
        os_prio = 2;
        break;
    case LV_THREAD_PRIO_LOW:
        os_prio = 5;
        break;
    case LV_THREAD_PRIO_MID:
        os_prio = 15;
        break;
    case LV_THREAD_PRIO_HIGH:
        os_prio = 25;
        break;
    case LV_THREAD_PRIO_HIGHEST:
        os_prio = 29;
        break;
    default:
        os_prio = 15;
        break;
    }

    if (thread_fork(NULL, os_prio, stack_size, 0, &thread->pid, callback, user_data)) {
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_thread_delete(lv_thread_t *thread)
{
    thread_kill(&thread->pid, KILL_WAIT);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_init(lv_mutex_t *mutex)
{
    os_mutex_create(mutex);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock(lv_mutex_t *mutex)
{
    os_mutex_pend(mutex, 0);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock_isr(lv_mutex_t *mutex)
{
    LV_LOG_ERROR("Error: %d", LV_RESULT_INVALID);
}

lv_result_t lv_mutex_unlock(lv_mutex_t *mutex)
{
    os_mutex_post(mutex);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_delete(lv_mutex_t *mutex)
{
    os_mutex_del(mutex, 0);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_init(lv_thread_sync_t *sync)
{
    os_sem_create(&sync->sem, 0);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_wait(lv_thread_sync_t *sync)
{
    os_sem_pend(&sync->sem, 0);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_signal(lv_thread_sync_t *sync)
{
    os_sem_post(&sync->sem);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_delete(lv_thread_sync_t *sync)
{
    os_sem_del(&sync->sem, 0);
    return LV_RESULT_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#endif /*LV_USE_OS == LV_OS_CUSTOM*/
