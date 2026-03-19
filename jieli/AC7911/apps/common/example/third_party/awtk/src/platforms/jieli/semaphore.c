/**
 * File:   semaphore_null.c
 * Author: AWTK Develop Team
 * Brief:  semaphore do nothing
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
 * 2020-05-10 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/semaphore.h"
#include "tkc/mem.h"
#include "os/os_api.h"

struct _tk_semaphore_t {
    OS_SEM sem;
};

tk_semaphore_t *TK_WEAK tk_semaphore_create(uint32_t value, const char *name)
{
    tk_semaphore_t *semaphore = TKMEM_ALLOC(sizeof(tk_semaphore_t));
    return_value_if_fail(semaphore != NULL, NULL);
    if (os_sem_create(&semaphore->sem, value)) {
        TKMEM_FREE(semaphore);
    }

    return semaphore;
}

ret_t tk_semaphore_wait(tk_semaphore_t *semaphore, uint32_t timeout_ms)
{
    return_value_if_fail(semaphore != NULL, RET_BAD_PARAMS);

    if (timeout_ms) {
        timeout_ms /= (1000 / configTICK_RATE_HZ);
        if (timeout_ms == 0) {
            timeout_ms = 1;
        }
    }

    if (os_sem_pend(&semaphore->sem, timeout_ms) != 0) {
        return RET_TIMEOUT;
    }

    return RET_OK;
}

ret_t tk_semaphore_post(tk_semaphore_t *semaphore)
{
    return_value_if_fail(semaphore != NULL, RET_BAD_PARAMS);

    return_value_if_fail(os_sem_post(&semaphore->sem) == 0, RET_FAIL);

    return RET_OK;
}

ret_t tk_semaphore_destroy(tk_semaphore_t *semaphore)
{
    return_value_if_fail(semaphore != NULL, RET_BAD_PARAMS);

    os_sem_del(&semaphore->sem, 0);
    TKMEM_FREE(semaphore);

    return RET_OK;
}
