/**
 * @copyright
 *
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright(C) 2018 - 2022 THL A29 Limited, a Tencent company.All rights reserved.
 *
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file qcloud_iot_link_ble_esp32s3.c
 * @brief
 * @author fancyxu (fancyxu@tencent.com)
 * @version 1.0
 * @date 2022-12-29
 *
 * @par Change Log:
 * <table>
 * <tr><th>Date       <th>Version <th>Author    <th>Description
 * <tr><td>2022-12-29 <td>1.0     <td>fancyxu   <td>first commit
 * </table>
 */
#include "qcloud_iot_link.h"
#include "qcloud_iot_llsync_config.h"

#define LLSYNC_LOG_TAG "LLSYNC"


// -------------------------------------------------------------
// qcloud iot link ble
// -------------------------------------------------------------

void *qcloud_iot_link_ble_init(IotLinkCallback callback, void *usr_data)
{
    return NULL;
}

void qcloud_iot_link_ble_deinit(void *handle)
{
    return;
}

int qcloud_iot_link_ble_send(void *handle, void *addr, size_t addr_len, uint8_t *data, size_t data_len,
                             uint32_t timeout_ms)
{
    return 0;
}

int qcloud_iot_link_ble_get_uuid(void *handle, void *addr, size_t addr_len, uint8_t *uuid, size_t uuid_len)
{
    return 0;
}

int qcloud_iot_link_ble_start_advertising(void *handle, void *adv_data, size_t adv_data_len, uint32_t adv_time)
{
    return 0;
}

int qcloud_iot_link_ble_stop_advertising(void *handle)
{
    return 0;
}

int qcloud_iot_link_ble_get_mtu(void *handle)
{
    return 0;
}
