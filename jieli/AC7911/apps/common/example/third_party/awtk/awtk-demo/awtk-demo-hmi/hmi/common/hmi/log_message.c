/**
 * File:   log_message.c
 * Author: AWTK Develop Team
 * Brief:  log_message
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

#include "mvvm/mvvm.h"
#include "tkc/date_time.h"
#include "log_message.h"
#include "csv_view_model.h"
#include "csv/csv_file_object.h"

/*默认参数*/
/*默认用|分隔，因为|比较少用，如果使用','分隔，message则需要用英文引起来*/
#define LOG_MESSAGE_FEIDLS_SEPERATOR '|'

/*最大记录数*/
#define LOG_MESSAGE_MAX_ROWS 1000

typedef struct _log_message_filter_ctx {
    int32_t level;
    int64_t start_date_time;
    int64_t end_date_time;
    const char *keywords;
    const char *device;

    uint32_t level_index;
    uint32_t time_index;
    uint32_t device_index;
    uint32_t message_index;
} log_message_filter_ctx_t;

static view_model_t *s_log_message_vm = NULL;
static log_message_filter_ctx_t s_log_message_filter_ctx;

static ret_t log_message_filter_info_init(log_message_filter_ctx_t *info, tk_object_t *args)
{
    date_time_t dt;
    return_value_if_fail(info != NULL && args != NULL, RET_BAD_PARAMS);

    const char *str_start_date = tk_object_get_prop_str(args, LOG_MESSAGE_PROP_QUERY_START_DATE);
    const char *str_start_time = tk_object_get_prop_str(args, LOG_MESSAGE_PROP_QUERY_START_TIME);
    const char *str_end_date = tk_object_get_prop_str(args, LOG_MESSAGE_PROP_QUERY_END_DATE);
    const char *str_end_time = tk_object_get_prop_str(args, LOG_MESSAGE_PROP_QUERY_END_TIME);

    date_time_init(&dt);
    info->start_date_time = -1;
    info->end_date_time = -1;
    info->keywords = NULL;
    info->level = -1;

    if (str_start_time != NULL) {
        date_time_parse_time(&dt, str_start_time);
    }

    if (str_start_date != NULL) {
        date_time_parse_date(&dt, str_start_date);
        info->start_date_time = date_time_to_time(&dt);
    }

    date_time_init(&dt);
    if (str_end_time != NULL) {
        date_time_parse_time(&dt, str_end_time);
    }

    if (str_end_date != NULL) {
        date_time_parse_date(&dt, str_end_date);
        info->end_date_time = date_time_to_time(&dt);
    }

    info->keywords = tk_object_get_prop_str(args, CSV_QUERY_PREFIX "keywords");
    info->device = tk_object_get_prop_str(args, CSV_QUERY_PREFIX "device");
    info->level = tk_object_get_prop_int(args, LOG_MESSAGE_PROP_QUERY_LEVEL, -1);

    return RET_OK;
}

static ret_t log_message_filter(void *ctx, tk_object_t *args, uint32_t index, csv_row_t *row)
{
    log_message_filter_ctx_t *info = (log_message_filter_ctx_t *)ctx;

    if (index == 0) {
        log_message_filter_info_init(info, args);
    }

    if (info->level >= 0) {
        const char *level = csv_row_get(row, info->level_index);
        if (info->level != tk_atoi(level)) {
            return RET_FAIL;
        }
    }

    if (TK_STR_IS_NOT_EMPTY(info->keywords)) {
        const char *message = csv_row_get(row, info->message_index);
        if (message == NULL || strstr(message, info->keywords) == NULL) {
            return RET_FAIL;
        }
    }

    if (TK_STR_IS_NOT_EMPTY(info->device)) {
        const char *device = csv_row_get(row, info->device_index);
        if (device == NULL || strstr(device, info->device) == NULL) {
            return RET_FAIL;
        }
    }

    if (info->end_date_time > 0) {
        const char *date_time = csv_row_get(row, info->time_index);
        int64_t row_date_time = tk_atol(date_time);

        if (row_date_time > info->end_date_time) {
            return RET_FAIL;
        }
    }

    if (info->start_date_time > 0) {
        const char *date_time = csv_row_get(row, info->time_index);
        int64_t row_date_time = tk_atol(date_time);

        if (row_date_time < info->start_date_time) {
            return RET_FAIL;
        }
    }

    if (info->end_date_time > 0 && info->start_date_time > 0) {
        if (info->end_date_time < info->start_date_time) {
            return RET_FAIL;
        }
    }

    return RET_OK;
}

