/**
 * File:   hmi_model.c
 * Author: AWTK Develop Team
 * Brief:  hmi model
 *
 * Copyright (c) 2019 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY { without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2023-01-1 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/value.h"
#include "tkc/utils.h"
#include "tkc/object_default.h"
#include "conf_io/conf_node.h"
#include "conf_io/conf_json.h"

#include "base/events.h"
#include "base/assets_manager.h"
#include "base/window_manager.h"
#include "base/ui_feedback.h"

#include "hmi/hmi_model.h"
#include "hal/hmi_hal.h"

static ret_t hmi_model_load(tk_object_t *obj);

ret_t ui_on_btn_feedback(void *ctx, widget_t *widget, event_t *evt)
{
    if (evt->type == EVT_POINTER_DOWN) {
        bool_t ui_feedback = tk_object_get_prop_bool(TK_OBJECT(ctx), STR_MODEL_PROP_UI_FEEDBACK, FALSE);
        if (ui_feedback) {
            hmi_hal_beep_on(50);
        }
    }

    return RET_OK;
}

conf_doc_t *hmi_model_load_conf_from_asset(const char *asset_name)
{
    const asset_info_t *info = NULL;
    assets_manager_t *am = assets_manager();
    return_value_if_fail(asset_name != NULL, NULL);

    info = assets_manager_ref(am, ASSET_TYPE_DATA, asset_name);
    if (info != NULL) {
        conf_doc_t *doc = conf_doc_load_json((const char *)(info->data), info->size);
        asset_info_unref((asset_info_t *)info);
        return doc;
    }

    return NULL;
}

static ret_t hmi_model_on_destroy(tk_object_t *obj)
{
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    TK_OBJECT_UNREF(o->obj);
    TK_OBJECT_UNREF(o->cmds);

    return RET_OK;
}

static ret_t hmi_model_remove_prop(tk_object_t *obj, const char *name)
{
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    return tk_object_remove_prop(o->obj, name);
}

static ret_t hmi_model_set_system_prop(tk_object_t *obj, const char *name, const value_t *v)
{
    if (tk_str_eq(name, STR_MODEL_PROP_APP_NAME)) {
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_BUILD_DATE)) {
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_BUILD_TIME)) {
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_BACKLIGHT)) {
        hmi_hal_set_backlight(value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_AUDIO_VOLUME)) {
        hmi_hal_set_audio_volume(value_uint32(v));
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_SCREEN_SAVER_TIME)) {
        window_manager_set_screen_saver_time(window_manager(), value_uint32(v));
        return RET_FAIL/*继续，以便保存*/;
    } else if (tk_str_eq(name, STR_MODEL_PROP_RTC_YEAR_MONTH_DAY)) {
        uint32_t year = 0;
        uint32_t month = 0;
        uint32_t day = 0;
        const char *str = value_str(v);
        const char *format = "%u-%u-%u";

        return_value_if_fail(str != NULL, RET_BAD_PARAMS);
        if (strchr(str, '/')) {
            format = "%u/%u/%u";
        }

        if (tk_sscanf(str, format, &year, &month, &day) == 3) {
            value_t vv;
            value_set_uint32(&vv, year);
            tk_object_set_prop(obj, STR_MODEL_PROP_RTC_YEAR, &vv);

            value_set_uint32(&vv, month);
            tk_object_set_prop(obj, STR_MODEL_PROP_RTC_MONTH, &vv);

            value_set_uint32(&vv, day);
            tk_object_set_prop(obj, STR_MODEL_PROP_RTC_DAY, &vv);

            tk_object_set_prop(HMI_MODEL(obj)->obj, STR_MODEL_PROP_RTC_YEAR_MONTH_DAY, v);
        } else {
            log_debug("invalid date format\n");
        }
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_RTC_HOUR_MINUTE_SECOND)) {
        uint32_t hour = 0;
        uint32_t minute = 0;
        uint32_t second = 0;
        const char *str = value_str(v);
        const char *format = "%u:%u:%u";

        return_value_if_fail(str != NULL, RET_BAD_PARAMS);

        if (tk_sscanf(str, format, &hour, &minute, &second) == 3) {
            value_t vv;
            value_set_uint32(&vv, hour);
            tk_object_set_prop(obj, STR_MODEL_PROP_RTC_HOUR, &vv);

            value_set_uint32(&vv, minute);
            tk_object_set_prop(obj, STR_MODEL_PROP_RTC_MINUTE, &vv);

            value_set_uint32(&vv, second);
            tk_object_set_prop(obj, STR_MODEL_PROP_RTC_SECOND, &vv);

            tk_object_set_prop(HMI_MODEL(obj)->obj, STR_MODEL_PROP_RTC_HOUR_MINUTE_SECOND, v);
        } else {
            log_debug("invalid time format\n");
        }
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t hmi_model_get_system_prop(tk_object_t *obj, const char *name, value_t *v)
{
    if (tk_str_eq(name, STR_MODEL_PROP_APP_NAME)) {
        value_set_str(v, obj->name);
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_BUILD_DATE)) {
        value_set_str(v, __DATE__);
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_BUILD_TIME)) {
        value_set_str(v, __TIME__);
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_BACKLIGHT)) {
        value_set_uint32(v, hmi_hal_get_backlight());
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_AUDIO_VOLUME)) {
        value_set_uint32(v, hmi_hal_get_audio_volume());
        return RET_OK;
    } else if (tk_str_eq(name, STR_MODEL_PROP_SCREEN_SAVER_TIME)) {
        return widget_get_prop(window_manager(), WIDGET_PROP_SCREEN_SAVER_TIME, v);
    }

    return RET_NOT_FOUND;
}

