/**
 * File:   history_data.c
 * Author: AWTK Develop Team
 * Brief:  history_data
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
#include "tkc/darray.h"
#include "tkc/date_time.h"
#include "history_data.h"
#include "csv_view_model.h"

/*默认参数*/
/*默认用|分隔，因为|比较少用。*/
#define HISTORY_DATA_FEIDLS_SEPERATOR '|'
/*最大记录数*/
#define HISTORY_DATA_MAX_ROWS 1000

typedef struct _history_data_item_t {
    char *name;
    double min;
    double max;
    bool_t min_is_set;
    bool_t max_is_set;
} history_data_item_t;

static history_data_item_t *history_data_item_create(const char *name)
{
    history_data_item_t *item = TKMEM_ZALLOC(history_data_item_t);
    return_value_if_fail(item != NULL, NULL);
    item->name = tk_strdup(name);

    return item;
}

static ret_t history_data_item_destroy(history_data_item_t *item)
{
    return_value_if_fail(item != NULL, RET_BAD_PARAMS);
    TKMEM_FREE(item->name);
    TKMEM_FREE(item);

    return RET_OK;
}

typedef struct _history_data_filter_ctx_t {
    int64_t start_date_time;
    int64_t end_date_time;
    uint32_t time_index;
    darray_t fields;
} history_data_filter_ctx_t;

static view_model_t *s_history_data_vm = NULL;
static history_data_filter_ctx_t s_history_data_filter_ctx;

static ret_t history_data_filter_info_init(history_data_filter_ctx_t *info, tk_object_t *args)
{
    date_time_t dt;
    uint32_t i = 0;
    uint32_t n = 0;
    char prop[TK_NAME_LEN + 1] = {0};
    return_value_if_fail(info != NULL && args != NULL, RET_BAD_PARAMS);

    const char *str_start_date = tk_object_get_prop_str(args, HISTORY_DATA_PROP_QUERY_START_DATE);
    const char *str_start_time = tk_object_get_prop_str(args, HISTORY_DATA_PROP_QUERY_START_TIME);
    const char *str_end_date = tk_object_get_prop_str(args, HISTORY_DATA_PROP_QUERY_END_DATE);
    const char *str_end_time = tk_object_get_prop_str(args, HISTORY_DATA_PROP_QUERY_END_TIME);

    date_time_init(&dt);
    info->start_date_time = -1;
    info->end_date_time = -1;

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

    n = info->fields.size;
    for (i = 0; i < n; i++) {
        value_t v;
        history_data_item_t *item = (history_data_item_t *)darray_get(&(info->fields), i);

        if (info->time_index == i) {
            continue;
        }

        tk_snprintf(prop, sizeof(prop) - 1, CSV_QUERY_PREFIX "%s.min", item->name);
        if (tk_object_get_prop(args, prop, &v) == RET_OK) {
            item->min = value_double(&v);
            item->min_is_set = TRUE;
        }

        tk_snprintf(prop, sizeof(prop) - 1, CSV_QUERY_PREFIX "%s.max", item->name);
        if (tk_object_get_prop(args, prop, &v) == RET_OK) {
            item->max = value_double(&v);
            item->max_is_set = TRUE;
        }
    }

    return RET_OK;
}