static view_model_t *view_model_log_message_create(navigator_request_t *req)
{
    return csv_view_model_create_ex(req, s_log_message_vm);
}

static ret_t auto_save_log_message(const timer_info_t *timer)
{
    view_model_t *vm = s_log_message_vm;
    if (vm != NULL) {
        view_model_exec(vm, TK_OBJECT_CMD_SAVE, NULL);
        return RET_REPEAT;
    } else {
        return RET_REMOVE;
    }
}

ret_t log_message_init(tk_object_t *model, conf_doc_t *settings)
{
    uint32_t field_count = 0;
    conf_node_t *node = NULL;
    conf_node_t *iter = NULL;
    uint32_t auto_save_interval = 0;
    uint32_t max_rows = LOG_MESSAGE_MAX_ROWS;
    char sep = LOG_MESSAGE_FEIDLS_SEPERATOR;
    log_message_filter_ctx_t *filter_ctx = &s_log_message_filter_ctx;

    return_value_if_fail(model != NULL, RET_BAD_PARAMS);
    return_value_if_fail(settings != NULL, RET_BAD_PARAMS);

    if (conf_doc_get_bool(settings, LOG_MESSAGE_PROP_NAME ".enable", FALSE) == FALSE) {
        log_debug("log_message is disabled\n");
        return RET_OK;
    }
    auto_save_interval = conf_doc_get_int(settings, LOG_MESSAGE_PROP_NAME ".auto_save_interval", 0);
    if (auto_save_interval > 0) {
        timer_add(auto_save_log_message, NULL, auto_save_interval);
    }

    node = conf_doc_find_node(settings, settings->root, LOG_MESSAGE_PROP_NAME ".fields", FALSE);
    return_value_if_fail(node != NULL, RET_BAD_PARAMS);
    iter = conf_node_get_first_child(node);

    filter_ctx->level_index = 0;
    filter_ctx->time_index = 1;
    filter_ctx->device_index = 2;
    filter_ctx->message_index = 3;
    while (iter != NULL) {
        value_t v;
        const char *value = NULL;
        if (conf_node_get_value(iter, &v) != RET_OK) {
            log_debug("conf_node_get_value failed\n");
            return RET_FAIL;
        }

        value = value_str(&v);
        if (tk_str_eq(value, LOG_MESSAGE_FIELD_LEVEL)) {
            filter_ctx->level_index = field_count;
        } else if (tk_str_eq(value, LOG_MESSAGE_FIELD_TIME)) {
            filter_ctx->time_index = field_count;
        } else if (tk_str_eq(value, LOG_MESSAGE_FIELD_DEVICE)) {
            filter_ctx->device_index = field_count;
        } else if (tk_str_eq(value, LOG_MESSAGE_FIELD_MESSAGE)) {
            filter_ctx->message_index = field_count;
        }
        field_count++;
        iter = iter->next;
    }

    max_rows = conf_doc_get_int(settings, LOG_MESSAGE_PROP_NAME ".max_rows", max_rows);
    sep = conf_doc_get_str(settings, LOG_MESSAGE_PROP_NAME ".fields_seperator", "|")[0];

    log_debug("log_message enabled with(field_count=%u max_rows=%u sep=%c)\n", field_count, max_rows,
              sep);

    s_log_message_vm = csv_view_model_create(model, LOG_MESSAGE_PROP_NAME, field_count, sep, max_rows,
                       log_message_filter, filter_ctx);
    view_model_factory_register(LOG_MESSAGE_PROP_NAME, view_model_log_message_create);

    if (s_log_message_vm != NULL) {
        char today[64] = {0};
        date_time_t dt;
        date_time_init(&dt);
        tk_snprintf(today, sizeof(today), "%04d/%02d/%02d", dt.year, dt.month, dt.day);

        tk_object_set_prop_str(TK_OBJECT(s_log_message_vm), CSV_QUERY_PREFIX "keywords", "");
        tk_object_set_prop_str(TK_OBJECT(s_log_message_vm), CSV_QUERY_PREFIX "device", "");
        tk_object_set_prop_str(TK_OBJECT(s_log_message_vm), LOG_MESSAGE_PROP_QUERY_END_DATE, today);
    }

    return s_log_message_vm != NULL ? RET_OK : RET_FAIL;
}

ret_t log_message_deinit(void)
{
    TK_OBJECT_UNREF(s_log_message_vm);
    return RET_OK;
}
