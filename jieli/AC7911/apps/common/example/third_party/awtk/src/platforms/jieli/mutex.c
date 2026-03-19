/**
 * File:   mutex.c
 * Author: AWTK Develop Team
 * Brief:  mutex base on rtthread
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
#include "tkc/mutex.h"
#include "tkc/utils.h"
#include "tkc/time_now.h"

struct _tk_mutex_t {
    OS_MUTEX mtx;
};

tk_mutex_t *tk_mutex_create()
{
    tk_mutex_t *mutex = TKMEM_ALLOC(sizeof(tk_mutex_t));
    return_value_if_fail(mutex != NULL, NULL);

    if (os_mutex_create(&mutex->mtx)) {
        TKMEM_FREE(mutex);
    }

    return mutex;
}

ret_t tk_mutex_lock(tk_mutex_t *mutex)
{
    return_value_if_fail(mutex != NULL, RET_BAD_PARAMS);

    return_value_if_fail(os_mutex_pend(&mutex->mtx, 0) == 0, RET_FAIL);

    return RET_OK;
}

ret_t tk_mutex_try_lock(tk_mutex_t *mutex)
{
    return_value_if_fail(mutex != NULL, RET_BAD_PARAMS);

    return_value_if_fail(os_mutex_accept(&mutex->mtx) == 0, RET_FAIL);

    return RET_OK;
}

ret_t tk_mutex_unlock(tk_mutex_t *mutex)
{
    return_value_if_fail(mutex != NULL, RET_BAD_PARAMS);

    return_value_if_fail(os_mutex_post(&mutex->mtx) == 0, RET_FAIL);

    return RET_OK;
}

ret_t tk_mutex_destroy(tk_mutex_t *mutex)
{
    return_value_if_fail(mutex != NULL, RET_BAD_PARAMS);

    os_mutex_del(&mutex->mtx, 0);
    TKMEM_FREE(mutex);

    return RET_OK;
}
