/**
 * File:   view_model_record.c
 * Author: AWTK Develop Team
 * Brief:  view_model_record
 *
 * Copyright (c) 2019 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-09-08 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/fscript.h"
#include "tkc/utils.h"
#include "tkc/object_default.h"
#include "tkc/func_call_parser.h"
#include "repository/view_model_record.h"
#include "repository/repository_mvvm_const.h"

static ret_t view_model_record_on_destroy(object_t *obj)
{
    view_model_record_t *record = VIEW_MODEL_RECORD(obj);

    view_model_deinit(VIEW_MODEL(record));
    OBJECT_UNREF(record->o);

    return RET_OK;
}

static int32_t view_model_record_compare(object_t *obj, object_t *other)
{
    return tk_str_cmp(obj->name, other->name);
}

static ret_t view_model_record_set_prop(object_t *obj, const char *name, const value_t *v)
{
    view_model_record_t *record = VIEW_MODEL_RECORD(obj);
    return_value_if_fail(obj != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    return object_set_prop(OBJECT(record->o), name, v);
}

static ret_t view_model_record_get_prop(object_t *obj, const char *name, value_t *v)
{
    view_model_record_t *record = VIEW_MODEL_RECORD(obj);
    return_value_if_fail(obj != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);
    return object_get_prop(OBJECT(record->o), name, v);
}

static bool_t view_model_record_can_exec(object_t *obj, const char *name, const char *args)
{
    view_model_record_t *record = VIEW_MODEL_RECORD(obj);
    return_value_if_fail(obj != NULL && name != NULL, FALSE);

    if (args != NULL) {
        value_t v;
        if (fscript_eval(record->o, args, &v) == RET_OK) {
            if (!value_bool(&v)) {
                return FALSE;
            }
        }
    }

    if (tk_str_ieq(OBJECT_CMD_ADD, name)) {
        return !repository_exist(record->r, record->o);
    } else if (tk_str_ieq(OBJECT_CMD_SAVE, name)) {
        return repository_exist(record->r, record->o);
    }

    return FALSE;
}

static ret_t view_model_record_exec(object_t *obj, const char *name, const char *args)
{
    value_t v;
    view_model_record_t *record = VIEW_MODEL_RECORD(obj);
    return_value_if_fail(obj != NULL && name != NULL, RET_BAD_PARAMS);

    value_set_bool(&v, TRUE);
    if (tk_str_ieq(OBJECT_CMD_ADD, name)) {
        repository_insert(record->r, record->o);
        navigator_request_on_result(record->req, &v);
        return RET_OK;
    } else if (tk_str_ieq(OBJECT_CMD_SAVE, name)) {
        repository_update(record->r, record->o);
        navigator_request_on_result(record->req, &v);
        return RET_OK;
    }

    return RET_NOT_IMPL;
}

static const view_model_vtable_t s_view_model_vtable;

static const object_vtable_t s_object_vtable = {.type = "view_model_record",
                                                .desc = "view_model_record",
                                                .size = sizeof(view_model_record_t),
                                                .is_collection = FALSE,
                                                .on_destroy = view_model_record_on_destroy,
                                                .compare = view_model_record_compare,
                                                .get_prop = view_model_record_get_prop,
                                                .set_prop = view_model_record_set_prop,
                                                .can_exec = view_model_record_can_exec,
                                                .exec = view_model_record_exec
                                               };

view_model_t *view_model_record_create(navigator_request_t *req)
{
    object_t *model = object_create(&s_object_vtable);
    view_model_record_t *record = VIEW_MODEL_RECORD(model);
    view_model_t *view_model = VIEW_MODEL(model);
    const char *type_and_args = tk_object_get_prop_str(req->args, NAVIGATOR_ARG_VIEW_MODEL_TYPE);

    return_value_if_fail(record != NULL, NULL);

    assert(req != NULL);
    view_model->vt = &s_view_model_vtable;

    record->req = req;
    record->o = (object_t *)object_get_prop_pointer(OBJECT(req), STR_ARG_RECORD);
    record->r = (repository_t *)object_get_prop_pointer(OBJECT(req), STR_ARG_REPOSITORY);

    if (record->o != NULL) {
        record->o = object_ref(record->o);
    } else {
        if (type_and_args != NULL && strchr(type_and_args, '(') != NULL) {
            record->o = func_call_parse(type_and_args, tk_strlen(type_and_args));
        } else {
            record->o = repository_create_object(record->r);
        }
    }

    view_model_init(view_model);

    return view_model;
}
