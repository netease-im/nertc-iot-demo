/**
 * File:   table_client_custom_binder.h
 * Author: AWTK Develop Team
 * Brief:  table_client_custom_binder
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-07-24 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TABLE_CLIENT_CUSTOM_BINDER_H
#define TK_TABLE_CLIENT_CUSTOM_BINDER_H

#include "tkc/types_def.h"

BEGIN_C_DECLS

/**
 * @method table_client_custom_binder_register
 * 注册table client的自定义绑定函数。
 *
 * @annotation ["global"]
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_custom_binder_register(void);

END_C_DECLS

#endif /*TK_TABLE_CLIENT_CUSTOM_BINDER_H*/
