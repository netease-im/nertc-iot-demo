/**
 * File:   csv_view_model.c
 * Author: AWTK Develop Team
 * Brief:  csv_view_model
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
 * 2023-11-22 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/fs.h"
#include "tkc/path.h"
#include "mvvm/mvvm.h"
#include "csv_view_model.h"
#include "csv/csv_file_object.h"
#include "csv/csv_row_object.h"
#include "tkc/func_call_parser.h"
#include "base/assets_manager.h"

static ret_t add_csv_to_view_model(void *ctx, event_t *e)
{
    prop_change_event_t *evt = (prop_change_event_t *)e;
    view_model_array_object_wrapper_t *vm = VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(ctx);

    if (vm != NULL) {
        tk_object_t *obj = TK_OBJECT(vm->obj);
        if (obj != NULL && tk_str_eq(obj->name, evt->name)) {
            csv_file_t *csv_file = csv_file_object_get_csv(vm->obj);
            log_debug("log: %s %s\n", evt->name, value_str(evt->value));
            csv_file_append_row(csv_file, value_str(evt->value));
            emitter_dispatch_simple_event(EMITTER(vm), EVT_ITEMS_CHANGED);
            emitter_dispatch_simple_event(EMITTER(vm), EVT_PROPS_CHANGED);
        }
    }

    return RET_OK;
}

static const char *csv_view_model_get_filename(const char *app_name, const char *name,
        char filename[MAX_PATH + 1])
{
    char fullpath[MAX_PATH + 1] = {0};
    return_value_if_fail(filename != NULL, NULL);
    path_prepend_user_storage_path(fullpath, app_name);

    if (!path_exist(fullpath)) {
        fs_create_dir(os_fs(), fullpath);
    }

    tk_snprintf(filename, MAX_PATH, "%s/%s.csv", fullpath, name, "csv");

    return filename;
}

static csv_file_t *csv_file_load_file_or_assets(const char *appname, const char *name,
        uint32_t cols, char separator)
{
    csv_file_t *csv_file = NULL;
    char filename[MAX_PATH + 1] = {0};
    csv_view_model_get_filename(appname, name, filename);

    log_debug("csv filename %s\n", filename);

    csv_file = csv_file_create(filename, separator);

    if (csv_file == NULL) {
        char asset_name[TK_NAME_LEN + 1] = {0};
        const asset_info_t *info = NULL;
        assets_manager_t *am = assets_manager();
        tk_snprintf(asset_name, sizeof(asset_name) - 1, "%s.csv", name);
        info = assets_manager_ref(am, ASSET_TYPE_DATA, asset_name);
        if (info != NULL) {
            csv_file = csv_file_create_with_buff((const char *)(info->data), info->size, separator);
            log_debug("load %s from assets\n", asset_name);
            asset_info_unref((asset_info_t *)info);
        }
    }

    if (csv_file == NULL) {
        csv_file = csv_file_create_empty(separator, NULL, NULL);
    }
    return_value_if_fail(csv_file != NULL, NULL);

    if (csv_file->cols < cols) {
        csv_file->cols = cols;
    }

    if (csv_file->filename == NULL) {
        csv_file->filename = tk_strdup(filename);
    }

    return csv_file;
}

view_model_t *csv_view_model_create(tk_object_t *default_modal, const char *name, uint32_t cols,
                                    char separator, uint32_t max_rows,
                                    csv_file_object_filter_t filter, void *filter_ctx)
{
    view_model_t *vm = NULL;
    tk_object_t *csv = NULL;
    csv_file_t *csv_file = NULL;
    return_value_if_fail(default_modal != NULL, NULL);

    csv_file = csv_file_load_file_or_assets(default_modal->name, name, cols, separator);
    return_value_if_fail(csv_file != NULL, NULL);

    csv = csv_file_object_create(csv_file);
    goto_error_if_fail(csv != NULL);

    csv_file_set_max_rows(csv_file, max_rows);
    tk_object_set_name(csv, name);
    vm = view_model_array_object_wrapper_create(csv);
    goto_error_if_fail(vm != NULL);

    emitter_on(EMITTER(default_modal), EVT_PROP_CHANGED, add_csv_to_view_model, vm);
    csv_file_object_set_filter(csv, filter, filter_ctx);

    return vm;
error:
    if (csv != NULL) {
        TK_OBJECT_UNREF(csv);
    }

    if (vm != NULL) {
        TK_OBJECT_UNREF(vm);
    }

    return NULL;
}

view_model_t *csv_view_model_create_ex(navigator_request_t *req, view_model_t *vm)
{
    const char *prefix = NULL;
    const char *type_and_args = NULL;
    view_model_array_object_wrapper_t *wrapper = VIEW_MODEL_ARRAY_OBJECT_WRAPPPER(vm);
    return_value_if_fail(wrapper != NULL && req != NULL, NULL);

    type_and_args = tk_object_get_prop_str(req->args, NAVIGATOR_ARG_VIEW_MODEL_TYPE);
    prefix = tk_object_get_prop_str(req->args, STR_PATH_PREFIX);

    if (type_and_args != NULL && strchr(type_and_args, '(') != NULL) {
        /*add*/
        tk_object_t *args = func_call_parse(type_and_args, tk_strlen(type_and_args));
        const char *new_args = tk_object_get_prop_str(args, "new");
        tk_object_t *obj = csv_row_object_create(wrapper->obj, new_args);
        TK_OBJECT_UNREF(args);
        tk_object_remove_prop(req->args, STR_PATH_PREFIX);
        return view_model_object_wrapper_create(obj);
    }
    if (prefix != NULL) {
        /*edit/detail*/
        return view_model_object_wrapper_create(wrapper->obj);
    } else {
        tk_object_ref(TK_OBJECT(vm));
        return vm;
    }
}
