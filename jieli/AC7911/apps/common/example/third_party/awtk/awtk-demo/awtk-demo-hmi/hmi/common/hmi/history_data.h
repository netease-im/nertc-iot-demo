/**
 * File:   history_data.h
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

#ifndef TK_HISTORY_DATA_H
#define TK_HISTORY_DATA_H

#include "tkc/object.h"
#include "conf_io/conf_node.h"

BEGIN_C_DECLS

/**
 * @class history_data_t
 * @annotation ["fake"]
 * 处理 MCU 端上报的采样数据。
 * 创建一个 history_data 的 view_model，将采样数据保存到csv文件中。
 *
 */

/**
 * @method history_data_init
 * 初始化history_data。
 *
 * @param {tk_object_t*} model 默认模型对象。
 * @param {conf_doc_t*} settings 配置信息。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t history_data_init(tk_object_t *model, conf_doc_t *settings);

/**
 * @method history_data_deinit
 * 销毁history_data。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t history_data_deinit(void);

/*MCU端设置此属性，视为上报采样数据。*/
#define HISTORY_DATA_PROP_NAME "history_data"

#define HISTORY_DATA_PROP_QUERY_START_DATE CSV_QUERY_PREFIX"start_date"
#define HISTORY_DATA_PROP_QUERY_START_TIME CSV_QUERY_PREFIX"start_time"
#define HISTORY_DATA_PROP_QUERY_END_DATE CSV_QUERY_PREFIX"end_date"
#define HISTORY_DATA_PROP_QUERY_END_TIME CSV_QUERY_PREFIX"end_time"

#define HISTORY_DATA_FIELD_TIME "time"

END_C_DECLS

#endif /*TK_HISTORY_DATA_H*/
