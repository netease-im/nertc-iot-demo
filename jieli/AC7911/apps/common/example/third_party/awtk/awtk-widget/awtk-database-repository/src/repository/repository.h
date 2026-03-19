/**
 * File:   repository.h
 * Author: AWTK Develop Team
 * Brief:  repository interface
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

#ifndef TK_REPOSITORY_H
#define TK_REPOSITORY_H

#include "tkc/object.h"
#include "tkc/darray.h"

BEGIN_C_DECLS

struct _repository_t;
typedef struct _repository_t repository_t;

typedef ret_t (*repository_select_t)(repository_t *r, const char *sql);
typedef ret_t (*repository_exec_sql_t)(repository_t *r, const char *sql);
typedef uint32_t (*repository_count_t)(repository_t *r, const char *sql);
typedef ret_t (*repository_destroy_t)(repository_t *r);

typedef object_t *(*repository_create_object_t)(repository_t *r);

typedef struct _repository_vtable_t {
    repository_count_t count;
    repository_select_t select;
    repository_exec_sql_t exec_sql;
    repository_destroy_t destroy;
} repository_vtable_t;

/**
 * @class repository_t
 * repository对sql数据库的封装。
 *
 */
struct _repository_t {
    /*private*/
    repository_create_object_t create_object;
    const repository_vtable_t *vt;
    char *table_name;
    char *primary_key;
    darray_t cache;
};

/**
 * @method repository_get_cache_nr
 *
 * 获取缓存记录条数。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {uint32_t} 返回缓存记录条数。
 */
uint32_t repository_get_cache_nr(repository_t *r);

/**
 * @method repository_get_cache_object
 *
 * 获取指定序数的缓存记录。
 *
 * @param {repository_t*} r repository对象。
 * @param {uint32_t} o index 序数。
 *
 * @return {object_t*} 返回NULL表示失败，否则返回对应的记录。
 */
object_t *repository_get_cache_object(repository_t *r, uint32_t index);

/**
 * @method repository_insert
 *
 * 插入一条记录。
 *
 * @param {repository_t*} r repository对象。
 * @param {object_t*} o 数据记录对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_insert(repository_t *r, object_t *o);

/**
 * @method repository_update
 *
 * 更新一条记录。
 *
 * @param {repository_t*} r repository对象。
 * @param {object_t*} o 数据记录对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_update(repository_t *r, object_t *o);

/**
 * @method repository_remove
 *
 * 删除一条记录。
 *
 * @param {repository_t*} r repository对象。
 * @param {object_t*} o 数据记录对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_remove(repository_t *r, object_t *o);

/**
 * @method repository_clear
 *
 * 删除全部数据。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_clear(repository_t *r);

/**
 * @method repository_clear_ex
 *
 * 删除全部数据。
 *
 * @param {repository_t*} r repository对象。
 * @param {const char*} filter 过滤器。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_clear_ex(repository_t *r, const char *filter);

/**
 * @method repository_exist
 *
 * 检查一条记录是否已经存在。
 *
 * @param {repository_t*} r repository对象。
 * @param {object_t*} o 数据记录对象。
 *
 * @return {bool_t} 返回TRUE表示存在，否则表示不存在。
 */
bool_t repository_exist(repository_t *r, object_t *o);

/**
 * @method repository_exec_sql
 *
 * 执行SQL语句
 *
 * @param {repository_t*} r repository对象。
 * @param {const char*} sql SQL语句。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_exec_sql(repository_t *r, const char *sql);

/**
 * @method repository_trans_begin
 *
 * 开始事务。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_trans_begin(repository_t *r);

/**
 * @method repository_trans_commit
 *
 * 提交事务。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_trans_commit(repository_t *r);

/**
 * @method repository_trans_rollback
 *
 * 回滚事务。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_trans_rollback(repository_t *r);

/**
 * @method repository_select
 *
 * 查询更新缓存
 *
 * @param {repository_t*} r repository对象。
 * @param {const char*} sql SQL语句。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_select(repository_t *r, const char *sql);

/**
 * @method repository_count
 *
 * 统计满足条件的记录条数。
 *
 * @param {repository_t*} r repository对象。
 * @param {const char*} sql SQL语句。
 *
 * @return {uint32_t} 返回满足条件的记录条数。
 */
uint32_t repository_count(repository_t *r, const char *sql);

/**
 * @method repository_remove_index
 *
 * 删除一条记录。
 *
 * @param {repository_t*} r repository对象。
 * @param {uint32_t} index 缓存对象的序数。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_remove_index(repository_t *r, uint32_t index);

/**
 * @method repository_create_object
 *
 * 创建一个空对象。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {object_t*} 返回NULL表示失败，否则返回新建的对象。
 */
object_t *repository_create_object(repository_t *r);

/**
 * @method repository_clear_cache
 *
 * 清除全部缓存。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_clear_cache(repository_t *r);

/**
 * @method repository_add_cache
 *
 * 增加一条记录到缓存。
 *
 * @param {repository_t*} r repository对象。
 * @param {object_t*} o 数据记录对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_add_cache(repository_t *r, object_t *o);

/**
 * @method repository_remove_cache
 *
 * 删除一条缓存记录。
 *
 * @param {repository_t*} r repository对象。
 * @param {object_t*} o 数据记录对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_remove_cache(repository_t *r, object_t *o);

/**
 * @method repository_destroy
 *
 * 销毁repository对象。
 *
 * @param {repository_t*} r repository对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t repository_destroy(repository_t *r);

END_C_DECLS

#endif /*TK_REPOSITORY_H*/
