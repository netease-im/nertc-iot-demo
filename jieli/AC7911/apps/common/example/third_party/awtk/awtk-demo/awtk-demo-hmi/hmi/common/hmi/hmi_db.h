/**
 * File:   hmi_db.h
 * Author: AWTK Develop Team
 * Brief:  hmi_db.h
 *
 * Copyright (c) 2023 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2023-12-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_HMI_DB_H
#define TK_HMI_DB_H

#include "tkc/object.h"
#include "conf_io/conf_node.h"

BEGIN_C_DECLS

/**
 * @method hmi_db_init
 * 初始化 db。
 *
 * @param {tk_object_t*} model 默认模型对象。
 * @param {conf_doc_t*} settings 配置信息。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_db_init(tk_object_t *model, conf_doc_t *settings);

/**
 * @method hmi_db_deinit
 * 反初始化 db。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_db_deinit(void);

END_C_DECLS

#endif /*TK_HMI_DB_H*/
