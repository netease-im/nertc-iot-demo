/**
 * @file HAL_wifi.c
 * @author {hubert} ({hubertxxu@tencent.com})
 * @brief
 * @version 1.0
 * @date 2022-10-31
 *
 * @copyright
 *
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright(C) 2018 - 2021 THL A29 Limited, a Tencent company.All rights reserved.
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
 * 2022-10-31		1.0			hubertxxu		first commit
 * </table>
 */

#include "qcloud_iot_platform.h"

static const char *TAG = "HAL wifi";

#define WIFI_CONNECTED_BIT        BIT0
#define WIFI_FAIL_BIT             BIT1
#define EXAMPLE_ESP_MAXIMUM_RETRY 20

#define WIFI_NVS_NAMESPACE "wifi_info"

static int sg_retry_num         = 0;
static uint32_t local_ipv4_addr = 0xC0A80401;  // 192.168.4.1


int HAL_Wifi_Init(void)
{
    return 0;
}

int HAL_Wifi_ModeSet(TCIoTWifiMode mode)
{
    return 0;
}

int HAL_Wifi_StaInfoSet(const char *ssid, uint8_t ssid_len, const char *passwd, uint8_t passwd_len)
{
    return 0;
}

int HAL_Wifi_StaConnect(uint32_t timeout_ms)
{
    return 0;
}

int HAL_Wifi_LogGet(void)
{
    // nothing todo
    return 0;
}

uint32_t HAL_Wifi_Ipv4Get(void)
{
    return local_ipv4_addr;
}

size_t HAL_Wifi_MacGet(uint8_t *mac)
{
    return 6;
}

int HAL_Wifi_StartStaConnect(const char *ssid, const char *passwd, uint32_t timeout_ms)
{
    return 0;
}
