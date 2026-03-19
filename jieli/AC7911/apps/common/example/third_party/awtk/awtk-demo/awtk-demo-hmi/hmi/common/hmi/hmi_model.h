/**
 * File:   hmi_model.h
 * Author: AWTK Develop Team
 * Brief:  hmi model
 *
 * Copyright (c) 2019 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2023-01-1 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_HMI_MODEL_H
#define TK_HMI_MODEL_H

#include "tkc/object.h"
#include "tkc/darray.h"
#include "tkc/named_value.h"

BEGIN_C_DECLS

struct _hmi_model_cmd_t;
typedef struct _hmi_model_cmd_t hmi_model_cmd_t;

typedef ret_t (*hmi_model_cmd_exec_t)(hmi_model_cmd_t *cmd, tk_object_t *obj, const char *args);
typedef bool_t (*hmi_model_cmd_can_exec_t)(hmi_model_cmd_t *cmd, tk_object_t *obj,
        const char *args);
typedef bool_t (*hmi_model_cmd_destroy_t)(hmi_model_cmd_t *cmd);

struct _hmi_model_cmd_t {
    const char *name;
    hmi_model_cmd_exec_t exec;
    hmi_model_cmd_can_exec_t can_exec;
    hmi_model_cmd_destroy_t destroy;
};

/**
 * @class hmi_model_t
 * @parent tk_object_t
 * @annotation ["scriptable"]
 *
 * HMI 模型
 *
 */
typedef struct _hmi_model_t {
    tk_object_t object;

    /*private*/
    tk_object_t *obj;
    tk_object_t *cmds;
} hmi_model_t;

/**
 * @method hmi_model_create
 *
 * 创建对象。
 * @param {const char*} app_name 应用名称。
 * @annotation ["constructor", "scriptable", "gc"]
 *
 * @return {tk_object_t*} 返回object对象。
 *
 */
tk_object_t *hmi_model_create(const char *app_name);

/**
 * @method hmi_model_cast
 * 转换为hmi_model对象。
 * @annotation ["cast"]
 * @param {tk_object_t*} obj hmi_model对象。
 *
 * @return {hmi_model_t*} hmi_model对象。
 */
hmi_model_t *hmi_model_cast(tk_object_t *obj);

/**
 * @method hmi_model_save
 * 保存数据。
 * @param {const char*} name 模型名称。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_model_save(tk_object_t *obj);

/**
 * @method hmi_model_add_cmd
 * 添加命令。
 * @param {tk_object_t*} obj hmi_model对象。
 * @param {hmi_model_cmd_t*} cmd 命令。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_model_add_cmd(tk_object_t *obj, hmi_model_cmd_t *cmd);

/**
 * @method hmi_model_load_conf_from_asset
 * 从资源文件加载配置。
 * @annotation ["scriptable"]
 * @param {const char*} asset_name 资源文件名。
 *
 * @return {conf_doc_t*} 返回配置对象。
 */
conf_doc_t *hmi_model_load_conf_from_asset(const char *asset_name);

#define HMI_MODEL(obj) hmi_model_cast(obj)

/*默认模型资源名*/
#define STR_DEFAULT_MODEL_ASSET "default_model.json"

/*配置文件资源名*/
#define STR_SETTINGS_ASSET "settings.json"

/*内置属性*/
#define STR_MODEL_PROP_APP_NAME "app_name"
#define STR_MODEL_PROP_BUILD_DATE "build_date"
#define STR_MODEL_PROP_BUILD_TIME "build_time"
#define STR_MODEL_PROP_RTC_YEAR "rtc_year"
#define STR_MODEL_PROP_RTC_MONTH "rtc_month"
#define STR_MODEL_PROP_RTC_DAY "rtc_day"
#define STR_MODEL_PROP_RTC_HOUR "rtc_hour"
#define STR_MODEL_PROP_RTC_MINUTE "rtc_minute"
#define STR_MODEL_PROP_RTC_SECOND "rtc_second"
#define STR_MODEL_PROP_RTC_WEEK "rtc_week"
#define STR_MODEL_PROP_RTC_YEAR_MONTH_DAY "rtc_ymd"
#define STR_MODEL_PROP_RTC_HOUR_MINUTE_SECOND "rtc_hms"
#define STR_MODEL_PROP_UI_FEEDBACK "ui_feedback"
#define STR_MODEL_PROP_BACKLIGHT "backlight"
#define STR_MODEL_PROP_AUDIO_VOLUME "audio_volume"
#define STR_MODEL_PROP_SCREEN_SAVER_TIME "screen_saver_time"
#define STR_MODEL_CMD_SET_RTC_TIME "set_rtc"
#define STR_MODEL_CMD_GET_RTC_TIME "get_rtc"

#define STR_MODEL_CMD_RESET "reset"
#define STR_MODEL_CMD_SAVE "save"
#define STR_MODEL_CMD_RELOAD "reload"
#define STR_MODEL_CMD_BEEP_ON "beep_on"
#define STR_MODEL_CMD_PLAY_AUDIO "play_audio"
#define STR_MODEL_CMD_STOP_AUDIO "stop_audio"

END_C_DECLS

#endif /*TK_HMI_MODEL_H*/