static ret_t hmi_model_set_prop(tk_object_t *obj, const char *name, const value_t *v)
{
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    if (hmi_model_set_system_prop(obj, name, v) == RET_OK) {
        return RET_OK;
    }

    return tk_object_set_prop(o->obj, name, v);
}

static ret_t hmi_model_get_prop(tk_object_t *obj, const char *name, value_t *v)
{
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    if (hmi_model_get_system_prop(obj, name, v) == RET_OK) {
        return RET_OK;
    }

    return tk_object_get_prop(o->obj, name, v);
}

static ret_t hmi_model_load_from_doc(tk_object_t *model, conf_doc_t *doc)
{
    value_t v;
    conf_node_t *iter = NULL;
    return_value_if_fail(model != NULL && doc != NULL, RET_BAD_PARAMS);

    iter = conf_node_get_first_child(doc->root);
    while (iter != NULL) {
        const char *name = conf_node_get_name(iter);
        if (conf_node_get_value(iter, &v) == RET_OK) {
            tk_object_set_prop(model, name, &v);
        }

        iter = iter->next;
    }

    return RET_OK;
}

static ret_t hmi_model_reset(tk_object_t *obj)
{
    ret_t ret = RET_OK;
    conf_doc_t *doc = NULL;
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    doc = hmi_model_load_conf_from_asset(STR_DEFAULT_MODEL_ASSET);
    return_value_if_fail(doc != NULL, RET_FAIL);

    ret = hmi_model_load_from_doc(obj, doc);

    conf_doc_destroy(doc);

    return ret;
}

static ret_t hmi_model_save_with_settings(tk_object_t *obj, conf_doc_t *settings)
{
    ret_t ret = RET_OK;
    str_t str;
    conf_doc_t *doc = NULL;
    conf_node_t *iter = NULL;
    hmi_model_t *o = HMI_MODEL(obj);
    conf_node_t *persistent = conf_node_find_child(settings->root, "persistent");
    return_value_if_fail(persistent != NULL, RET_BAD_PARAMS);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    doc = conf_doc_create(30);
    iter = conf_node_get_first_child(persistent);

    while (iter != NULL) {
        value_t v;
        const char *name = conf_node_get_name(iter);
        if (conf_node_get_value(iter, &v) == RET_OK) {
            if (value_bool(&v)) {
                if (tk_object_get_prop(obj, name, &v) == RET_OK) {
                    conf_doc_set(doc, name, &v);
                } else {
                    log_debug("can not find prop: %s\n", name);
                }
            } else {
                log_debug("skip prop: %s as its value is false\n", name);
            }
        }

        iter = iter->next;
    }

    str_init(&str, 1024);
    conf_doc_save_json(doc, &str);
    conf_doc_destroy(doc);
    ret = hmi_hal_save_settings(obj->name, str.str, str.size);
    str_reset(&str);

    return ret;
}

