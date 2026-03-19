/**
 * File:   slidable_row.c
 * Author: AWTK Develop Team
 * Brief:  slidable row(仿微信列表项目，像左滑动显示“删除”按钮)。
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
 * 2020-07-28 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "slidable_row_register.h"
#include "base/widget_factory.h"
#include "slidable_row/slidable_row.h"
#include "confirmable_button/confirmable_button.h"

ret_t slidable_row_register(void)
{
    widget_factory_register(widget_factory(), WIDGET_TYPE_CONFIRMABLE_BUTTON,
                            confirmable_button_create);
    return widget_factory_register(widget_factory(), WIDGET_TYPE_SLIDABLE_ROW, slidable_row_create);
}

const char *slidable_row_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
