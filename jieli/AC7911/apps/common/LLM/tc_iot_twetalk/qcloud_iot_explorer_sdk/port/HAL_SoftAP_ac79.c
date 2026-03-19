/**
 * @file HAL_SoftAP_config.c
 * @author {hubert} ({hubertxxu@tencent.com})
 * @brief
 * @version 1.0
 * @date 2022-09-06
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
 * 2022-09-06		1.0			hubertxxu		first commit
 * </table>
 */

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_platform.h"

static const char *TAG = "wifi softAP";

int HAL_SoftAP_Start(const char *ssid, const char *password, uint8_t ch)
{
    return QCLOUD_RET_SUCCESS;
}

int HAL_SoftAP_Stop(void)
{
    return QCLOUD_RET_SUCCESS;
}
