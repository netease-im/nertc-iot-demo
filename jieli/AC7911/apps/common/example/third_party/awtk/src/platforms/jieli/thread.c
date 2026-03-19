/**
 * File:   thread.c
 * Author: AWTK Develop Team
 * Brief:  thread on rtthread
 *
 * Copyright (c) 2018 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2019-11-08 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "os/os_api.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/thread.h"
#include "tkc/mutex.h"

struct _tk_thread_t {
    void *args;
    tk_thread_entry_t entry;

    int task;
    bool_t running;

    const char *name;
    uint32_t stack_size;
    int32_t priority;
//  tk_mutex_t* mutex;
};

ret_t tk_thread_set_name(tk_thread_t *thread, const char *name)
{
    return_value_if_fail(thread != NULL && name != NULL, RET_BAD_PARAMS);

    thread->name = name;

    return RET_OK;
}

ret_t tk_thread_set_stack_size(tk_thread_t *thread, uint32_t stack_size)
{
    return_value_if_fail(thread != NULL, RET_BAD_PARAMS);

    thread->stack_size = stack_size;

    return RET_OK;
}

static int32_t tk_thread_get_priority_from_platform(tk_thread_priority_t priority)
{
    int32_t prio;
    switch (priority) {
    case TK_THREAD_PRIORITY_TIME_CRITICAL:
        prio = 28;
        break;
    case TK_THREAD_PRIORITY_HIGHEST:
        prio = 27;
        break;
    case TK_THREAD_PRIORITY_ABOVE_NORAML:
        prio = 25;
        break;
    case TK_THREAD_PRIORITY_NORMAL:
        prio = 22;
        break;
    case TK_THREAD_PRIORITY_BELOW_NORAML:
        prio = 10;
        break;
    case TK_THREAD_PRIORITY_LOWEST:
        prio = 2;
        break;
    default:
        prio = 20;
        break;
    }
    return prio;
}

ret_t tk_thread_set_priority(tk_thread_t *thread, tk_thread_priority_t priority)
{
    return_value_if_fail(thread != NULL, RET_BAD_PARAMS);
    thread->priority = tk_thread_get_priority_from_platform(priority);
    if (thread->running) {
        assert(0);
//    rt_thread_control(thread->task, RT_THREAD_CTRL_CHANGE_PRIORITY, &thread->priority);
    }

    return RET_OK;
}

void *tk_thread_get_args(tk_thread_t *thread)
{
    return_value_if_fail(thread != NULL, NULL);

    return thread->args;
}

static void tk_thread_entry(void *arg)
{
    tk_thread_t *thread = (tk_thread_t *)arg;

//  tk_mutex_lock(thread->mutex);
    thread->running = TRUE;
    thread->entry(thread->args);
    thread->running = FALSE;
//  tk_mutex_unlock(thread->mutex);
}

tk_thread_t *tk_thread_create(tk_thread_entry_t entry, void *args)
{
    tk_thread_t *thread = NULL;
    return_value_if_fail(entry != NULL, NULL);

    thread = (tk_thread_t *)TKMEM_ZALLOC(tk_thread_t);
    return_value_if_fail(thread != NULL, NULL);

    thread->args = args;
    thread->entry = entry;
//  thread->mutex = tk_mutex_create();

    return thread;
}

ret_t tk_thread_start(tk_thread_t *thread)
{
    return_value_if_fail(thread != NULL && !thread->running, RET_BAD_PARAMS);

    thread_fork(thread->name, thread->priority, thread->stack_size, 0, &thread->task, tk_thread_entry, thread);

    if (thread->task == 0) {
        return RET_FAIL;
    } else {
        return RET_OK;
    }
}

//ret_t tk_thread_join(tk_thread_t* thread) {assert(0);
//  return_value_if_fail(thread != NULL, RET_BAD_PARAMS);
//
////  return_value_if_fail(tk_mutex_lock(thread->mutex) == RET_OK, RET_FAIL);
////  tk_mutex_unlock(thread->mutex);
//
//  return RET_OK;
//}

ret_t tk_thread_destroy(tk_thread_t *thread)
{
    return_value_if_fail(thread != NULL && &(thread->task), RET_BAD_PARAMS);

//  if (thread->mutex != NULL) {
//    tk_mutex_destroy(thread->mutex);
//  }

    thread_kill(&thread->task, KILL_WAIT);
    TKMEM_FREE(thread);

    return RET_OK;
}

uint64_t tk_thread_self(void)
{
    return (uint64_t)get_cur_thread_pid();
}
