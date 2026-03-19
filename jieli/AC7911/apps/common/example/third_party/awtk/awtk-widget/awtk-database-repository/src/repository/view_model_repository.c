/**
 * File:   view_model_repository.c
 * Author: AWTK Develop Team
 * Brief:  repository view model
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
 * 2020-09-06 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/func_call_parser.h"
#include "mvvm/base/utils.h"
#include "view_model_repository.h"
#include "mvvm/base/navigator.h"
#include "mvvm/base/navigator_request.h"
#include "repository/repository_mvvm_const.h"
#include "app_database.h"

static ret_t view_model_repository_set_prop(object_t *obj, const char *name, const value_t *v)
{
    uint32_t index = 0;
    object_t *record = NULL;
    view_model_t *view_model = VIEW_MODEL(obj);
    view_model_repository_t *vm = (view_model_repository_t *)(obj);
    repository_t *r = vm->r;

    if (view_model_array_default_set_prop(view_model, name, v) == RET_OK) {
        return RET_OK;
    }

    if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_FILTER, name)) {
        view_model_repository_set_filter(vm, value_str(v));
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_FIELDS, name)) {
        view_model_repository_set_fields(vm, value_str(v));
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_ORDERBY, name)) {
        view_model_repository_set_orderby(vm, value_str(v));
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_ASCENDING, name)) {
        view_model_repository_set_ascending(vm, value_bool(v));
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_LIMIT, name)) {
        view_model_repository_set_limit(vm, value_int(v));
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_START_ROW, name)) {
        view_model_repository_set_start_row(vm, value_int(v));
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_PROP_ITEMS, name)) {
        return RET_OK;
    }

    if (tk_str_start_with(name, VIEW_MODEL_PROP_ITEMS ".")) {
        name = destruct_array_prop_name(name + sizeof(VIEW_MODEL_PROP_ITEMS), &index);
        record = repository_get_cache_object(r, index);
        return_value_if_fail(record != NULL, RET_BAD_PARAMS);
    }

    return object_set_prop(record, name, v);
}

static ret_t view_model_repository_get_prop(object_t *obj, const char *name, value_t *v)
{
    uint32_t index = 0;
    object_t *record = NULL;
    view_model_t *view_model = VIEW_MODEL(obj);
    view_model_repository_t *vm = (view_model_repository_t *)(obj);
    repository_t *r = vm->r;

    if (view_model_array_default_get_prop(view_model, name, v) == RET_OK) {
        return RET_OK;
    }

    if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_FILTER, name)) {
        value_set_str(v, vm->filter);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_FIELDS, name)) {
        value_set_str(v, vm->fields);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_ORDERBY, name)) {
        value_set_str(v, vm->orderby);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_ASCENDING, name)) {
        value_set_bool(v, vm->ascending);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_LIMIT, name)) {
        value_set_int(v, vm->limit);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_PROP_START_ROW, name)) {
        value_set_int(v, vm->start_row);
        return RET_OK;
    } else if (tk_str_ieq(TK_OBJECT_PROP_SIZE, name)) {
        value_set_int(v, vm->r->cache.size);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_PROP_ITEMS, name)) {
        value_set_object(v, obj);
        return RET_OK;
    }

    if (tk_str_start_with(name, VIEW_MODEL_PROP_ITEMS ".")) {
        name = destruct_array_prop_name(name + sizeof(VIEW_MODEL_PROP_ITEMS), &index);
        if (index >= vm->r->cache.size) {
            /*table view滚动到最后时，不需要的row也会绑定数据，index会超出正常范围*/
            value_set_int(v, 0);
            return RET_OK;
        }
        record = repository_get_cache_object(r, index);
        return_value_if_fail(record != NULL, RET_BAD_PARAMS);
    }

    return object_get_prop(record, name, v);
}

static bool_t view_model_repository_can_exec(object_t *obj, const char *name, const char *args)
{
    view_model_repository_t *vm = (view_model_repository_t *)(obj);

    if (tk_str_ieq(OBJECT_CMD_CLEAR, name)) {
        return TRUE;
    } else if (tk_str_ieq(OBJECT_CMD_REMOVE, name)) {
        return TRUE;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_CMD_SEARCH, name)) {
        return TRUE;
    } else if (tk_str_ieq(OBJECT_CMD_DETAIL, name)) {
        return vm->window_name_of_detail != NULL;
    } else if (tk_str_ieq(OBJECT_CMD_ADD, name)) {
        return vm->window_name_of_add != NULL;
    } else if (tk_str_ieq(OBJECT_CMD_EDIT, name)) {
        return vm->window_name_of_edit != NULL;
    }

    return FALSE;
}

