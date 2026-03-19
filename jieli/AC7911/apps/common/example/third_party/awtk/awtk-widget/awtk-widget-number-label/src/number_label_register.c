/**
 * File:   number_label_register.c
 * Author: AWTK Develop Team
 * Brief:  number label register
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
 * 2020-05-19 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "base/widget_factory.h"
#include "number_label_register.h"
#include "number_label/number_label.h"

ret_t number_label_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_NUMBER_LABEL, number_label_create);
}

const char *number_label_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