static ret_t history_data_filter(void *ctx, tk_object_t *args, uint32_t index, csv_row_t *row)
{
    uint32_t i = 0;
    uint32_t n = 0;
    history_data_filter_ctx_t *info = (history_data_filter_ctx_t *)ctx;

    if (index == 0) {
        history_data_filter_info_init(info, args);
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

    n = info->fields.size;
    for (i = 0; i < n; i++) {
        history_data_item_t *item = (history_data_item_t *)darray_get(&(info->fields), i);
        if (info->time_index == i) {
            continue;
        }
        if (item->min_is_set) {
            const char *value = csv_row_get(row, i);
            double v = tk_atof(value);
            if (v < item->min) {
                return RET_FAIL;
            }
        }

        if (item->max_is_set) {
            const char *value = csv_row_get(row, i);
            double v = tk_atof(value);
            if (v > item->max) {
                return RET_FAIL;
            }
        }
    }

    return RET_OK;
}

static view_model_t *view_model_history_data_create(navigator_request_t *req)
{
    return csv_view_model_create_ex(req, s_history_data_vm);
}

static ret_t history_data_init_with_params(tk_object_t *model, uint32_t field_count,
        uint32_t max_rows, char sep)
{
    return_value_if_fail(model != NULL, RET_BAD_PARAMS);

    s_history_data_vm =
        csv_view_model_create(model, HISTORY_DATA_PROP_NAME, field_count, sep, max_rows,
                              history_data_filter, &s_history_data_filter_ctx);
    view_model_factory_register(HISTORY_DATA_PROP_NAME, view_model_history_data_create);

    return s_history_data_vm != NULL ? RET_OK : RET_FAIL;
}

static ret_t history_data_init_query_values(conf_node_t *node,
        history_data_filter_ctx_t *filter_ctx,
        view_model_t *vm)
{
    value_t v;
    uint32_t i = 0;
    date_time_t dt;
    char today[64] = {0};
    char prop[TK_NAME_LEN + 1] = {0};
    conf_node_t *iter = conf_node_get_first_child(node);

    date_time_init(&dt);
    tk_snprintf(today, sizeof(today), "%04d/%02d/%02d", dt.year, dt.month, dt.day);
    tk_object_set_prop_str(TK_OBJECT(vm), HISTORY_DATA_PROP_QUERY_END_DATE, today);

    while (iter != NULL) {
        history_data_item_t *item = (history_data_item_t *)darray_get(&(filter_ctx->fields), i);

        if (filter_ctx->time_index != i) {
            if (conf_node_get_child_value(iter, "min", &v) == RET_OK) {
                tk_snprintf(prop, sizeof(prop) - 1, CSV_QUERY_PREFIX "%s.min", item->name);
                tk_object_set_prop(TK_OBJECT(vm), prop, &v);
            }

            if (conf_node_get_child_value(iter, "max", &v) == RET_OK) {
                tk_snprintf(prop, sizeof(prop) - 1, CSV_QUERY_PREFIX "%s.max", item->name);
                tk_object_set_prop(TK_OBJECT(vm), prop, &v);
            }
        }

        i++;
        iter = iter->next;
    }

    return RET_OK;
}

static ret_t auto_save_history_data(const timer_info_t *timer)
{
    view_model_t *vm = s_history_data_vm;
    if (vm != NULL) {
        view_model_exec(vm, TK_OBJECT_CMD_SAVE, NULL);
        return RET_REPEAT;
    } else {
        return RET_REMOVE;
    }
}

ret_t history_data_init(tk_object_t *model, conf_doc_t *settings)
{
    ret_t ret = RET_OK;
    uint32_t field_count = 0;
    conf_node_t *node = NULL;
    conf_node_t *iter = NULL;
    uint32_t auto_save_interval = 0;
    uint32_t max_rows = HISTORY_DATA_MAX_ROWS;
    char sep = HISTORY_DATA_FEIDLS_SEPERATOR;
    history_data_filter_ctx_t *filter_ctx = &s_history_data_filter_ctx;

    return_value_if_fail(model != NULL, RET_BAD_PARAMS);
    return_value_if_fail(settings != NULL, RET_BAD_PARAMS);

    if (conf_doc_get_bool(settings, HISTORY_DATA_PROP_NAME ".enable", FALSE) == FALSE) {
        log_debug("history_data is disabled\n");
        return RET_OK;
    }
    auto_save_interval = conf_doc_get_int(settings, HISTORY_DATA_PROP_NAME ".auto_save_interval", 0);
    if (auto_save_interval > 0) {
        timer_add(auto_save_history_data, NULL, auto_save_interval);
    }

    node = conf_doc_find_node(settings, settings->root, HISTORY_DATA_PROP_NAME ".fields", FALSE);
    return_value_if_fail(node != NULL, RET_BAD_PARAMS);

    iter = conf_node_get_first_child(node);
    darray_init(&(filter_ctx->fields), 10, (tk_destroy_t)history_data_item_create, NULL);
    while (iter != NULL) {
        history_data_item_t *item = NULL;
        const char *name = conf_node_get_name(iter);

        ENSURE(TK_STR_IS_NOT_EMPTY(name));
        item = history_data_item_create(name);
        return_value_if_fail(item != NULL, RET_OOM);
        if (darray_push(&(filter_ctx->fields), item) != RET_OK) {
            history_data_item_destroy(item);
            return RET_OOM;
        }

        if (tk_str_eq(name, HISTORY_DATA_FIELD_TIME)) {
            filter_ctx->time_index = field_count;
        }

        field_count++;
        iter = iter->next;
    }

    field_count = conf_doc_get_int(settings, HISTORY_DATA_PROP_NAME ".fields_count", field_count);
    max_rows = conf_doc_get_int(settings, HISTORY_DATA_PROP_NAME ".max_rows", max_rows);
    sep = conf_doc_get_str(settings, HISTORY_DATA_PROP_NAME ".fields_seperator", "|")[0];

    log_debug("history_data enabled with(field_count=%u max_rows=%u sep=%c)\n", field_count, max_rows,
              sep);

    ret = history_data_init_with_params(model, field_count, max_rows, sep);
    if (ret == RET_OK) {
        history_data_init_query_values(node, filter_ctx, s_history_data_vm);
    }

    return ret;
}

ret_t history_data_deinit(void)
{
    darray_deinit(&(s_history_data_filter_ctx.fields));
    TK_OBJECT_UNREF(s_history_data_vm);

    return RET_OK;
}
