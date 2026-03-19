/**
 * File:   hmi_service.c
 * Author: AWTK Develop Team
 * Brief:  hmi_service
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
 * 2023-11-18 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/events.h"
#include "base/main_loop.h"
#include "base/window.h"
#include "base/window_manager.h"
#include "tkc/socket_helper.h"
#include "mvvm/mvvm.h"
#include "conf_io/conf_json.h"

#include "hal/hmi_hal.h"
#include "hmi/hmi_model.h"
#include "hmi/hmi_service.h"
#include "hmi/log_message.h"
#include "hmi/user_manager.h"
#include "hmi/history_data.h"
#include "hmi/hmi_db.h"

#include "table_view_register.h"
#include "table_client_custom_binder.h"
#include "slidable_row_register.h"
#include "streams/serial/iostream_serial.h"
#include "remote_ui/service/remote_ui_service.h"
#include "mvvm/base/view_model_object_wrapper.h"

typedef struct _view_model_entry_t {
    char name[TK_NAME_LEN + 1];
    tk_object_t *vm;
} view_model_entry_t;

static view_model_entry_t *view_model_entry_create(const char *name, view_model_t *vm)
{
    view_model_entry_t *entry = NULL;
    return_value_if_fail(name != NULL, NULL);
    return_value_if_fail(vm != NULL, NULL);
    entry = TKMEM_ZALLOC(view_model_entry_t);
    return_value_if_fail(entry != NULL, NULL);

    entry->vm = TK_OBJECT(vm);
    tk_strncpy(entry->name, name, TK_NAME_LEN);

    return entry;
}

static int view_model_entry_compare_by_name(const void *l, const void *r)
{
    const view_model_entry_t *left = l;
    const view_model_entry_t *right = r;
    return_value_if_fail(left != NULL && right != NULL, -1);

    return tk_strcmp(left->name, right->name);
}

static int view_model_entry_compare_by_model(const void *l, const void *r)
{
    const view_model_entry_t *left = l;
    const view_model_entry_t *right = r;
    return_value_if_fail(left != NULL && right != NULL, -1);

    return left->vm - right->vm;
}

ret_t view_model_entry_destroy(view_model_entry_t *entry)
{
    if (entry != NULL) {
        TK_OBJECT_UNREF(entry->vm);
        TKMEM_FREE(entry);
    }

    return RET_OK;
}

static darray_t s_view_models;
static tk_service_t *s_remote_ui_service;

#define STR_GLOBAL_MODEL "global.model"

static ret_t hmi_service_add_view_model(const char *name, view_model_t *vm)
{
    view_model_entry_t *entry = NULL;
    return_value_if_fail(name != NULL && vm != NULL, RET_BAD_PARAMS);

    entry = view_model_entry_create(name, vm);
    return_value_if_fail(entry != NULL, RET_OOM);

    return darray_push(&s_view_models, entry);
}

static tk_object_t *hmi_service_find_view_model(const char *name)
{
    view_model_entry_t entry;
    view_model_entry_t *ret = NULL;
    return_value_if_fail(name != NULL, NULL);

    tk_strncpy(entry.name, name, TK_NAME_LEN);
    entry.vm = NULL;

    ret =
        (view_model_entry_t *)darray_find_ex(&s_view_models, view_model_entry_compare_by_name, &entry);
    if (ret != NULL) {
        return ret->vm;
    } else {
        return NULL;
    }
}

static tk_object_t *hmi_service_get_default_view_model(void)
{
    return hmi_service_find_view_model(STR_GLOBAL_MODEL);
}

tk_object_t *hmi_service_get_default_model(void)
{
    tk_object_t *vm = hmi_service_get_default_view_model();
    return_value_if_fail(vm != NULL, NULL);

    return VIEW_MODEL_OBJECT_WRAPPPER(vm)->obj;
}

static ret_t log_set_prop(void *ctx, event_t *e)
{
    /*仅用于PC调试*/
#ifdef TK_IS_PC
    char buff[64] = {0};
    prop_change_event_t *evt = (prop_change_event_t *)e;

    log_debug("%s %s\n", evt->name, value_str_ex(evt->value, buff, sizeof(buff) - 1));
#endif /*TK_IS_PC*/

    return RET_OK;
}

static view_model_t *view_model_default_create(navigator_request_t *req)
{
    tk_object_t *vm = hmi_service_get_default_view_model();
    return_value_if_fail(vm != NULL, NULL);
    tk_object_ref(vm);

    return VIEW_MODEL(vm);
}

static tk_object_t *hmi_service_find_target(tk_service_t *service, const char *target)
{
    if (tk_str_eq(target, STR_GLOBAL_MODEL)) {
        return hmi_service_get_default_model();
    } else {
        return hmi_service_find_view_model(target);
    }
}

