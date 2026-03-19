/**
 * File:   user_manager.c
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

#include "mvvm/mvvm.h"
#include "tkc/date_time.h"
#include "user_manager.h"
#include "hmi/hmi_model.h"
#include "csv_view_model.h"
#include "csv/csv_file_object.h"

/*默认参数*/
/*默认用|分隔，因为|比较少用。*/
#define USER_MANAGER_FEIDLS_SEPERATOR '|'

/*最大记录数*/
#define USER_MANAGER_MAX_ROWS 100

typedef struct _user_manager_filter_ctx {
    int32_t role;
    const char *name;

    uint32_t role_index;
    uint32_t name_index;
    uint32_t password_index;
} user_manager_filter_ctx_t;

static view_model_t *s_user_manager_vm = NULL;
static user_manager_filter_ctx_t s_user_manager_filter_ctx;

#define ROLE_INDEX s_user_manager_filter_ctx.role_index
#define NAME_INDEX s_user_manager_filter_ctx.name_index
#define PASSWORD_INDEX s_user_manager_filter_ctx.password_index

#define csv_row_get_name(row) csv_row_get(row, NAME_INDEX)
#define csv_row_get_role(row) tk_atoi(csv_row_get(row, ROLE_INDEX))
#define csv_row_get_password(row) csv_row_get(row, PASSWORD_INDEX)

static int compare_row_by_name(const char *username, csv_row_t *row)
{
    return tk_strcmp(username, csv_row_get_name(row));
}

static csv_row_t *find_row_by_name(const char *username)
{
    tk_object_t *obj = VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(s_user_manager_vm)->obj;
    return_value_if_fail(obj != NULL && username != NULL, NULL);

    return csv_file_object_find_first(obj, (tk_compare_t)compare_row_by_name, (void *)username);
}

static ret_t user_manager_save(void)
{
    tk_object_t *obj = VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(s_user_manager_vm)->obj;
    return_value_if_fail(obj != NULL, RET_BAD_PARAMS);

    return tk_object_exec(obj, TK_OBJECT_CMD_SAVE, "true");
}

static ret_t hmi_model_login_exec(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args)
{
    const char *username = tk_object_get_prop_str(obj, MODEL_PROP_LOGIN_USERNAME);
    const char *password = tk_object_get_prop_str(obj, MODEL_PROP_LOGIN_PASSWORD);
    csv_row_t *row = find_row_by_name(username);

    if (row != NULL) {
        const char *real_password = csv_row_get_password(row);
        if (tk_str_eq(password, real_password)) {
            int32_t role = csv_row_get_role(row);

            tk_object_set_prop_str(obj, MODEL_PROP_CURRENT_USERNAME, username);
            tk_object_set_prop_int(obj, MODEL_PROP_CURRENT_USERROLE, role);
            tk_object_set_prop_str(obj, MODEL_PROP_LOGIN_USERNAME, "");
            tk_object_set_prop_str(obj, MODEL_PROP_LOGIN_PASSWORD, "");

            navigator_to(args);
        } else {
            navigator_toast("Invalid Password.", 3000);
        }
    } else {
        navigator_toast("Invalid Username.", 3000);
    }

    return RET_OK;
}

static bool_t hmi_model_login_can_exec(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args)
{
    const char *username = tk_object_get_prop_str(obj, MODEL_PROP_LOGIN_USERNAME);

    return tk_strlen(username) > 0;
}

static hmi_model_cmd_t s_hmi_model_login_cmd = {
    .name = "login",
    .exec = hmi_model_login_exec,
    .can_exec = hmi_model_login_can_exec,
};

