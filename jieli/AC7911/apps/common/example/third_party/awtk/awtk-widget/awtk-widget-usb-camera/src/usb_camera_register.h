/**
 * File:   usb_camera_register.h
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

#ifndef TK_USB_CAMERA_REGISTER_H
#define TK_USB_CAMERA_REGISTER_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @method  usb_camera_register
 * 注册控件。
 *
 * @annotation ["global"]
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_register(void);

/**
 * @method  usb_camera_supported_render_mode
 * 获取支持的渲染模式。
 *
 * @annotation ["global"]
 *
 * @return {const char*} 返回渲染模式。
 */
const char *usb_camera_supported_render_mode(void);

END_C_DECLS

#endif /*TK_USB_CAMERA_REGISTER_H*/
