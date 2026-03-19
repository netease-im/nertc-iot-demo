/**
 * File:   usb_camera_linux_device_v4l.cpp
 * Author: AWTK Develop Team
 * Brief:  Linux 的 v4l 协议的 USB 摄像头驱动代码
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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "color_format_conversion.inc"

#include <linux/videodev.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define CHANNEL_NUMBER 1
#define MAX_CAMERAS 8

#define MAX_DEVICE_DRIVER_NAME 80

typedef struct _usb_camera_device_t {
    usb_camera_device_base_t base;

    int device_handle;

    unsigned long palette;

    char *memoryMap;
    video_mmap *mmaps;
    video_mbuf memoryBuffer;
    video_window captureWindow;
    video_capability capability;
    video_picture image_properties;

} usb_camera_device_t;

static ret_t usb_camera_get_all_devices_info(slist_t *devices_list)
{
    int device_handle = 0;
    int camera_number = 0;
    char device_name[MAX_DEVICE_DRIVER_NAME] = {0};

    while (camera_number < MAX_CAMERAS) {
        sprintf(device_name, "/dev/video%d", camera_number);
        device_handle = open(device_name, O_RDONLY);
        if (device_handle != -1) {
            usb_camera_device_info_t *device_info =
                TKMEM_ZALLOC(usb_camera_device_info_t);
            device_info->camera_id = camera_number;
            memcpy(device_info->camera_name, device_name,
                   MAX_DEVICE_DRIVER_NAME < USB_CAMERA_NAME_MAX_LENGTH
                   ? MAX_DEVICE_DRIVER_NAME
                   : USB_CAMERA_NAME_MAX_LENGTH);
            slist_append(devices_list, device_info);
            close(device_handle);
        }
        camera_number++;
    }
    return RET_OK;
}

static ret_t usb_camera_get_devices_ratio_list(void *p_usb_camera_device,
        slist_t *ratio_list)
{
    return RET_OK;
}

static ret_t usb_camera_try_init_v4l(usb_camera_device_t *usb_camera_device)
{
    char device_name[MAX_DEVICE_DRIVER_NAME] = {0};
    sprintf(device_name, "/dev/video%d",
            usb_camera_device->base.device_info.camera_id);
    usb_camera_device->device_handle = open(device_name, O_RDWR);
    if (usb_camera_device->device_handle > 0) {
        memset(&usb_camera_device->capability, 0, sizeof(video_capability));
        if (ioctl(usb_camera_device->device_handle, VIDIOCGCAP,
                  &usb_camera_device->capability) >= 0) {
            return RET_OK;
        } else {
            printf("ioctl(usb_camera_device->device_handle, VIDIOCGCAP, "
                   "&usb_camera_device->capability) >= 0 \r\n");
        }

    } else {
        printf("usb_camera_device->device_handle < 0 \r\n");
    }

    usb_camera_close_device(usb_camera_device);
    return RET_FAIL;
}

static ret_t usb_camera_try_palette_v4l(usb_camera_device_t *usb_camera_device,
                                        int pal, int depth)
{
    usb_camera_device->image_properties.palette = pal;
    usb_camera_device->image_properties.depth = depth;
    if (ioctl(usb_camera_device->device_handle, VIDIOCSPICT,
              &usb_camera_device->image_properties) < 0) {
        return RET_OK;
    }
    if (ioctl(usb_camera_device->device_handle, VIDIOCGPICT,
              &usb_camera_device->image_properties) < 0) {
        return RET_OK;
    }
    if (usb_camera_device->image_properties.palette == pal) {
        return RET_FAIL;
    }
    return RET_OK;
}

static ret_t
usb_camera_auto_setup_capture_mode_v4l(usb_camera_device_t *usb_camera_device)
{
    if (ioctl(usb_camera_device->device_handle, VIDIOCGPICT,
              &usb_camera_device->image_properties) < 0) {
        printf(
            "HIGHGUI ERROR: V4L: Unable to determine size of incoming image \r\n");
        usb_camera_close_device(usb_camera_device);
        return RET_FAIL;
    }

    if (usb_camera_try_palette_v4l(usb_camera_device, VIDEO_PALETTE_RGB24, 24) ==
        RET_OK) {
        usb_camera_device->palette = VIDEO_PALETTE_RGB24;
        printf("negotiated palette RGB24\r\n");
    } else if (usb_camera_try_palette_v4l(usb_camera_device,
                                          VIDEO_PALETTE_YUV420P, 16) == RET_OK) {
        usb_camera_device->palette = VIDEO_PALETTE_YUV420P;
        printf("negotiated palette YUV420P\r\n");
    } else if (usb_camera_try_palette_v4l(usb_camera_device, VIDEO_PALETTE_YUV420,
                                          16) == RET_OK) {
        usb_camera_device->palette = VIDEO_PALETTE_YUV420;
        printf("negotiated palette YUV420\r\n");
    } else if (usb_camera_try_palette_v4l(usb_camera_device,
                                          VIDEO_PALETTE_YUV411P, 16) == RET_OK) {
        usb_camera_device->palette = VIDEO_PALETTE_YUV411P;
        printf("negotiated palette YUV420P\r\n");
    } else if (usb_camera_try_palette_v4l(usb_camera_device, VIDEO_PALETTE_YUYV,
                                          16) == RET_OK) {
        usb_camera_device->palette = VIDEO_PALETTE_YUYV;
        printf("negotiated palette YUYV\r\n");
    } else {
        printf("HIGHGUI ERROR: V4L: Pixel format of incoming image is unsupported "
               "by OpenCV \r\n");
        usb_camera_close_device(usb_camera_device);
        return RET_FAIL;
    }

    return RET_OK;
}

static usb_camera_device_t *usb_camera_device_init(uint32_t device_id,
        uint32_t width,
        uint32_t height,
        bool_t is_mirror)
{
    usb_camera_device_t *usb_camera_device = TKMEM_ZALLOC(usb_camera_device_t);

    usb_camera_device->base.device_info.camera_id = device_id;
    usb_camera_device->base.is_mirror = is_mirror;

    usb_camera_device->base.device_ratio.width = width;
    usb_camera_device->base.device_ratio.height = height;
    return usb_camera_device;
}

static ret_t
usb_camera_selected_channel(usb_camera_device_t *usb_camera_device)
{
    if (usb_camera_device->capability.channels > 0) {
        struct video_channel selectedChannel;
        memset(&selectedChannel, 0, sizeof(selectedChannel));

        selectedChannel.channel = CHANNEL_NUMBER;
        if (ioctl(usb_camera_device->device_handle, VIDIOCGCHAN,
                  &selectedChannel) != -1) {
            if (ioctl(usb_camera_device->device_handle, VIDIOCSCHAN,
                      &selectedChannel) == -1) {
                printf(" %d, %s not NTSC capable.\r\n", selectedChannel.channel,
                       selectedChannel.name);
                return RET_FAIL;
            }
        }
    }

    return RET_OK;
}

static ret_t usb_camera_buffer_init(usb_camera_device_t *usb_camera_device)
{
    ioctl(usb_camera_device->device_handle, VIDIOCGMBUF,
          &usb_camera_device->memoryBuffer);
    usb_camera_device->memoryMap = (char *)mmap(
                                       0, usb_camera_device->memoryBuffer.size, PROT_READ | PROT_WRITE,
                                       MAP_SHARED, usb_camera_device->device_handle, 0);
    if (usb_camera_device->memoryMap == MAP_FAILED) {
        printf(
            "HIGHGUI ERROR: V4L: Mapping Memmory from video source error: %s\r\n",
            strerror(errno));
        usb_camera_close_device(usb_camera_device);
        return RET_FAIL;
    }
    printf("frames:%d \r\n", usb_camera_device->memoryBuffer.frames);
    usb_camera_device->mmaps = (struct video_mmap *)(TKMEM_ALLOC(
                                   usb_camera_device->memoryBuffer.frames * sizeof(struct video_mmap)));
    if (!usb_camera_device->mmaps) {
        printf("HIGHGUI ERROR: V4L: Could not memory map video frames.\r\n");
        usb_camera_close_device(usb_camera_device);
        return RET_FAIL;
    }
    return RET_OK;
}

void *usb_camera_start_device(uint32_t device_id, uint32_t width,
                              uint32_t height, bool_t is_mirror)
{
    usb_camera_device_t *usb_camera_device =
        usb_camera_device_init(device_id, width, height, is_mirror);

    if (usb_camera_try_init_v4l(usb_camera_device) != RET_OK) {
        if ((usb_camera_device->capability.type & VID_TYPE_CAPTURE) == 0) {
            printf("HIGHGUI ERROR: V4L: device /dev/video%d is unable to capture "
                   "video memory.\r\n",
                   device_id);
            usb_camera_close_device(usb_camera_device);
            return NULL;
        }
    }

    usb_camera_selected_channel(usb_camera_device);

    if (ioctl(usb_camera_device->device_handle, VIDIOCGWIN,
              &usb_camera_device->captureWindow) == -1) {
        printf("HIGHGUI ERROR: V4L: Could not obtain specifics of capture "
               "window.\r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_auto_setup_capture_mode_v4l(usb_camera_device) != RET_OK) {
        printf("HIGHGUI ERROR: V4L: Could not obtain specifics of capture "
               "window.\r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_buffer_init(usb_camera_device) != RET_OK) {
        return NULL;
    }

    return usb_camera_device;
}

static void *usb_camera_open_device(uint32_t device_id, bool_t is_mirror)
{
    return usb_camera_start_device(device_id, USB_CAMERA_DEFAULT_WIDTH,
                                   USB_CAMERA_DEFAULT_HEIGHT, is_mirror);
}

static void *usb_camera_open_device_with_width_and_height(uint32_t device_id,
        bool_t is_mirror,
        uint32_t width,
        uint32_t height)
{
    return usb_camera_start_device(device_id, width, height, is_mirror);
}

static void *usb_camera_set_ratio(void *p_usb_camera_device, uint32_t width,
                                  uint32_t height)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, NULL);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    usb_camera_device->base.device_ratio.width = width;
    usb_camera_device->base.device_ratio.height = height;

    return NULL;
}

static ret_t usb_camera_close_device(void *p_usb_camera_device)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    if (usb_camera_device->mmaps != NULL) {
        free(usb_camera_device->mmaps);
        usb_camera_device->mmaps = NULL;
    }
    if (usb_camera_device->memoryMap != NULL) {
        munmap(usb_camera_device->memoryMap, usb_camera_device->memoryBuffer.size);
        usb_camera_device->memoryMap = NULL;
    }

    if (usb_camera_device->device_handle != -1) {
        close(usb_camera_device->device_handle);
    }
    TKMEM_FREE(p_usb_camera_device);
    return RET_OK;
}

static ret_t color_format_conver(unsigned char *src, unsigned long palette,
                                 unsigned int width, unsigned int height,
                                 unsigned int bpp, unsigned char *dst,
                                 bitmap_format_t format, unsigned int dst_len,
                                 unsigned char is_mirror,
                                 unsigned char is_reverse)
{
    uint32_t error = -1;
    if (palette == VIDEO_PALETTE_RGB24 && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_RGB2BGRA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == VIDEO_PALETTE_RGB24 && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_RGB2RGBA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == VIDEO_PALETTE_RGB24 && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_RGB2BGR565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == VIDEO_PALETTE_RGB24 && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_RGB2RGB565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == VIDEO_PALETTE_YUYV && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2BGRA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == VIDEO_PALETTE_YUYV && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2RGBA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    }

    return error == 0 ? RET_OK : RET_FAIL;
}

static ret_t usb_camera_get_video_image_data(void *p_usb_camera_device,
        unsigned char *data,
        bitmap_format_t format,
        uint32_t data_len)
{
    // ret_t ret = RET_FAIL;
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    // ret = usb_camera_try_init_v4l(usb_camera_device);

    // ret = usb_camera_read_image_data_v4l2(usb_camera_device);
    // if(ret == RET_OK && usb_camera_device->image_data_buffers.length ==
    // usb_camera_device->base.device_ratio.width *
    // usb_camera_device->base.device_ratio.height *
    // usb_camera_device->base.device_ratio.bpp)
    // {
    //     return color_format_conver((unsigned
    //     char*)usb_camera_device->image_data_buffers.start,
    //     usb_camera_device->palette,
    //                     usb_camera_device->base.device_ratio.width,
    //                     usb_camera_device->base.device_ratio.height,
    //                     usb_camera_device->base.device_ratio.bpp,
    //                     data, format, data_len,
    //                     usb_camera_device->base.is_mirror, TRUE);
    // }
    // else
    // {
    return RET_FAIL;
    // }
}

const usb_camera_device_base_vt_t v4l_vt = {
    usb_camera_get_all_devices_info,
    NULL,
    usb_camera_get_devices_ratio_list,
    usb_camera_open_device_with_width_and_height,
    usb_camera_open_device,
    usb_camera_set_ratio,
    usb_camera_close_device,
    usb_camera_get_video_image_data,
};

extern "C" const usb_camera_drive_info_t v4l_usb_camera_drive_info = {
    "V4L",
    &v4l_vt,
};