static ret_t on_request_done(navigator_request_t *req, const value_t *result)
{
    view_model_repository_t *vm =
        VIEW_MODEL_REPOSITORY(object_get_prop_pointer(OBJECT(req), STR_ARG_CALLER));
    view_model_repository_search(vm);
    emitter_dispatch_simple_event(EMITTER(vm), EVT_ITEMS_CHANGED);

    return RET_OK;
}

static ret_t view_model_repository_request(view_model_repository_t *vm, const char *target,
        object_t *record)
{
    navigator_request_t *req = navigator_request_create(target, on_request_done);

    object_set_prop_pointer(OBJECT(req), STR_ARG_CALLER, vm);
    object_set_prop_pointer(OBJECT(req), STR_ARG_RECORD, record);
    object_set_prop_pointer(OBJECT(req), STR_ARG_REPOSITORY, vm->r);

    navigator_to_ex(req);

    object_unref(OBJECT(req));

    return RET_OK;
}

#define STR_IS_EMPTY(s) ((s) == NULL || *(s) == '\0')

ret_t view_model_repository_gen_select(view_model_repository_t *vm, str_t *str)
{
    str_append(str, "SELECT ");
    str_append(str, vm->fields != NULL ? vm->fields : "*");
    str_append(str, " FROM ");
    str_append(str, vm->r->table_name);

    if (!STR_IS_EMPTY(vm->filter)) {
        str_append(str, " WHERE ");

        if (vm->filter != NULL) {
            str_append(str, vm->filter);
        }
    }

    if (vm->orderby != NULL && vm->orderby[0]) {
        str_append(str, " ORDER BY ");
        str_append(str, vm->orderby);
    }

    if (!vm->ascending) {
        str_append(str, " DESC ");
    }

    if (vm->limit > 0) {
        str_append(str, " LIMIT ");
        str_append_int(str, vm->start_row);
        str_append(str, ",");
        str_append_int(str, vm->limit);
    }

    str_append(str, ";");

    return RET_OK;
}

ret_t view_model_repository_search(view_model_repository_t *vm)
{
    str_t str;
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);

    str_init(&str, 1024);
    view_model_repository_gen_select(vm, &str);
    repository_select(vm->r, str.str);
    str_reset(&str);

    return RET_OK;
}

static ret_t view_model_repository_clear(view_model_repository_t *vm)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);

    repository_clear_ex(vm->r, vm->filter);

    return RET_OK;
}

static ret_t view_model_repository_exec(object_t *obj, const char *name, const char *args)
{
    view_model_repository_t *vm = (view_model_repository_t *)(obj);
    repository_t *r = vm->r;

    if (tk_str_ieq(OBJECT_CMD_CLEAR, name)) {
        view_model_repository_clear(vm);
        view_model_repository_search(vm);
        return RET_ITEMS_CHANGED;
    } else if (tk_str_ieq(OBJECT_CMD_REMOVE, name)) {
        repository_remove_index(r, tk_atoi(args));
        return RET_ITEMS_CHANGED;
    } else if (tk_str_ieq(OBJECT_CMD_DETAIL, name)) {
        int32_t index = tk_atoi(args);
        object_t *record = repository_get_cache_object(r, index);
        view_model_repository_request(vm, vm->window_name_of_detail, record);
        return RET_OK;
    } else if (tk_str_ieq(OBJECT_CMD_ADD, name)) {
        view_model_repository_request(vm, vm->window_name_of_add, NULL);
        return RET_OK;
    } else if (tk_str_ieq(OBJECT_CMD_EDIT, name)) {
        int32_t index = tk_atoi(args);
        object_t *record = repository_get_cache_object(r, index);
        view_model_repository_request(vm, vm->window_name_of_edit, record);
        return RET_OK;
    } else if (tk_str_ieq(VIEW_MODEL_REPOSITORY_CMD_SEARCH, name)) {
        view_model_repository_search(vm);
        return RET_ITEMS_CHANGED;
    }

    return RET_NOT_IMPL;
}

static ret_t view_model_repository_on_destroy(object_t *obj)
{
    view_model_repository_t *vm = (view_model_repository_t *)(obj);
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);

    view_model_deinit(VIEW_MODEL(vm));
    emitter_off_by_ctx(EMITTER(vm->r), vm);
    repository_destroy(vm->r);

    return view_model_array_deinit(VIEW_MODEL(obj));
}

static const object_vtable_t s_view_model_repository_vtable = {"view_model_repository_t",
                                                               "view_model_repository_t",
                                                               sizeof(view_model_repository_t),
                                                               TRUE,
                                                               view_model_repository_on_destroy,
                                                               NULL,
                                                               view_model_repository_get_prop,
                                                               view_model_repository_set_prop,
                                                               NULL,
                                                               NULL,
                                                               view_model_repository_can_exec,
                                                               view_model_repository_exec
                                                              };

