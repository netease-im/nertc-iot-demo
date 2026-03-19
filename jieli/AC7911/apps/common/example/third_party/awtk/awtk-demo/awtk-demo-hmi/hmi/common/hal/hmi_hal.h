/**
 * File:   hmi_hal.h
 * Author: AWTK Develop Team
 * Brief:  hmi hal
 *
 * Copyright (c) 2019 - 2023  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2023-12-02 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_HMI_HAL_H
#define TK_HMI_HAL_H

#include "tkc/date_time.h"

BEGIN_C_DECLS

/**
 * @class hmi_hal_t
 * @annotation ["fake"]
 * 硬件相关接口。
 *
 */

/**
 * @method hmi_hal_init
 * 初始化硬件相关接口。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_init(void);

/**
 * @method hmi_hal_save_settings
 * 保存配置。
 * @param {const char*} app_name 应用名称。
 * @param {const void*} data 配置数据。
 * @param {uint32_t} size 配置数据大小。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_save_settings(const char *app_name, const void *data, uint32_t size);

/**
 * @method hmi_hal_load_setttings
 * 加载配置。
 * @param {const char*} app_name 应用名称。
 * @param {void**} data 配置数据。
 * @param {uint32_t*} size 配置数据大小。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_load_setttings(const char *app_name, void **data, uint32_t *size);

/**
 * @method hmi_hal_beep_on
 * 蜂鸣器响。
 * @param {uint32_t} duration 响多长时间(ms)。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_beep_on(uint32_t nms);

/**
 * @method hmi_hal_set_backlight
 * 设置背光。
 * @param {uint32_t} value 背光值(0-100)。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_set_backlight(uint32_t value);

/**
 * @method hmi_hal_get_backlight
 * 获取背光。
 * @return {uint32_t} 返回背光值(0-100)。
 */
uint32_t hmi_hal_get_backlight(void);

/**
 * @method hmi_hal_set_rtc
 * 设置RTC时间。
 * @param {date_time_t*} dt RTC时间。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_set_rtc(date_time_t *dt);

/**
 * @method hmi_hal_get_rtc
 * 获取RTC时间。
 * @param {date_time_t*} dt RTC时间。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_get_rtc(date_time_t *dt);

/**
 * @method hmi_hal_play_audio
 * 播放音频。
 * @param {const char*} file 音频文件。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_play_audio(const char *file);

/**
 * @method hmi_hal_stop_audio
 * 停止播放音频。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_stop_audio(void);

/**
 * @method hmi_hal_set_audio_volume
 * 设置音量。
 * @param {int} volume 音量值(0-100)。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_set_audio_volume(int volume);

/**
 * @method hmi_hal_get_audio_volume
 * 获取音量。
 * @return {uint32_t} 返回音量值(0-100)。
 */
uint32_t hmi_hal_get_audio_volume(void);

/**
 * @method hmi_hal_deinit
 * 反初始化硬件相关接口。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t hmi_hal_deinit(void);

END_C_DECLS

#endif /*TK_HMI_HAL_H*/
