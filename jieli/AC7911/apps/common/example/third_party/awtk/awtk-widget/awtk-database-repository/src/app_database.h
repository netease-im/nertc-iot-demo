/**
 * File:   app_database.h
 * Author: AWTK Develop Team
 * Brief:  helper functions to init/deinit database and create repository
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
 * 2020-09-10 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef APP_DATABASE_H
#define APP_DATABASE_H

#include "repository/repository.h"

BEGIN_C_DECLS

/**
 * @method app_database_init
 * @annotation ["global"]
 * 初始化数据库。
 *
 * 通常我们在资源中放一个初始的数据库文件，里面有缺省的表和数据。
 * 本函数检查数据库文件是否存在，如果不存在，就从资源中取出初始的数据库文件，拷贝到用户运行目录。
 * 然后打开数据库。
 *
 * @param {const char*} appname 应用程序的名称。
 * @param {const char*} dbname 数据库名称。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t app_database_init(const char *appname, const char *dbname);

/**
 * @method app_database_create_repository
 * @annotation ["global"]
 * 创建repository对象。
 *
 * @param {const char*} table 表名。
 * @param {const char*} primary_key 主键。
 *
 * @return {repository_t*} 返回repository对象。
 */
repository_t *app_database_create_repository(const char *table, const char *primary_key);

/**
 * @method app_database_deinit
 * @annotation ["global"]
 * 关闭数据库。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t app_database_deinit(void);

END_C_DECLS

#endif/*APP_DATABASE_H*/

