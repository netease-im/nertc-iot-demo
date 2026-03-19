/**
 * File:   title_value.c
 * Author: AWTK Develop Team
 * Brief:  用于显示标题和值的控件。
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
 * 2020-10-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "title_value_register.h"
#include "base/widget_factory.h"
#include "title_value/title_value.h"

ret_t title_value_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_TITLE_VALUE, title_value_create);
}

const char *title_value_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
