/**
 * File:   csv_view_model.h
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

#ifndef TK_CSV_VIEW_MODEL_H
#define TK_CSV_VIEW_MODEL_H

#include "mvvm/mvvm.h"
#include "csv/csv_file_object.h"

BEGIN_C_DECLS

/**
 * @class csv_view_model_t
 * @parent object_t
 * @annotation ["fake"]
 * 处理从 MCU 上传的 CSV 格式的数据。比如告警信息，采样数据等等。
 */

/**
 * @method csv_view_model_create
 * 创建csv_view_model对象。
 *
 * @param {tk_object_t*} default_modal 默认的modal对象。
 * @param {const char*} name csv_view_model的名称。
 * @param {uint32_t} cols 列数。
 * @param {char} separator 分隔符。
 * @param {uint32_t} max_rows 最大行数。
 * @param {csv_file_object_filter_t} filter 过滤器。
 * @param {void*} filter_ctx 过滤器上下文。
 * @return {view_model_t*} 返回view_model对象。
 */
view_model_t *csv_view_model_create(tk_object_t *default_modal, const char *name, uint32_t cols,
                                    char separator, uint32_t max_rows, csv_file_object_filter_t filter,
                                    void *filter_ctx);

/**
 * @method csv_view_model_create_ex
 * 创建csv_view_model对象。
 *
 * @param {navigator_request_t*} req 请求对象。
 * @param {view_model_t*} vm view_model对象。
 *
 * @return {view_model_t*} 返回view_model对象。
 */
view_model_t *csv_view_model_create_ex(navigator_request_t *req, view_model_t *vm);

END_C_DECLS

#endif /*TK_CSV_VIEW_MODEL_H*/
