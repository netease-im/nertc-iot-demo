/**
 * File:   application.c
 * Author: AWTK Develop Team
 * Brief:  application
 *
 * Copyright (c) 2020 - 2021  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2021-06-15 Liu YuXin <liuyuxin@zlg.cn> created
 *
 */

#include "awtk.h"
#include "mvvm/mvvm.h"
#include "mvvm/base/view_model_array_object_wrapper.h"
#include "csv/csv_file_object.h"
#include "table_view_register.h"
#include "table_view_mvvm_register.h"

static view_model_t *csv_view_model_create(navigator_request_t *req)
{
    char path[MAX_PATH + 1] = {0};
    fs_get_exe(os_fs(), path);
    return_value_if_fail(tk_strlen(path) > 0, NULL);

    char *bin = strstr(path, "bin");
    tk_strcpy(bin, "data/scores.csv");

    csv_file_t *file = csv_file_create(path, ',');

    if (file != NULL) {
        object_t *obj = csv_file_object_create(file);
        if (obj != NULL) {
            view_model_t *view_model = view_model_array_object_wrapper_create(obj);
            return view_model;
        }
        csv_file_destroy(file);
    }

    return NULL;
}

ret_t application_init()
{
    table_view_register();
    table_view_mvvm_register();

    view_model_factory_register("csv_view", csv_view_model_create);

    return navigator_to("csv_view");
}

ret_t application_exit()
{
    log_debug("application_exit\n");
    return RET_OK;
}

#include "../main.inc"
