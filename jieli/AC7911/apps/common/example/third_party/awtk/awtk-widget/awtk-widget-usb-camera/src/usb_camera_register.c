/**
 * File:   usb_camera.c
 * Author: AWTK Develop Team
 * Brief:  USB 摄像头控件
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
 * 2020-06-17 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "usb_camera_register.h"
#include "base/widget_factory.h"
#include "usb_camera/usb_camera.h"

ret_t usb_camera_register(void)
{
    return widget_factory_register(widget_factory(), WIDGET_TYPE_USB_CAMERA, usb_camera_create);
}

const char *usb_camera_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}