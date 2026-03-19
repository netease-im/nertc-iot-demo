/**
 * File:   log_message.h
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

#ifndef TK_LOG_MESSAGE_H
#define TK_LOG_MESSAGE_H

#include "tkc/object.h"
#include "conf_io/conf_node.h"

BEGIN_C_DECLS

/**
 * @class log_message_t
 * @annotation ["fake"]
 * 处理 MCU 端上报的告警信息。
 * 创建一个 log_message 的 view_model，将告警信息保存到csv文件中。
 *
 */

/**
 * @method log_message_init
 * 初始化log_message。
 *
 * @param {tk_object_t*} model 默认模型对象。
 * @param {conf_doc_t*} settings 配置信息。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t log_message_init(tk_object_t *model, conf_doc_t *settings);

/**
 * @method log_message_deinit
 * 销毁log_message。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t log_message_deinit(void);


/*MCU端设置此属性，视为上报告警信息。*/
#define LOG_MESSAGE_PROP_NAME "log_message"

#define LOG_MESSAGE_PROP_QUERY_LEVEL CSV_QUERY_PREFIX"level"
#define LOG_MESSAGE_PROP_QUERY_START_DATE CSV_QUERY_PREFIX"start_date"
#define LOG_MESSAGE_PROP_QUERY_START_TIME CSV_QUERY_PREFIX"start_time"
#define LOG_MESSAGE_PROP_QUERY_END_DATE CSV_QUERY_PREFIX"end_date"
#define LOG_MESSAGE_PROP_QUERY_END_TIME CSV_QUERY_PREFIX"end_time"
#define LOG_MESSAGE_PROP_QUERY_KEYWORDS CSV_QUERY_PREFIX"keywords"
#define LOG_MESSAGE_PROP_QUERY_DEVICE CSV_QUERY_PREFIX"device"

#define LOG_MESSAGE_FIELD_LEVEL "level"
#define LOG_MESSAGE_FIELD_TIME "time"
#define LOG_MESSAGE_FIELD_DEVICE "device"
#define LOG_MESSAGE_FIELD_MESSAGE "message"

END_C_DECLS

#endif /*TK_LOG_MESSAGE_H*/
