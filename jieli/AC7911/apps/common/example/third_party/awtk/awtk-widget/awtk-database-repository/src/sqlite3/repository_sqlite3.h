/**
 * File:   repository_sqlite3.h
 * Author: AWTK Develop Team
 * Brief:  repository sqlite3 implementation
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
 * 2020-08-30 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_REPOSITORY_SQLITE3_H
#define TK_REPOSITORY_SQLITE3_H

#include "sqlite3.h"
#include "repository/repository.h"

BEGIN_C_DECLS

/**
 * @class repository_sqlite3_t
 * sqlite3实现的repository。
 */
typedef struct _repository_sqlite3_t {
    repository_t repository;
    /*private*/
    sqlite3 *db;
} repository_sqlite3_t;

/**
 * @method repository_sqlite3_create
 *
 * 创建repository对象。
 *
 * @param {sqlite3*} db 数据库对象。
 * @param {const char*} table_name 数据库表名。
 * @param {const char*} primary_key 数据库主键名。
 *
 * @return {repository_t*} 返回NULL表示失败，否则返回repository对象。
 */
repository_t *repository_sqlite3_create(sqlite3 *db, const char *table_name,
                                        const char *primary_key);

END_C_DECLS

#endif /*TK_REPOSITORY_SQLITE3_H*/
