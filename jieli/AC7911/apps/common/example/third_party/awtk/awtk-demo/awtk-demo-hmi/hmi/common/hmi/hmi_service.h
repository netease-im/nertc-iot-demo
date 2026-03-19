/**
 * File:   hmi_service.h
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

#ifndef TK_HMI_SERVICE_H
#define TK_HMI_SERVICE_H

#include "tkc/mem.h"
#include "mvvm/mvvm.h"

BEGIN_C_DECLS

/**
 * @class hmi_service_t
 * 串口屏服务。
 */

/**
 * @method hmi_service_init
 * 初始化串口屏。
 * @param {const char*} uart_device 串口设备名称。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_service_init(const char *uart_device);

/**
 * @method hmi_service_get_default_model
 * 获取默认模型对象。
 * @return {tk_object_t*} 返回默认模型对象。
 */
tk_object_t *hmi_service_get_default_model(void);

/**
 * @method hmi_service_deinit
 * 反初始化串口屏。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_service_deinit(void);

#ifndef HMI_SERVICE_DEFAULT_UART
#define HMI_SERVICE_DEFAULT_UART "uart0"
#endif/*HMI_SERVICE_DEFAULT_UART*/

END_C_DECLS

#endif /*TK_HMI_SERVICE_H*/
