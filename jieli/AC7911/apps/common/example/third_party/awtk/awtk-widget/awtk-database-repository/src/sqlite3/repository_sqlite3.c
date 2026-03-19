/**
 * File:   repository_sqlite3.c
 * Author: AWTK Develop Team
 * Brief:  repository sqlite3
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
#include "tkc/mem.h"
#include "tkc/str.h"
#include "tkc/utils.h"
#include "tkc/darray.h"
#include "tkc/named_value.h"
#include "repository_sqlite3.h"

typedef int (*on_sqlite3_callback_t)(void *data, int argc, char **argv, char **azColName);

static repository_sqlite3_t *repository_cast(repository_t *r);

#define REPOSITORY_SQLITE3(r) repository_cast(r);

static int on_sql_count_result(void *data, int argc, char **argv, char **azColName)
{
    int *count = (int *)data;

    if (count != NULL && argc > 0) {
        *count = tk_atoi(argv[0]);
    }

    return 0;
}

static int on_sql_result_dump(void *data, int argc, char **argv, char **azColName)
{
    int i;
    log_info("%s: ", (const char *)data);

    for (i = 0; i < argc; i++) {
        log_info("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    log_info("\n");
    return 0;
}

typedef struct _select_info_t {
    repository_t *r;
    uint32_t index;
} select_info_t;

static int on_sql_select_result(void *data, int argc, char **argv, char **azColName)
{
    int i = 0;
    select_info_t *info = (select_info_t *)data;
    object_t *o = repository_create_object(info->r);

    if (o != NULL) {
        for (i = 0; i < argc; i++) {
            object_set_prop_str(o, azColName[i], argv[i]);
        }
        repository_add_cache(info->r, o);
        OBJECT_UNREF(o);
    } else {
        log_warn("%s:OOM\n", __FUNCTION__);
    }

    return SQLITE_OK;
}

static ret_t repository_sqlite3_exec_sql_ex(repository_t *r, const char *sql,
        on_sqlite3_callback_t on_sql_result, void *ctx)
{
    char *zErrMsg = NULL;
    repository_sqlite3_t *rsqlite3 = REPOSITORY_SQLITE3(r);
    int rc = sqlite3_exec(rsqlite3->db, sql, on_sql_result, ctx, &zErrMsg);
    log_debug("%d:%s\n", rc, sql);

    if (zErrMsg != NULL) {
        log_warn("Sqlite3 error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    return rc == SQLITE_OK ? RET_OK : RET_FAIL;
}


static ret_t repository_sqlite3_select(repository_t *r, const char *sql)
{
    select_info_t info = {r, 0};
    repository_clear_cache(r);

    return repository_sqlite3_exec_sql_ex(r, sql, on_sql_select_result, &info);
}

static uint32_t repository_sqlite3_count(repository_t *r, const char *sql)
{
    uint32_t count = 0;

    repository_sqlite3_exec_sql_ex(r, sql, on_sql_count_result, &count);

    return count;
}

static ret_t repository_sqlite3_exec_sql(repository_t *r, const char *sql)
{
    return repository_sqlite3_exec_sql_ex(r, sql, on_sql_result_dump, r);
}

static ret_t repository_sqlite3_destroy(repository_t *r)
{
    repository_sqlite3_t *rsqlite3 = REPOSITORY_SQLITE3(r);

    TKMEM_FREE(r->table_name);
    TKMEM_FREE(r->primary_key);
    darray_deinit(&(r->cache));
    memset(rsqlite3, 0x0, sizeof(repository_sqlite3_t));
    TKMEM_FREE(rsqlite3);

    return RET_OK;
}

static const repository_vtable_t s_sqlite3_vtable = {.count = repository_sqlite3_count,
                                                     .select = repository_sqlite3_select,
                                                     .destroy = repository_sqlite3_destroy,
                                                     .exec_sql = repository_sqlite3_exec_sql
                                                    };

static repository_sqlite3_t *repository_cast(repository_t *r)
{
    return_value_if_fail(r != NULL && r->vt != NULL, NULL);

    if (r->vt == &s_sqlite3_vtable) {
        return (repository_sqlite3_t *)r;
    } else {
        return NULL;
    }
}

static ret_t repository_sqlite3_show_error(repository_t *r, const char *tips)
{
    repository_sqlite3_t *rsqlite3 = REPOSITORY_SQLITE3(r);
    const char *error = sqlite3_errmsg(rsqlite3->db);
    if (error != NULL) {
        log_warn("%s: %s\n", tips, error);
    }

    return RET_OK;
}

repository_t *repository_sqlite3_create(sqlite3 *db, const char *table_name,
                                        const char *primary_key)
{
    int ret = 0;
    repository_t *r = NULL;
    repository_sqlite3_t *rsqlite3 = NULL;
    return_value_if_fail(db != NULL && table_name != NULL && primary_key != NULL, NULL);

    rsqlite3 = TKMEM_ZALLOC(repository_sqlite3_t);
    return_value_if_fail(rsqlite3, NULL);

    r = (repository_t *)rsqlite3;
    rsqlite3->repository.vt = &s_sqlite3_vtable;
    darray_init(&(r->cache), 100, (tk_destroy_t)object_unref, NULL);

    rsqlite3->db = db;
    if (ret == SQLITE_OK) {
        r->table_name = tk_strdup(table_name);
        r->primary_key = tk_strdup(primary_key);
    } else {
        repository_sqlite3_show_error(r, "Open Database");
        repository_destroy(r);
        r = NULL;
    }

    return r;
}