ret_t hmi_model_save(tk_object_t *obj)
{
    ret_t ret = RET_OK;
    hmi_model_t *o = HMI_MODEL(obj);
    conf_doc_t *settings = NULL;
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    settings = hmi_model_load_conf_from_asset(STR_SETTINGS_ASSET);
    return_value_if_fail(settings != NULL, RET_FAIL);

    ret = hmi_model_save_with_settings(obj, settings);

    conf_doc_destroy(settings);

    return ret;
}

static bool_t hmi_model_can_exec(tk_object_t *obj, const char *name, const char *args)
{
    bool_t ret = FALSE;
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, STR_MODEL_CMD_SAVE)) {
        ret = TRUE;
    } else if (tk_str_eq(name, STR_MODEL_CMD_RESET)) {
        ret = TRUE;
    } else if (tk_str_eq(name, STR_MODEL_CMD_RELOAD)) {
        ret = TRUE;
    } else if (tk_str_eq(name, STR_MODEL_CMD_GET_RTC_TIME)) {
        ret = TRUE;
    } else if (tk_str_eq(name, STR_MODEL_CMD_SET_RTC_TIME)) {
        ret = TRUE;
    } else {
        hmi_model_cmd_t *cmd = NULL;
        cmd = (hmi_model_cmd_t *)tk_object_get_prop_pointer(o->cmds, name);
        if (cmd != NULL) {
            ret = cmd->can_exec(cmd, obj, args);
        }
    }

    return ret;
}

static ret_t hmi_model_set_rtc(tk_object_t *obj)
{
    value_t v;
    date_time_t dt;
    ret_t ret = RET_OK;
    value_set_int32(&v, 0);

    ret = tk_object_get_prop(obj, STR_MODEL_PROP_RTC_YEAR, &v);
    return_value_if_fail(ret == RET_OK, ret);
    dt.year = value_uint32(&v);

    ret = tk_object_get_prop(obj, STR_MODEL_PROP_RTC_MONTH, &v);
    return_value_if_fail(ret == RET_OK, ret);
    dt.month = value_uint32(&v);

    ret = tk_object_get_prop(obj, STR_MODEL_PROP_RTC_DAY, &v);
    return_value_if_fail(ret == RET_OK, ret);
    dt.day = value_uint32(&v);

    ret = tk_object_get_prop(obj, STR_MODEL_PROP_RTC_HOUR, &v);
    dt.hour = value_uint32(&v);
    return_value_if_fail(ret == RET_OK, ret);

    ret = tk_object_get_prop(obj, STR_MODEL_PROP_RTC_MINUTE, &v);
    return_value_if_fail(ret == RET_OK, ret);
    dt.minute = value_uint32(&v);

    ret = tk_object_get_prop(obj, STR_MODEL_PROP_RTC_SECOND, &v);
    return_value_if_fail(ret == RET_OK, ret);
    dt.second = value_uint32(&v);

    return hmi_hal_set_rtc(&dt);
}

static ret_t hmi_model_get_rtc(tk_object_t *obj)
{
    value_t v;
    date_time_t dt;
    ret_t ret = RET_OK;
    ret = hmi_hal_get_rtc(&dt);

    if (ret == RET_OK) {
        value_set_uint32(&v, dt.year);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_YEAR, &v);

        value_set_uint32(&v, dt.month);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_MONTH, &v);

        value_set_uint32(&v, dt.day);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_DAY, &v);

        value_set_uint32(&v, dt.hour);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_HOUR, &v);

        value_set_uint32(&v, dt.minute);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_MINUTE, &v);

        value_set_uint32(&v, dt.second);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_SECOND, &v);

        value_set_uint32(&v, dt.wday);
        tk_object_set_prop(obj, STR_MODEL_PROP_RTC_WEEK, &v);
    }

    return ret;
}

