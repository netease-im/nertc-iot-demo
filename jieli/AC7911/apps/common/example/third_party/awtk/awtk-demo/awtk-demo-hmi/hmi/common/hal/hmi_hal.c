/**
 * File:   hmi_hal.c
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

#include "hal/hmi_hal.h"


#include "tkc/fs.h"
#include "tkc/path.h"
#include "tkc/event.h"
#include "tkc/utils.h"

ret_t hmi_hal_init(void)
{
    /*rtc_cfg_init();*/

    return RET_OK;
}

static const char *hmi_hal_get_settings_filename(const char *app_name,
        char filename[MAX_PATH + 1])
{
    char fullpath[MAX_PATH + 1] = {0};
    return_value_if_fail(filename != NULL, NULL);
    path_prepend_user_storage_path(fullpath, app_name);

    if (!path_exist(fullpath)) {
        fs_create_dir(os_fs(), fullpath);
    }

    tk_snprintf(filename, MAX_PATH, "%s/%s", fullpath, "settings.json");

    return filename;
}

ret_t hmi_hal_save_settings(const char *app_name, const void *data, uint32_t size)
{
    char filename[MAX_PATH + 1] = {0};
    return_value_if_fail(data != NULL && size > 0, RET_BAD_PARAMS);
    return_value_if_fail(hmi_hal_get_settings_filename(app_name, filename) != NULL, RET_BAD_PARAMS);

    return file_write(filename, data, size);
}

ret_t hmi_hal_load_setttings(const char *app_name, void **data, uint32_t *size)
{
    char filename[MAX_PATH + 1] = {0};
    return_value_if_fail(data != NULL && size != NULL, RET_BAD_PARAMS);
    return_value_if_fail(hmi_hal_get_settings_filename(app_name, filename) != NULL, RET_BAD_PARAMS);

    *data = file_read(filename, size);

    return *data != NULL ? RET_OK : RET_IO;
}


ret_t hmi_hal_beep_on(uint32_t nms)
{
    /*beep_on_ms(nms);*/
    log_debug("beep on: %u\n", nms);
    return RET_OK;
}

static uint32_t s_backlight = 60;

ret_t hmi_hal_set_backlight(uint32_t value)
{
    value = tk_clampi(value, 0, 100);
    s_backlight = value;
    /*adjust_backlight(value);*/
    log_debug("set backlight: %u\n", s_backlight);
    return RET_OK;
}

uint32_t hmi_hal_get_backlight(void)
{
    return s_backlight;
}

ret_t hmi_hal_set_rtc(date_time_t *dt)
{
    /*systime_t st;*/
    return_value_if_fail(dt != NULL, RET_BAD_PARAMS);

    /*st.year = dt->year;*/
    /*st.month = dt->month;*/
    /*st.day = dt->day;*/
    /*st.hour = dt->hour;*/
    /*st.minute = dt->minute;*/
    /*st.second = dt->second;*/

    /*rtc_set_time(st);*/
    log_debug("set rtc: %d-%d-%d %d:%d:%d\n", dt->year, dt->month, dt->day, dt->hour, dt->minute,
              dt->second);

    return RET_OK;
}

ret_t hmi_hal_get_rtc(date_time_t *dt)
{
    /*systime_t st;*/
    return_value_if_fail(dt != NULL, RET_BAD_PARAMS);

    memset(dt, 0x00, sizeof(date_time_t));

    return RET_OK;
    /*if (rtc_get_time(&st) == 0) {*/
    /*dt->year = st.year;*/
    /*dt->month = st.month;*/
    /*dt->day = st.day;*/
    /*dt->hour = st.hour;*/
    /*dt->minute = st.minute;*/
    /*dt->second = st.second;*/
    /*dt->wday = st.week;*/
    /*return RET_OK;*/
    /*} else {*/
    /*return RET_FAIL;*/
    /*}*/
}

ret_t hmi_hal_play_audio(const char *file)
{
    return_value_if_fail(file != NULL, RET_BAD_PARAMS);
    log_debug("play audio: %s\n", file);
    /*audio_play_file((char *)file);*/
    return RET_OK;
}

ret_t hmi_hal_stop_audio(void)
{
    log_debug("stop audio\n");
    /*audio_close();*/
    return RET_OK;
}

static uint32_t s_audio_volume = 60;
ret_t hmi_hal_set_audio_volume(int volume)
{
    s_audio_volume = tk_clampi(volume, 0, 100);
    log_debug("set audio volume: %u\n", s_audio_volume);
    return RET_OK;
}

uint32_t hmi_hal_get_audio_volume(void)
{
    return s_audio_volume;
}

ret_t hmi_hal_deinit(void)
{
    return RET_OK;
}


