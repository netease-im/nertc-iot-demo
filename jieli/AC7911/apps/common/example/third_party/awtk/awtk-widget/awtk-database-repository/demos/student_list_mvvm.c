/**
 * File:   application.c
 * Author: AWTK Develop Team
 * Brief:  application
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-09-06 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "awtk.h"
#include "mvvm/mvvm.h"
#include "app_database.h"
#include "table_view_register.h"
#include "../res/assets_default.inc"
#include "table_client_custom_binder.h"
#include "slidable_row_register.h"
#include "repository/view_model_record.h"
#include "repository/view_model_repository.h"

ret_t application_init(void)
{
    mvvm_init();
    table_view_register();
    slidable_row_register();
    table_client_custom_binder_register();

    app_database_init("student_list_mvvm", "students.db");

    return navigator_to("student_list_mvvm");
}

ret_t application_exit()
{
    app_database_deinit();
    mvvm_deinit();
    return RET_OK;
}

#include "awtk_main.inc"