static ret_t hmi_service_on_login(tk_service_t *service, const char *username,
                                  const char *password)
{
    /*如果需要认证，请在此处理*/
    return RET_OK;
}

static ret_t hmi_service_on_logout(tk_service_t *service)
{
    uint32_t i = 0;
    uint32_t n = s_view_models.size;
    tk_object_t *model = hmi_service_get_default_model();

    emitter_off_by_ctx(EMITTER(model), service);
    for (i = 0; i < n; i++) {
        view_model_entry_t *entry = darray_get(&s_view_models, i);
        if (entry != NULL) {
            emitter_off_by_ctx(EMITTER(entry->vm), service);
        }
    }

    return RET_OK;
}

static ret_t hmi_service_start(const char *uart_device)
{
    const char *remote_ui_url = REMOTE_UI_URL;
    static remote_ui_service_args_t args = {0};
    args.auth = hmi_service_on_login;
    args.logout = hmi_service_on_logout;
    args.find_target = hmi_service_find_target;

#if defined(TK_IS_PC) || defined(LINUX)
    if (getenv("REMOTE_UI_URL") != NULL) {
        remote_ui_url = getenv("REMOTE_UI_URL");
    } else {
        remote_ui_url = REMOTE_UI_URL;
    }
#else
    remote_ui_url = NULL;
#endif /*TK_IS_PC*/

    if (remote_ui_url != NULL) {
        event_source_manager_t *esm = main_loop_get_event_source_manager(main_loop());
        tk_service_start(esm, remote_ui_url, remote_ui_service_create, &args);
    } else {
        tk_iostream_t *io = tk_iostream_serial_create(uart_device);
        return_value_if_fail(io != NULL, RET_BAD_PARAMS);
        s_remote_ui_service = remote_ui_service_start_with_uart(io, &args);
        return_value_if_fail(s_remote_ui_service != NULL, RET_BAD_PARAMS);
    }

    return RET_OK;
}

#define STR_SCREEN_SAVER_WINDOW "screen_saver"

static ret_t close_window_on_event(void *ctx, event_t *e)
{
    window_close(WIDGET(ctx));
    return RET_OK;
}

static ret_t hmi_service_on_screen_saver(void *ctx, event_t *e)
{
    widget_t *win = NULL;

    if (widget_child(window_manager(), STR_SCREEN_SAVER_WINDOW) != NULL) {
        log_debug("screen saver exist.\n");
        return RET_OK;
    }

    navigator_to(STR_SCREEN_SAVER_WINDOW);

    win = widget_child(window_manager(), STR_SCREEN_SAVER_WINDOW);
    if (win == NULL) {
        log_debug("%s asset not exist, disable screen saver.\n", STR_SCREEN_SAVER_WINDOW);
        return RET_REMOVE;
    }

    return RET_OK;
}

ret_t hmi_service_init(const char *uart_device)
{
    ret_t ret = RET_OK;
    tk_object_t *model = NULL;
    conf_doc_t *settings = NULL;
    view_model_t *view_model = NULL;
    const char *app_name = "hmi_app";

    table_view_register();
    slidable_row_register();
    table_client_custom_binder_register();

    return_value_if_fail(hmi_service_start(uart_device) == RET_OK, RET_BAD_PARAMS);

    hmi_hal_init();
    darray_init(&s_view_models, 0, (tk_destroy_t)view_model_entry_destroy, NULL);

    settings = hmi_model_load_conf_from_asset(STR_SETTINGS_ASSET);
    if (settings != NULL) {
        app_name = conf_doc_get_str(settings, "name", app_name);
    }

    model = hmi_model_create(app_name);
    return_value_if_fail(model != NULL, RET_BAD_PARAMS);

    view_model = view_model_object_wrapper_create(model);
    ENSURE(view_model != NULL);

    ret = view_model_factory_register("default", view_model_default_create);
    emitter_on(EMITTER(model), EVT_PROP_CHANGED, log_set_prop, NULL);

    hmi_service_add_view_model(STR_GLOBAL_MODEL, view_model);

    if (settings != NULL) {
#ifndef HMI_ZDP1440D
        if (os_fs() != NULL) {
            hmi_db_init(model, settings);
            log_message_init(model, settings);
            user_manager_init(model, settings);
            history_data_init(model, settings);
        }
#endif /*HMI_ZDP1440D*/
        conf_doc_destroy(settings);
    }
    widget_on(window_manager(), EVT_SCREEN_SAVER, hmi_service_on_screen_saver, NULL);

    TK_OBJECT_UNREF(model);

    return ret;
}

ret_t hmi_service_deinit(void)
{
    if (s_remote_ui_service != NULL) {
        tk_service_destroy(s_remote_ui_service);
    }

    hmi_model_save(hmi_service_get_default_model());
    darray_deinit(&s_view_models);
    hmi_hal_deinit();

    return RET_OK;
}