static ret_t hmi_model_change_password_exec(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args)
{
    const char *username = tk_object_get_prop_str(obj, MODEL_PROP_CURRENT_USERNAME);
    const char *password = tk_object_get_prop_str(obj, MODEL_PROP_CHANGE_PASSWORD);
    const char *confirm_password = tk_object_get_prop_str(obj, MODEL_PROP_CHANGE_CONFIRM_PASSWORD);
    csv_row_t *row = find_row_by_name(username);

    if (row != NULL && tk_str_eq(password, confirm_password)) {
        csv_row_set(row, PASSWORD_INDEX, password);
        user_manager_save();
    } else {
        navigator_toast("Unknown error.", 3000);
    }

    return RET_OK;
}

static bool_t hmi_model_change_password_can_exec(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args)
{
    const char *password = tk_object_get_prop_str(obj, MODEL_PROP_CHANGE_PASSWORD);
    const char *confirm_password = tk_object_get_prop_str(obj, MODEL_PROP_CHANGE_CONFIRM_PASSWORD);

    if (TK_STR_IS_EMPTY(password) || TK_STR_IS_EMPTY(confirm_password)) {
        return FALSE;
    }

    return tk_str_eq(password, confirm_password);
}

static hmi_model_cmd_t s_hmi_model_change_password_cmd = {
    .name = "change_password",
    .exec = hmi_model_change_password_exec,
    .can_exec = hmi_model_change_password_can_exec,
};

static ret_t hmi_model_logout_exec(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args)
{
    tk_object_set_prop_str(obj, MODEL_PROP_CURRENT_USERNAME, "nobody");
    tk_object_set_prop_int(obj, MODEL_PROP_CURRENT_USERROLE, 0xffff);
    navigator_to(args);

    return RET_OK;
}

static bool_t hmi_model_logout_can_exec(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args)
{
    return TRUE;
}

static hmi_model_cmd_t s_hmi_model_logout_cmd = {
    .name = "logout",
    .exec = hmi_model_logout_exec,
    .can_exec = hmi_model_logout_can_exec,
};

static bool_t is_valid_username(const value_t *value, str_t *msg)
{
    char buff[64] = {0};
    const char *username = value_str_ex(value, buff, sizeof(buff));

    if (strchr(username, USER_MANAGER_FEIDLS_SEPERATOR) != NULL) {
        str_set(msg, "username has invalid chars.");
        return FALSE;
    }

    if (find_row_by_name(username) != NULL) {
        str_set(msg, "username already exists.");
        return FALSE;
    } else {
        str_set(msg, "");
        return TRUE;
    }
}

static ret_t csv_file_object_check_new_user(void *ctx, csv_row_t *row)
{
    const char *username = csv_row_get_name(row);

    if (tk_strlen(username) < 3) {
        return RET_FAIL;
    }

    if (find_row_by_name(username) != NULL) {
        return RET_FAIL;
    } else {
        return RET_OK;
    }
}

static ret_t fix_username(value_t *value)
{
    return RET_OK;
}

static void *create_username_validator(void)
{
    return value_validator_delegate_create(is_valid_username, fix_username);
}

static ret_t user_manager_filter_info_init(user_manager_filter_ctx_t *info, tk_object_t *args)
{
    return_value_if_fail(info != NULL && args != NULL, RET_BAD_PARAMS);

    info->name = tk_object_get_prop_str(args, CSV_QUERY_PREFIX "name");
    info->role = tk_object_get_prop_int(args, USER_MANAGER_PROP_QUERY_ROLE, -1);

    return RET_OK;
}

static ret_t user_manager_filter(void *ctx, tk_object_t *args, uint32_t index, csv_row_t *row)
{
    user_manager_filter_ctx_t *info = (user_manager_filter_ctx_t *)ctx;

    if (index == 0) {
        user_manager_filter_info_init(info, args);
    }

    if (info->role >= 0) {
        int32_t role = csv_row_get_role(row);
        if (info->role != role) {
            return RET_FAIL;
        }
    }

    if (TK_STR_IS_NOT_EMPTY(info->name)) {
        const char *name = csv_row_get_name(row);
        if (name == NULL || strstr(name, info->name) == NULL) {
            return RET_FAIL;
        }
    }

    return RET_OK;
}