static ret_t hmi_model_exec(tk_object_t *obj, const char *name, const char *args)
{
    ret_t ret = RET_NOT_FOUND;
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, STR_MODEL_CMD_SAVE)) {
        ret = hmi_model_save(obj);
    } else if (tk_str_eq(name, STR_MODEL_CMD_RESET)) {
        ret = hmi_model_reset(obj);
    } else if (tk_str_eq(name, STR_MODEL_CMD_RELOAD)) {
        ret = hmi_model_load(obj);
    } else if (tk_str_eq(name, STR_MODEL_CMD_GET_RTC_TIME)) {
        return hmi_model_get_rtc(obj);
    } else if (tk_str_eq(name, STR_MODEL_CMD_SET_RTC_TIME)) {
        return hmi_model_set_rtc(obj);
    } else if (tk_str_eq(name, STR_MODEL_CMD_PLAY_AUDIO)) {
        return hmi_hal_play_audio(args);
    } else if (tk_str_eq(name, STR_MODEL_CMD_BEEP_ON)) {
        return hmi_hal_beep_on(tk_atoi(args));
    } else if (tk_str_eq(name, STR_MODEL_CMD_STOP_AUDIO)) {
        return hmi_hal_stop_audio();
    } else {
        hmi_model_cmd_t *cmd = NULL;
        cmd = (hmi_model_cmd_t *)tk_object_get_prop_pointer(o->cmds, name);
        if (cmd != NULL) {
            return cmd->exec(cmd, obj, args);
        }
    }

    return ret;
}

static ret_t hmi_model_foreach_prop(tk_object_t *obj, tk_visit_t on_prop, void *ctx)
{
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL && on_prop != NULL, RET_BAD_PARAMS);

    return tk_object_foreach_prop(o->obj, on_prop, ctx);
}

static const object_vtable_t s_hmi_model_vtable = {.type = "hmi_model",
                                                   .desc = "hmi_model",
                                                   .size = sizeof(hmi_model_t),
                                                   .is_collection = FALSE,
                                                   .on_destroy = hmi_model_on_destroy,
                                                   .get_prop = hmi_model_get_prop,
                                                   .set_prop = hmi_model_set_prop,
                                                   .can_exec = hmi_model_can_exec,
                                                   .exec = hmi_model_exec,
                                                   .remove_prop = hmi_model_remove_prop,
                                                   .foreach_prop = hmi_model_foreach_prop
                                                  };

static ret_t hmi_model_load(tk_object_t *obj)
{
    void *data = NULL;
    uint32_t size = 0;
    ret_t ret = RET_OK;
    conf_doc_t *settings = NULL;
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL, RET_BAD_PARAMS);

    if (hmi_hal_load_setttings(obj->name, &data, &size) != RET_OK) {
        return RET_FAIL;
    }

    settings = conf_doc_load_json(data, size);
    TKMEM_FREE(data);
    return_value_if_fail(settings != NULL, RET_FAIL);

    ret = hmi_model_load_from_doc(obj, settings);

    conf_doc_destroy(settings);

    return ret;
}

tk_object_t *hmi_model_create(const char *app_name)
{
    tk_object_t *obj = tk_object_create(&s_hmi_model_vtable);
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(obj != NULL, NULL);

    o->obj = object_default_create_ex(FALSE);
    o->cmds = object_default_create_ex(FALSE);

    if (o->obj != NULL) {
        tk_object_set_name(obj, app_name);
        object_default_set_keep_prop_type(o->obj, TRUE);
        hmi_model_reset(obj);
        hmi_model_load(obj);

        hmi_model_get_rtc(obj);
        ui_feedback_init(ui_on_btn_feedback, o->obj);
    } else {
        TK_OBJECT_UNREF(o);
    }

    return obj;
}

hmi_model_t *hmi_model_cast(tk_object_t *obj)
{
    return_value_if_fail(obj != NULL && obj->vt == &s_hmi_model_vtable, NULL);
    return (hmi_model_t *)(obj);
}

ret_t hmi_model_add_cmd(tk_object_t *obj, hmi_model_cmd_t *cmd)
{
    hmi_model_t *o = HMI_MODEL(obj);
    return_value_if_fail(o != NULL && cmd != NULL, RET_BAD_PARAMS);
    return_value_if_fail(o->cmds != NULL, RET_BAD_PARAMS);

    return tk_object_set_prop_pointer_ex(o->cmds, cmd->name, cmd, cmd->destroy);
}