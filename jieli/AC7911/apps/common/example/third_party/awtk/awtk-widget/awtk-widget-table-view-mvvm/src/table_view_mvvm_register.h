/**
 * File:   table_view_register.h
 * Author: AWTK Develop Team
 * Brief:  表格视图。
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
 * 2020-07-15 Tang Junjie <tangjunjie@zlg.cn> created
 *
 */

#ifndef TK_TABLE_VIEW_MVVM_REGISTER_H
#define TK_TABLE_VIEW_MVVM_REGISTER_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @method  table_view_mvvm_register
 * 注册控件。
 *
 * @annotation ["global"]
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_view_mvvm_register(void);

END_C_DECLS

#endif /*TK_TABLE_VIEW_MVVM_REGISTER_H*/
