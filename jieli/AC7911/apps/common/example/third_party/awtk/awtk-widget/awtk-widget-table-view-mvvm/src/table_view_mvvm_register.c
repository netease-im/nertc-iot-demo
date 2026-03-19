/**
 * File:   table_view_mvvm_register.c
 * Author: AWTK Develop Team
 * Brief:  表格视图。
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-07-15 Tang Junjie <tangjunjie@zlg.cn> created
 *
 */

#include "table_client_custom_binder.h"

ret_t table_view_mvvm_register(void)
{
    return table_client_custom_binder_register();
}
