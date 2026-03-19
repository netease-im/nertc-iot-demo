/**
 * File:   rlottie.c
 * Author: AWTK Develop Team
 * Brief:  Lottie animations player
 *
 * Copyright (c) 2021 - 2021 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2021-06-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */


#include "tkc/mem.h"
#include "tkc/utils.h"
#include "rlottie_register.h"
#include "base/widget_factory.h"
#include "rlottie/rlottie.h"

ret_t rlottie_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_RLOTTIE, rlottie_create);
}

const char *rlottie_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
