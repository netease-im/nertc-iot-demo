/**
 * File:   number_label_register.h
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

#ifndef TK_NUMBER_LABEL_REGISTER_H
#define TK_NUMBER_LABEL_REGISTER_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @method  number_label_register
 * 注册数值文本控件。
 *
 * @annotation ["global"]
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_register(void);

/**
 * @method  number_label_supported_render_mode
 * 获取支持的渲染模式。
 *
 * @annotation ["global"]
 *
 * @return {const char*} 返回渲染模式。
 */
const char *number_label_supported_render_mode(void);

END_C_DECLS

#endif /*TK_NUMBER_LABEL_REGISTER_H*/
