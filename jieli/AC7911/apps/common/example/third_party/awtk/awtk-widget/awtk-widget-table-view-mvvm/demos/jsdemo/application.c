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
#include "table_view_register.h"
#include "table_view_mvvm_register.h"

ret_t application_init()
{
    table_view_register();
    table_view_mvvm_register();

    return navigator_to("main");
}

ret_t application_exit()
{
    log_debug("application_exit\n");
    return RET_OK;
}

#include "../main.inc"