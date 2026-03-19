/**
 * File:   video_image_register.h
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

#ifndef TK_VIDEO_IMAGE_REGISTER_H
#define TK_VIDEO_IMAGE_REGISTER_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @method  video_image_register
 * 注册控件。
 *
 * @annotation ["global"]
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t video_image_register(void);

/**
 * @method  video_image_supported_render_mode
 * 获取支持的渲染模式。
 *
 * @annotation ["global"]
 *
 * @return {const char*} 返回渲染模式。
 */
const char *video_image_supported_render_mode(void);

END_C_DECLS

#endif /*TK_VIDEO_IMAGE_REGISTER_H*/
