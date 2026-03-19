/**
 * File:   user_manager.h
 * Author: AWTK Develop Team
 * Brief:  user_manager
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
 * 2023-12-12 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_USER_MANAGER_H
#define TK_USER_MANAGER_H

#include "tkc/object.h"
#include "conf_io/conf_node.h"

BEGIN_C_DECLS

/**
 * @class user_manager_t
 * @annotation ["fake"]
 * 用户管理。
 *
 */

/**
 * @method user_manager_init
 * 初始化user_manager。
 *
 * @param {tk_object_t*} model 默认模型对象。
 * @param {conf_doc_t*} settings 配置信息。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t user_manager_init(tk_object_t *model, conf_doc_t *settings);

/**
 * @method user_manager_deinit
 * 销毁user_manager。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t user_manager_deinit(void);


#define USER_MANAGER_PROP_NAME "user_manager"

#define USER_MANAGER_PROP_QUERY_ROLE CSV_QUERY_PREFIX"role"
#define USER_MANAGER_PROP_QUERY_NAME CSV_QUERY_PREFIX"name"

#define USER_MANAGER_FIELD_NAME "name"
#define USER_MANAGER_FIELD_ROLE "role"
#define USER_MANAGER_FIELD_PASSWORD "password"

#define MODEL_PROP_LOGIN_USERNAME "login_username"
#define MODEL_PROP_LOGIN_PASSWORD "login_password"

#define MODEL_PROP_CURRENT_USERNAME "username"
#define MODEL_PROP_CURRENT_USERROLE "userrole"

#define MODEL_PROP_CHANGE_PASSWORD "change_password"
#define MODEL_PROP_CHANGE_CONFIRM_PASSWORD "change_confirm_password"

END_C_DECLS

#endif /*TK_USER_MANAGER_H*/
