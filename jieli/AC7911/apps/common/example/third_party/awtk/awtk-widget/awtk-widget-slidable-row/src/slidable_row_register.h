/**
 * File:   slidable_row_register.h
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

#ifndef TK_SLIDABLE_ROW_REGISTER_H
#define TK_SLIDABLE_ROW_REGISTER_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @method  slidable_row_register
 * 注册控件。
 *
 * @annotation ["global"]
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t slidable_row_register(void);

/**
 * @method  slidable_row_supported_render_mode
 * 获取支持的渲染模式。
 *
 * @annotation ["global"]
 *
 * @return {const char*} 返回渲染模式。
 */
const char *slidable_row_supported_render_mode(void);

END_C_DECLS

#endif /*TK_SLIDABLE_ROW_REGISTER_H*/
