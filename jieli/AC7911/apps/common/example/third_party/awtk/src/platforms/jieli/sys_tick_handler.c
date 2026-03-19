/**
 * File:   sys_tick.c
 * Author: AWTK Develop Team
 * Brief:  use sys tick to implement sleep/get_time_ms64.
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
 * 2018-06-03 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/types_def.h"


uint64_t get_time_ms64()
{
    return timer_get_ms();
}

void sleep_ms(uint32_t ms)
{
    msleep(ms);
}


//void timer_2ms_handler()
//{
//    tk_sys_tick_inc(2);
//}
//#include "../common/sys_tick_common.inc"
