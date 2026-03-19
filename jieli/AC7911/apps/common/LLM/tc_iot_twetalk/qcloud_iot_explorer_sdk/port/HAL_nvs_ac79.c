/**
 * @file HAL_nvs_esp.c
 * @author hubertxxu (hubertxxu@tencent.com)
 * @brief
 * @version 0.1
 * @date 2025-08-25
 *
 * @copyright
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright(C) 2021 - 2026 THL A29 Limited, a Tencent company.All rights reserved.
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "qcloud_iot_platform.h"

static const char *TAG = "HAL NVS";
// 固定的命名空间
#define NVS_NAMESPACE "qcloud_iot"

int HAL_NVS_Write(const char *key, const uint8_t *value, uint32_t length)
{

    return 0;
}

int HAL_NVS_Read(const char *key, uint8_t *value, uint32_t *length)
{
    return 0;
}

int HAL_NVS_Erase(const char *key)
{
    return 0;
}
