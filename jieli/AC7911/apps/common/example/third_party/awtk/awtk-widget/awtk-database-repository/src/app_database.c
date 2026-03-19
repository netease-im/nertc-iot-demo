
/**
 * File:   app_database.c
 * Author: AWTK Develop Team
 * Brief:  app_database
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

#include "tkc/fs.h"
#include "tkc/path.h"
#include "base/assets_manager.h"
#include "app_database.h"

static const char *build_user_storage_file_name(char filename[MAX_PATH + 1], const char *appname,
        const char *name)
{
    char home[MAX_PATH + 1];
    char path[MAX_PATH + 1];

    fs_get_user_storage_path(os_fs(), home);
    path_build(path, MAX_PATH, home, appname, NULL);

    if (!path_exist(path)) {
        fs_create_dir(os_fs(), path);
    }
    path_build(filename, MAX_PATH, path, name, NULL);

    return filename;
}

static ret_t copy_asset_to_file(const char *name, const char *filename)
{
    assets_manager_t *am = assets_manager();
    const asset_info_t *info = assets_manager_ref(am, ASSET_TYPE_DATA, name);

    if (info != NULL) {
        file_write(filename, info->data, info->size);
        assets_manager_unref(am, info);

        log_info("copy %s to %s\n", name, filename);
        return RET_OK;
    } else {
        log_warn("asset %s not exist\n", name);
        return RET_OK;
    }
}

ret_t prepare_database_file(const char *appname, const char *dbname, char dbfilename[MAX_PATH + 1])
{
    /*to build filename that is writable for app*/
    build_user_storage_file_name(dbfilename, appname, dbname);

    if (!file_exist(dbfilename)) {
        return copy_asset_to_file(dbname, dbfilename);
    } else {
        log_info("database %s exist\n", dbfilename);
        return RET_OK;
    }
}


#include "sqlite3/repository_sqlite3.h"

#include "mvvm/mvvm.h"
#include "repository/view_model_record.h"
#include "repository/view_model_repository.h"

static sqlite3 *s_sqlite3_db;

ret_t app_database_init(const char *appname, const char *dbname)
{
    char dbfilename[MAX_PATH + 1];

    sqlite3_initialize();
    return_value_if_fail(dbname != NULL, RET_BAD_PARAMS);

    prepare_database_file(appname, dbname, dbfilename);

    ENSURE(sqlite3_open(dbfilename, &s_sqlite3_db) == SQLITE_OK);

    view_model_factory_register("record", view_model_record_create);
    view_model_factory_register("database", view_model_repository_create_with_req);

    return RET_OK;
}

ret_t app_database_deinit(void)
{
    if (s_sqlite3_db != NULL) {
        sqlite3_close(s_sqlite3_db);
        s_sqlite3_db = NULL;
    }

    return RET_OK;
}

repository_t *app_database_create_repository(const char *table, const char *primary_key)
{
    return_value_if_fail(table != NULL && primary_key != NULL && s_sqlite3_db != NULL, NULL);

    return repository_sqlite3_create(s_sqlite3_db, table, primary_key);
}