static view_model_t *view_model_user_manager_create(navigator_request_t *req)
{
    return csv_view_model_create_ex(req, s_user_manager_vm);
}

ret_t user_manager_init(tk_object_t *model, conf_doc_t *settings)
{
    uint32_t field_count = 0;
    conf_node_t *node = NULL;
    conf_node_t *iter = NULL;
    uint32_t max_rows = USER_MANAGER_MAX_ROWS;
    char sep = USER_MANAGER_FEIDLS_SEPERATOR;
    user_manager_filter_ctx_t *filter_ctx = &s_user_manager_filter_ctx;

    return_value_if_fail(model != NULL, RET_BAD_PARAMS);
    return_value_if_fail(settings != NULL, RET_BAD_PARAMS);

    if (conf_doc_get_bool(settings, USER_MANAGER_PROP_NAME ".enable", FALSE) == FALSE) {
        log_debug("user_manager is disabled\n");
        return RET_OK;
    }

    node = conf_doc_find_node(settings, settings->root, USER_MANAGER_PROP_NAME ".fields", FALSE);
    return_value_if_fail(node != NULL, RET_BAD_PARAMS);
    iter = conf_node_get_first_child(node);

    filter_ctx->name_index = 0;
    filter_ctx->role_index = 1;
    while (iter != NULL) {
        value_t v;
        const char *value = NULL;
        if (conf_node_get_value(iter, &v) != RET_OK) {
            log_debug("conf_node_get_value failed\n");
            return RET_FAIL;
        }

        value = value_str(&v);
        if (tk_str_eq(value, USER_MANAGER_FIELD_ROLE)) {
            filter_ctx->role_index = field_count;
        } else if (tk_str_eq(value, USER_MANAGER_FIELD_NAME)) {
            filter_ctx->name_index = field_count;
        } else if (tk_str_eq(value, USER_MANAGER_FIELD_PASSWORD)) {
            filter_ctx->password_index = field_count;
        }
        field_count++;
        iter = iter->next;
    }

    sep = USER_MANAGER_FEIDLS_SEPERATOR;
    max_rows = conf_doc_get_int(settings, USER_MANAGER_PROP_NAME ".max_rows", max_rows);

    log_debug("user_manager enabled with(field_count=%u max_rows=%u sep=%c)\n", field_count, max_rows,
              sep);

    s_user_manager_vm = csv_view_model_create(model, USER_MANAGER_PROP_NAME, field_count, sep,
                        max_rows, user_manager_filter, filter_ctx);
    view_model_factory_register(USER_MANAGER_PROP_NAME, view_model_user_manager_create);

    if (s_user_manager_vm != NULL) {
        tk_object_t *obj = VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(s_user_manager_vm)->obj;
        tk_object_set_prop_str(TK_OBJECT(s_user_manager_vm), CSV_QUERY_PREFIX "name", "");
        tk_object_set_prop_int(TK_OBJECT(s_user_manager_vm), CSV_QUERY_PREFIX "role", -1);
        tk_object_set_prop_str(model, MODEL_PROP_LOGIN_USERNAME, "");
        tk_object_set_prop_str(model, MODEL_PROP_LOGIN_PASSWORD, "");

        csv_file_object_set_check_new_row(obj, csv_file_object_check_new_user, NULL);
    }

    value_validator_register("username", create_username_validator);
    hmi_model_add_cmd(model, &s_hmi_model_login_cmd);
    hmi_model_add_cmd(model, &s_hmi_model_logout_cmd);
    hmi_model_add_cmd(model, &s_hmi_model_change_password_cmd);

    return s_user_manager_vm != NULL ? RET_OK : RET_FAIL;
}

ret_t user_manager_deinit(void)
{
    TK_OBJECT_UNREF(s_user_manager_vm);
    return RET_OK;
}
