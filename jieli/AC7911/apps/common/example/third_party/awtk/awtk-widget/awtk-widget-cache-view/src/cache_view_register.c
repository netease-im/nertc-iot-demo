/**
 * File:   cache_view_register.c
 * Author: AWTK Develop Team
 * Brief:  cache view register
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
 * 2020-06-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "base/widget_factory.h"
#include "cache_view_register.h"
#include "cache_view/cache_view.h"

ret_t cache_view_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_CACHE_VIEW, cache_view_create);
}

const char *cache_view_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888";
}
