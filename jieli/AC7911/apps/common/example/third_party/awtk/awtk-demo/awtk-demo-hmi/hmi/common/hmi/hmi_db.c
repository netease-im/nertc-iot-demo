/**
 * File:   hmi_db.c
 * Author: AWTK Develop Team
 * Brief:  hmi_db.c
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

#include "hmi_db.h"

#include "awtk.h"
#include "mvvm/mvvm.h"
#include "app_database.h"
#include "repository/view_model_record.h"
#include "repository/view_model_repository.h"
#include "tkc/func_call_parser.h"

view_model_t *db_table_view_model_create(navigator_request_t *req)
{
    return view_model_repository_create_with_req(req);
}

ret_t hmi_db_init(tk_object_t *model, conf_doc_t *settings)
{
    const char *app_name = NULL;
    const char *db_name = NULL;

    if (!conf_doc_get_bool(settings, "database.enable", FALSE)) {
        log_debug("database is disable\n");
        return RET_FAIL;
    }

    app_name = conf_doc_get_str(settings, "name", "myapp");
    db_name = conf_doc_get_str(settings, "database.filename", "default.db");

    return app_database_init(app_name, db_name);
}

ret_t hmi_db_deinit(void)
{
    app_database_deinit();
    return RET_OK;
}

