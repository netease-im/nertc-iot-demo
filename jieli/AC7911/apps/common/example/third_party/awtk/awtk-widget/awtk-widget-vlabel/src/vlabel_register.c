/**
 * File:   vlabel.c
 * Author: AWTK Develop Team
 * Brief:  垂直显示文本的控件。
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
 * 2020-10-01 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "vlabel_register.h"
#include "base/widget_factory.h"
#include "vlabel/vlabel.h"

ret_t vlabel_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_VLABEL, vlabel_create);
}

const char *vlabel_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
