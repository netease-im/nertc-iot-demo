/**
 * File:   usb_camera_devices.c
 * Author: AWTK Develop Team
 * Brief:  USB 摄像头驱动层的基类
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-06-17 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

#include "usb_camera_devices.h"

static uint32_t s_drive_index = 0;
static uint32_t s_drive_list_size = 0;
static usb_camera_drive_info_t *s_drive_list = NULL;

ret_t usb_camera_check_ratio(void *usb_camera_device, uint32_t width,
                             uint32_t height)
{
    ret_t ret = RET_OK;
    usb_camera_ratio_t usb_camera_ratio;
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    slist_t *device_ratio_list =
        slist_create(usb_camera_device_ratio_list_destroy,
                     usb_camera_device_ratio_list_compare);

    usb_camera_get_devices_ratio_list(usb_camera_device, device_ratio_list);

    usb_camera_ratio.width = width;
    usb_camera_ratio.height = height;

    if (slist_find(device_ratio_list, &usb_camera_ratio) == NULL) {
        ret = RET_NOT_FOUND;
    }

    slist_destroy(device_ratio_list);

    return ret;
}

ret_t usb_camera_get_ratio(void *p_usb_camera_device, uint32_t *width,
                           uint32_t *height)
{
    usb_camera_device_base_t *usb_camera_device_base = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device_base = (usb_camera_device_base_t *)p_usb_camera_device;

    *width = usb_camera_device_base->device_ratio.width;
    *height = usb_camera_device_base->device_ratio.height;

    return RET_OK;
}

ret_t usb_camera_set_mirror(void *p_usb_camera_device, bool_t is_mirror)
{
    usb_camera_device_base_t *usb_camera_device_base = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device_base = (usb_camera_device_base_t *)p_usb_camera_device;

    usb_camera_device_base->is_mirror = is_mirror;

    return RET_OK;
}

ret_t usb_camera_prepare_image_fun(void *ctx, bitmap_t *image)
{
    ret_t ret = 0;
    unsigned char *image_data = NULL;

#ifdef TK_GRAPHIC_BUFFER_H
    image_data = bitmap_lock_buffer_for_write(image);
#else
    image_data = (unsigned char *)(image->data);
#endif

    ret = usb_camera_get_video_image_data(ctx, image_data,
                                          (bitmap_format_t)image->format,
                                          image->h * image->line_length);

#ifdef TK_GRAPHIC_BUFFER_H
    bitmap_unlock_buffer(image);
#endif

    return ret;
}

bitmap_t *usb_camera_prepare_image_create_image_fun(void *ctx, bitmap_format_t format,
        bitmap_t *old_image)
{
    uint32_t bpp = bitmap_get_bpp_of_format(format);
    usb_camera_device_base_t *usb_camera_device_base = (usb_camera_device_base_t *)ctx;
    return_value_if_fail(usb_camera_device_base != NULL, NULL);

    if (old_image == NULL ||
        usb_camera_device_base->device_ratio.width != old_image->w ||
        usb_camera_device_base->device_ratio.height != old_image->h) {
        if (old_image != NULL) {
            bitmap_destroy(old_image);
        }
        return bitmap_create_ex(usb_camera_device_base->device_ratio.width, usb_camera_device_base->device_ratio.height, 0, format);
    }

    return old_image;
}

ret_t usb_camera_devices_info_list_destroy(void *data)
{
    TKMEM_FREE(data);
    return RET_OK;
}

int usb_camera_devices_info_list_compare(const void *a, const void *b)
{
    usb_camera_device_info_t *device_info_a = (usb_camera_device_info_t *)a;
    usb_camera_device_info_t *device_info_b = (usb_camera_device_info_t *)b;

    if (device_info_a->camera_id == device_info_b->camera_id) {
        return 0;
    }

    return 1;
}

ret_t usb_camera_device_ratio_list_destroy(void *data)
{
    TKMEM_FREE(data);
    return RET_OK;
}

int usb_camera_device_ratio_list_compare(const void *a, const void *b)
{
    usb_camera_ratio_t *ratio_a = (usb_camera_ratio_t *)a;
    usb_camera_ratio_t *ratio_b = (usb_camera_ratio_t *)b;

    if (ratio_a->width == ratio_b->width && ratio_a->height == ratio_b->height) {
        return 0;
    }
    return 1;
}

const usb_camera_drive_info_t *usb_camera_device_get_drive_list(uint32_t *ret_len)
{
    return_value_if_fail(ret_len != NULL, NULL);
#if WIN32
    extern const usb_camera_drive_info_t msmd_usb_camera_drive_info;
    extern const usb_camera_drive_info_t dshow_usb_camera_drive_info;
    static usb_camera_drive_info_t g_drive_list[2] = {0};
    g_drive_list[0] = msmd_usb_camera_drive_info;
    g_drive_list[1] = dshow_usb_camera_drive_info;
#elif LINUX
    extern const usb_camera_drive_info_t v4l2_usb_camera_drive_info;
    static usb_camera_drive_info_t g_drive_list[1] = {0};
    g_drive_list[0] = v4l2_usb_camera_drive_info;
#endif

    s_drive_list = g_drive_list;
    *ret_len = ARRAY_SIZE(g_drive_list);
    s_drive_list_size = ARRAY_SIZE(g_drive_list);

    return s_drive_list;
}

ret_t usb_camera_device_set_global_drive_index(uint32_t index)
{
    return_value_if_fail(index < s_drive_list_size, RET_BAD_PARAMS);
    s_drive_index = index;
    log_debug("use %s \r\n", s_drive_list[s_drive_index].name);
    return RET_OK;
}

ret_t usb_camera_get_all_devices_info(slist_t *devices_list)
{
    if (s_drive_list == NULL) {
        uint32_t ret_len = 0;
        usb_camera_device_get_drive_list(&ret_len);
    }
    return s_drive_list[s_drive_index].vt->get_all_devices_info(devices_list);
}

ret_t usb_camera_get_devices_ratio_list_for_device_id(uint32_t device_id, slist_t *ratio_list)
{
    if (s_drive_list == NULL) {
        uint32_t ret_len = 0;
        usb_camera_device_get_drive_list(&ret_len);
    }
    return s_drive_list[s_drive_index].vt->get_devices_ratio_list_for_device_id(device_id, ratio_list);
}

ret_t usb_camera_get_devices_ratio_list(void *p_usb_camera_device, slist_t *ratio_list)
{
    if (s_drive_list == NULL) {
        uint32_t ret_len = 0;
        usb_camera_device_get_drive_list(&ret_len);
    }
    return s_drive_list[s_drive_index].vt->get_devices_ratio_list(p_usb_camera_device, ratio_list);
}

void *usb_camera_open_device_with_width_and_height(uint32_t device_id, bool_t is_mirror, uint32_t width, uint32_t height)
{
    if (s_drive_list == NULL) {
        uint32_t ret_len = 0;
        usb_camera_device_get_drive_list(&ret_len);
    }
    return s_drive_list[s_drive_index].vt->open_device_with_width_and_height(device_id, is_mirror, width, height);
}

void *usb_camera_open_device(uint32_t device_id, bool_t is_mirror)
{
    if (s_drive_list == NULL) {
        uint32_t ret_len = 0;
        usb_camera_device_get_drive_list(&ret_len);
    }
    return s_drive_list[s_drive_index].vt->open_device(device_id, is_mirror);
}

void *usb_camera_set_ratio(void *p_usb_camera_device, uint32_t width, uint32_t height)
{
    return s_drive_list[s_drive_index].vt->set_ratio(p_usb_camera_device, width, height);
}

ret_t usb_camera_close_device(void *p_usb_camera_device)
{
    return s_drive_list[s_drive_index].vt->close_device(p_usb_camera_device);
}

ret_t usb_camera_get_video_image_data(void *p_usb_camera_device, unsigned char *data, bitmap_format_t format, uint32_t data_len)
{
    return s_drive_list[s_drive_index].vt->get_video_image_data(p_usb_camera_device, data, format, data_len);
}
