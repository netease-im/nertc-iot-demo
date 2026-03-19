/**
 * @file HAL_ota_esp.c
 * @author {hubert} ({hubertxxu@tencent.com})
 * @brief
 * @version 1.0
 * @date 2023-06-19
 *
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
 * @par Change Log:
 * <table>
 * Date				Version		Author			Description
 * 2023-06-19		1.0			hubertxxu		first commit
 * </table>
 */


#include "qcloud_iot_common.h"


uint32_t HAL_OTA_get_download_addr(void *usr_data)
{
    return 0;
}

int HAL_OTA_read_flash(void *usr_data, uint32_t read_addr, uint8_t *read_data, uint32_t read_len)
{
    return 0;
}

int HAL_OTA_write_flash(void *usr_data, uint32_t write_addr, uint8_t *write_data, uint32_t write_len)
{
    return 0;
}

int HAL_OTA_SwitchToNewFirmware(void)
{
    return 0;
}
