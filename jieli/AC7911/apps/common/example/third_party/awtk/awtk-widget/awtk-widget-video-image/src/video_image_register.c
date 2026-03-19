/**
 * File:   video_image.c
 * Author: AWTK Develop Team
 * Brief:  帧间差异视频播放控件
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
 * 2020-06-18 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "video_image_register.h"
#include "base/widget_factory.h"
#include "video_image/video_image.h"

ret_t video_image_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_VIDEO_IMAGE, video_image_create);
}

const char *video_image_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}