view_model_t *view_model_repository_create_with(repository_t *r)
{
    return_value_if_fail(r != NULL, NULL);

    if (r != NULL) {
        object_t *obj = object_create(&s_view_model_repository_vtable);
        view_model_t *vm = view_model_array_init(VIEW_MODEL(obj));
        view_model_repository_t *view_model_repository = (view_model_repository_t *)(vm);

        view_model_repository->r = r;
        view_model_repository->limit = 1000;
        view_model_repository->start_row = 0;
        view_model_repository->ascending = TRUE;
        view_model_init(vm);

        return vm;
    }

    return NULL;
}

ret_t view_model_repository_set_filter(view_model_repository_t *vm, const char *filter)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->filter = tk_str_copy(vm->filter, filter);

    return RET_OK;
}

ret_t view_model_repository_set_fields(view_model_repository_t *vm, const char *fields)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->fields = tk_str_copy(vm->fields, fields);

    return RET_OK;
}

ret_t view_model_repository_set_orderby(view_model_repository_t *vm, const char *orderby)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->orderby = tk_str_copy(vm->orderby, orderby);

    return RET_OK;
}

ret_t view_model_repository_set_ascending(view_model_repository_t *vm, bool_t ascending)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->ascending = ascending;

    return RET_OK;
}

ret_t view_model_repository_set_limit(view_model_repository_t *vm, int32_t limit)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->limit = limit;

    return RET_OK;
}

ret_t view_model_repository_set_start_row(view_model_repository_t *vm, int32_t start_row)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->start_row = start_row;

    return RET_OK;
}

ret_t view_model_repository_set_window_name_of_add(view_model_repository_t *vm,
        const char *window_name)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->window_name_of_add = tk_str_copy(vm->window_name_of_add, window_name);

    return RET_OK;
}

ret_t view_model_repository_set_window_name_of_edit(view_model_repository_t *vm,
        const char *window_name)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->window_name_of_edit = tk_str_copy(vm->window_name_of_edit, window_name);

    return RET_OK;
}

ret_t view_model_repository_set_window_name_of_detail(view_model_repository_t *vm,
        const char *window_name)
{
    return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
    vm->window_name_of_detail = tk_str_copy(vm->window_name_of_detail, window_name);

    return RET_OK;
}

view_model_repository_t *view_model_repository_cast(view_model_t *vm)
{
    return_value_if_fail(vm != NULL && OBJECT(vm)->vt == &s_view_model_repository_vtable, NULL);

    return (view_model_repository_t *)vm;
}

view_model_t *view_model_repository_create_with_req(navigator_request_t *req)
{
    view_model_t *vm = NULL;
    char target[256] = {0};
    view_model_repository_t *view_model = NULL;
    const char *type_and_args = tk_object_get_prop_str(req->args, NAVIGATOR_ARG_VIEW_MODEL_TYPE);
    tk_object_t *args = func_call_parse(type_and_args, tk_strlen(type_and_args));
    const char *table = tk_object_get_prop_str(args, "table");
    const char *key = tk_object_get_prop_str(args, "key");
    uint32_t start = tk_object_get_prop_int(args, "start", 0);
    uint32_t count = tk_object_get_prop_int(args, "count", 100);
    const char *filter = tk_object_get_prop_str(args, "filter");
    const char *orderby = tk_object_get_prop_str(args, "orderby");
    const char *fields = tk_object_get_prop_str(args, "fields");
    repository_t *repository = app_database_create_repository(table, key);
    return_value_if_fail(repository != NULL, NULL);

    vm = view_model_repository_create_with(repository);

    view_model = VIEW_MODEL_REPOSITORY(vm);

    if (filter != NULL) {
        view_model_repository_set_filter(view_model, filter);
    }

    if (orderby != NULL) {
        view_model_repository_set_orderby(view_model, orderby);
    }

    if (fields != NULL) {
        view_model_repository_set_fields(view_model, fields);
    }

    view_model_repository_set_limit(view_model, count);
    view_model_repository_set_start_row(view_model, start);

    /*加载数据*/
    view_model_repository_search(view_model);

    /*如果需要支持“新建”操作，请设置点击“新建”按钮时打开窗口的名称*/
    tk_snprintf(target, sizeof(target), "%s_add", table);
    view_model_repository_set_window_name_of_add(view_model, target);

    /*如果需要支持“编辑”操作，请设置点击“编辑”按钮时打开窗口的名称*/
    tk_snprintf(target, sizeof(target), "%s_edit", table);
    view_model_repository_set_window_name_of_edit(view_model, target);

    /*如果需要支持“详情”操作，请设置点击“详情”按钮时打开窗口的名称*/
    tk_snprintf(target, sizeof(target), "%s_detail", table);
    view_model_repository_set_window_name_of_detail(view_model, target);

    return vm;
}
