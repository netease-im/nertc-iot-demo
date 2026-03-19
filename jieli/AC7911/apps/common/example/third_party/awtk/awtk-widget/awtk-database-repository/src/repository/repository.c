/**
 * File:   repository.c
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

#include "tkc/str.h"
#include "tkc/utils.h"
#include "tkc/object_default.h"
#include "repository/repository.h"

typedef struct _visit_prop_info_t {
    str_t *str;
    uint32_t index;
    repository_t *r;
} visit_prop_info_t;

static ret_t generate_keys(void *ctx, const void *data)
{
    visit_prop_info_t *info = (visit_prop_info_t *)ctx;
    named_value_t *iter = (named_value_t *)data;
    str_t *str = info->str;

    if (info->index > 0) {
        str_append(str, ", ");
    }
    str_append(str, iter->name);

    info->index++;
    return RET_OK;
}

static ret_t str_append_value(str_t *str, value_t *v)
{
    if (v->type == VALUE_TYPE_STRING) {
        str_append_char(str, '"');
        str_append(str, value_str(v));
        str_append_char(str, '"');
    } else if (v->type == VALUE_TYPE_DOUBLE || v->type == VALUE_TYPE_FLOAT32 ||
               v->type == VALUE_TYPE_FLOAT) {
        str_append_double(str, "%lf", value_double(v));
    } else {
        str_append_int(str, value_int(v));
    }

    return RET_OK;
}

static ret_t generate_values(void *ctx, const void *data)
{
    visit_prop_info_t *info = (visit_prop_info_t *)ctx;
    named_value_t *iter = (named_value_t *)data;
    str_t *str = info->str;
    value_t *v = &(iter->value);

    if (info->index > 0) {
        str_append_char(str, ',');
        str_append_char(str, ' ');
    }

    str_append_value(str, v);

    info->index++;
    return RET_OK;
}

ret_t repository_insert(repository_t *r, object_t *o)
{
    str_t str;
    ret_t ret = RET_OK;
    visit_prop_info_t info;
    return_value_if_fail(r != NULL && o != NULL, RET_BAD_PARAMS);
    return_value_if_fail(str_init(&str, 4096) != NULL, RET_OOM);

    info.r = r;
    info.str = &str;
    str_append(&str, "INSERT INTO ");
    str_append(&str, r->table_name);
    str_append_char(&str, '(');
    info.index = 0;
    object_foreach_prop(o, generate_keys, &info);
    str_append(&str, ") VALUES (");

    info.index = 0;
    object_foreach_prop(o, generate_values, &info);
    str_append(&str, ");");

    ret = repository_exec_sql(r, str.str);
    str_reset(&str);

    return ret;
}

static ret_t generate_key_value(void *ctx, const void *data)
{
    visit_prop_info_t *info = (visit_prop_info_t *)ctx;
    named_value_t *iter = (named_value_t *)data;
    str_t *str = info->str;
    value_t *v = &(iter->value);

    if (info->index > 0) {
        str_append_char(str, ',');
        str_append_char(str, ' ');
    }

    str_append(str, iter->name);
    str_append_char(str, '=');
    str_append_value(str, v);

    info->index++;

    return RET_OK;
}

static ret_t repository_generate_where(repository_t *r, str_t *str, object_t *o)
{
    value_t v;
    visit_prop_info_t info;
    str_append(str, " WHERE ");
    if (object_get_prop(o, r->primary_key, &v) == RET_OK) {
        str_append(str, r->primary_key);
        str_append(str, " = ");
        str_append_value(str, &v);
        str_append(str, " ; ");
    } else {
        info.r = r;
        info.str = str;
        info.index = 0;
        object_foreach_prop(o, generate_key_value, &info);
    }

    return RET_OK;
}

ret_t repository_update(repository_t *r, object_t *o)
{
    str_t str;
    ret_t ret = RET_OK;
    visit_prop_info_t info;
    return_value_if_fail(r != NULL && o != NULL, RET_BAD_PARAMS);
    return_value_if_fail(str_init(&str, 4096) != NULL, RET_OOM);

    info.r = r;
    info.index = 0;
    info.str = &str;

    str_append(&str, "UPDATE ");
    str_append(&str, r->table_name);
    str_append(&str, " SET ");
    object_foreach_prop(o, generate_key_value, &info);
    repository_generate_where(r, &str, o);

    ret = repository_exec_sql(r, str.str);
    str_reset(&str);

    return ret;
}

ret_t repository_remove(repository_t *r, object_t *o)
{
    str_t str;
    ret_t ret = RET_OK;
    return_value_if_fail(r != NULL && o != NULL, RET_BAD_PARAMS);
    return_value_if_fail(str_init(&str, 4096) != NULL, RET_OOM);

    str_append(&str, "DELETE FROM ");
    str_append(&str, r->table_name);
    repository_generate_where(r, &str, o);

    ret = repository_exec_sql(r, str.str);
    repository_remove_cache(r, o);
    str_reset(&str);

    return ret;
}

ret_t repository_clear_ex(repository_t *r, const char *filter)
{
    char sql[1024] = {0};
    return_value_if_fail(r != NULL && r->table_name != NULL, RET_BAD_PARAMS);

    if (filter != NULL) {
        tk_snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE %s", r->table_name, filter);
    } else {
        tk_snprintf(sql, sizeof(sql), "DELETE FROM %s", r->table_name);
    }

    return repository_exec_sql(r, sql);
}

ret_t repository_clear(repository_t *r)
{
    char sql[256] = {0};
    return_value_if_fail(r != NULL && r->table_name != NULL, RET_BAD_PARAMS);
    tk_snprintf(sql, sizeof(sql), "DELETE FROM %s", r->table_name);

    return repository_exec_sql(r, sql);
}

bool_t repository_exist(repository_t *r, object_t *o)
{
    str_t str;
    bool_t ret = TRUE;
    return_value_if_fail(r != NULL && o != NULL, FALSE);
    return_value_if_fail(str_init(&str, 4096) != NULL, FALSE);

    str_append(&str, "SELECT COUNT(");
    str_append(&str, r->primary_key);
    str_append(&str, ") FROM ");
    str_append(&str, r->table_name);
    repository_generate_where(r, &str, o);

    ret = repository_count(r, str.str) > 0;
    str_reset(&str);

    return ret;
}

ret_t repository_select(repository_t *r, const char *sql)
{
    char select_all[512];
    return_value_if_fail(r != NULL && r->vt != NULL && r->vt->select != NULL, RET_BAD_PARAMS);

    if (sql == NULL) {
        return_value_if_fail(r->table_name != NULL, RET_BAD_PARAMS);
        tk_snprintf(select_all, sizeof(select_all), "SELECT * FROM %s;", r->table_name);
        sql = select_all;
    }

    return r->vt->select(r, sql);
}

uint32_t repository_count(repository_t *r, const char *sql)
{
    char select_all[512];
    return_value_if_fail(r != NULL && r->vt != NULL && r->vt->count != NULL, 0);

    if (sql == NULL) {
        return_value_if_fail(r->table_name != NULL && r->primary_key != NULL, 0);
        tk_snprintf(select_all, sizeof(select_all), "SELECT COUNT(%s) FROM %s;", r->primary_key,
                    r->table_name);
        sql = select_all;
    }

    return r->vt->count(r, sql);
}

ret_t repository_exec_sql(repository_t *r, const char *sql)
{
    return_value_if_fail(sql != NULL, RET_BAD_PARAMS);
    return_value_if_fail(r != NULL && r->vt != NULL && r->vt->exec_sql != NULL, RET_BAD_PARAMS);

    return r->vt->exec_sql(r, sql);
}

ret_t repository_remove_index(repository_t *r, uint32_t index)
{
    object_t *o = repository_get_cache_object(r, index);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    return repository_remove(r, o);
}

object_t *repository_create_object(repository_t *r)
{
    return_value_if_fail(r != NULL && r->vt != NULL, NULL);

    if (r->create_object != NULL) {
        return r->create_object(r);
    } else {
        return object_default_create();
    }
}

ret_t repository_destroy(repository_t *r)
{
    return_value_if_fail(r != NULL && r->vt != NULL, RET_BAD_PARAMS);
    if (r->vt->destroy != NULL) {
        r->vt->destroy(r);
    }

    return RET_OK;
}

uint32_t repository_get_cache_nr(repository_t *r)
{
    return_value_if_fail(r != NULL, 0);

    return r->cache.size;
}

ret_t repository_clear_cache(repository_t *r)
{
    return_value_if_fail(r != NULL, RET_FAIL);
    darray_clear(&(r->cache));

    return RET_OK;
}

ret_t repository_add_cache(repository_t *r, object_t *o)
{
    return_value_if_fail(r != NULL && o != NULL, RET_FAIL);
    object_ref(o);
    darray_push(&(r->cache), o);

    return RET_OK;
}

ret_t repository_remove_cache(repository_t *r, object_t *o)
{
    return_value_if_fail(r != NULL && o != NULL, RET_FAIL);

    ENSURE(o->ref_count == 1);
    if (darray_remove(&(r->cache), o) == RET_OK) {
    }

    return RET_OK;
}

object_t *repository_get_cache_object(repository_t *r, uint32_t index)
{
    return_value_if_fail(r != NULL, 0);
    return_value_if_fail(index < repository_get_cache_nr(r), NULL);

    return (object_t *)(r->cache.elms[index]);
}

ret_t repository_trans_begin(repository_t *r)
{
    return_value_if_fail(r != NULL, 0);

    return repository_exec_sql(r, "BEGIN TRANSACTION;");
}

ret_t repository_trans_commit(repository_t *r)
{
    return_value_if_fail(r != NULL, 0);

    return repository_exec_sql(r, "COMMIT;");
}

ret_t repository_trans_rollback(repository_t *r)
{
    return_value_if_fail(r != NULL, 0);

    return repository_exec_sql(r, "ROLLBACK;");
}
