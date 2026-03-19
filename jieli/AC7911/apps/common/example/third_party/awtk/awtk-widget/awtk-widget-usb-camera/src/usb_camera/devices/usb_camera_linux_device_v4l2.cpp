/**
 * File:   usb_camera_linux_device_v4l.cpp
 * Author: AWTK Develop Team
 * Brief:  Linux 的 v4l2 协议的 USB 摄像头驱动代码
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "color_format_conversion.inc"

#include <asm/types.h>
#include <linux/videodev2.h>

#ifndef V4L2_PIX_FMT_SBGGR8
#define V4L2_PIX_FMT_SBGGR8                                                    \
  v4l2_fourcc('B', 'A', '8', '1') /* 8 BGBG.. GRGR.. */
#endif

#ifndef V4L2_PIX_FMT_SN9C10X
#define V4L2_PIX_FMT_SN9C10X                                                   \
  v4l2_fourcc('S', '9', '1', '0') /* SN9C10x cmpr.  \                          \
                                     */
#endif

#ifndef V4L2_PIX_FMT_SGBRG
#define V4L2_PIX_FMT_SGBRG                                                     \
  v4l2_fourcc('G', 'B', 'R', 'G') /* bayer GBRG   GBGB.. RGRG.. */
#endif

#define MAX_V4L_BUFFERS 10
#define DEFAULT_V4L_BUFFERS 2

typedef struct _usb_camera_v4l2_buffer_t {
    void *start;
    uint32_t length;
} usb_camera_v4l2_buffer_t;

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

#define CHANNEL_NUMBER 1
#define MAX_CAMERAS 30

#define MAX_DEVICE_DRIVER_NAME 80

typedef struct _usb_camera_device_t {
    usb_camera_device_base_t base;

    int device_handle;

    unsigned long palette;
    uint32_t buffer_index;

    v4l2_input inp;
    v4l2_format form;
    timeval timestamp;
    v4l2_buf_type type;
    v4l2_capability cap;
    v4l2_requestbuffers req;
    usb_camera_v4l2_buffer_t image_data_buffers;
    usb_camera_v4l2_buffer_t buffers[MAX_V4L_BUFFERS];

} usb_camera_device_t;

static ret_t usb_camera_v4l2_close_device(void *p_usb_camera_device);
static ret_t usb_camera_try_init_v4l2(usb_camera_device_t *usb_camera_device);
static ret_t
usb_camera_auto_setup_mode_v4l2(usb_camera_device_t *usb_camera_device);
static usb_camera_device_t *usb_camera_device_init(uint32_t device_id,
        uint32_t width,
        uint32_t height,
        bool_t is_mirror);

static bool_t usb_camera_try_ioctl(usb_camera_device_t *usb_camera_device, unsigned long ioctlCode, void *parameter)
{
    int deviceHandle = usb_camera_device->device_handle;
    while (-1 == ioctl(deviceHandle, ioctlCode, parameter)) {
        if (!(errno == EBUSY || errno == EAGAIN)) {
            return FALSE;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(deviceHandle, &fds);

        /* Timeout. */
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int result = select(deviceHandle + 1, &fds, NULL, NULL, &tv);
        if (0 == result) {
            printf("select timeout\n");
            return FALSE;
        }
        if (-1 == result && EINTR != errno) {
            perror("select");
        }
    }
    return TRUE;
}

static ret_t usb_camera_v4l2_get_all_devices_info(slist_t *devices_list)
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

static ret_t usb_camera_v4l2_get_devices_ratio_list_for_device_id(uint32_t device_id,
        slist_t *ratio_list)
{
    int i = 0;
    ret_t ret = RET_OK;
    v4l2_frmsizeenum frmsize;
    usb_camera_device_t *usb_camera_device =
        usb_camera_device_init(device_id, 320, 240, FALSE);
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    ret = usb_camera_try_init_v4l2(usb_camera_device);

    if (ret == RET_OK) {
        ret = usb_camera_auto_setup_mode_v4l2(usb_camera_device);
        if (ret == RET_OK) {
            frmsize.pixel_format = usb_camera_device->palette;
            for (i = 0;; i++) {
                frmsize.index = i;
                if (ioctl(usb_camera_device->device_handle, VIDIOC_ENUM_FRAMESIZES,
                          &frmsize) == -1) {
                    break;
                }
                usb_camera_ratio_t *usb_camera_ratio = TKMEM_ZALLOC(usb_camera_ratio_t);
                usb_camera_ratio->id = i;
                usb_camera_ratio->width = frmsize.discrete.width;
                usb_camera_ratio->height = frmsize.discrete.height;
                usb_camera_ratio->bpp = usb_camera_device->base.device_ratio.bpp;
                slist_append(ratio_list, usb_camera_ratio);
            }
            usb_camera_v4l2_close_device(usb_camera_device);
            return RET_OK;
        }
    }
    usb_camera_v4l2_close_device(usb_camera_device);
    return RET_FAIL;
}

static ret_t usb_camera_v4l2_get_devices_ratio_list(void *p_usb_camera_device,
        slist_t *ratio_list)
{
    int i = 0;
    v4l2_frmsizeenum frmsize;
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    frmsize.pixel_format = usb_camera_device->palette;

    for (i = 0;; i++) {
        frmsize.index = i;
        if (ioctl(usb_camera_device->device_handle, VIDIOC_ENUM_FRAMESIZES,
                  &frmsize) == -1) {
            break;
        }
        usb_camera_ratio_t *usb_camera_ratio = TKMEM_ZALLOC(usb_camera_ratio_t);
        usb_camera_ratio->id = i;
        usb_camera_ratio->width = frmsize.discrete.width;
        usb_camera_ratio->height = frmsize.discrete.height;
        usb_camera_ratio->bpp = usb_camera_device->base.device_ratio.bpp;
        slist_append(ratio_list, usb_camera_ratio);
    }
    return RET_OK;
}

static ret_t usb_camera_try_init_v4l2(usb_camera_device_t *usb_camera_device)
{
    int device_index = 0;
    usb_camera_device_info_t *d_info = NULL;
    char device_name[MAX_DEVICE_DRIVER_NAME] = {0};
    slist_t *devices_list = slist_create(usb_camera_devices_info_list_destroy,
                                         usb_camera_devices_info_list_compare);

    usb_camera_v4l2_get_all_devices_info(devices_list);

    if (slist_size(devices_list) == 0) {
        slist_destroy(devices_list);
        return RET_FAIL;
    }

    d_info = (usb_camera_device_info_t *)slist_find(
                 devices_list, &(usb_camera_device->base.device_info));

    if (d_info == NULL) {
        d_info = (usb_camera_device_info_t *)devices_list->first;
    }

    memcpy(usb_camera_device->base.device_info.camera_name, d_info->camera_name,
           USB_CAMERA_NAME_MAX_LENGTH);
    usb_camera_device->base.device_info.camera_id = d_info->camera_id;

    sprintf(device_name, "/dev/video%d",
            usb_camera_device->base.device_info.camera_id);
    usb_camera_device->device_handle = open(device_name, O_RDWR | O_NONBLOCK, 0);
    if (-1 == usb_camera_device->device_handle) {
        slist_destroy(devices_list);
        printf("try_init_v4l2 open \"%s\": %s\n", device_name, strerror(errno));
        return RET_FAIL;
    }
    slist_destroy(devices_list);

    usb_camera_device->cap = v4l2_capability();
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_QUERYCAP,
                    &usb_camera_device->cap)) {
        printf("(DEBUG) try_init_v4l2 VIDIOC_QUERYCAP \"%s\": %s\n", device_name,
               strerror(errno));
        return RET_FAIL;
    }

    if (-1 ==
        ioctl(usb_camera_device->device_handle, VIDIOC_G_INPUT, &device_index)) {
        printf("(DEBUG) try_init_v4l2 VIDIOC_G_INPUT \"%s\": %s\n", device_name,
               strerror(errno));
        return RET_FAIL;
    }

    usb_camera_device->inp = v4l2_input();
    usb_camera_device->inp.index = device_index;
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_ENUMINPUT,
                    &usb_camera_device->inp)) {
        printf("(DEBUG) try_init_v4l2 VIDIOC_ENUMINPUT \"%s\": %s\n", device_name,
               strerror(errno));
        return RET_FAIL;
    }
    return RET_OK;
}

static ret_t usb_camera_try_palette_v4l2(usb_camera_device_t *usb_camera_device,
        unsigned long pixel_format)
{
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device->form = v4l2_format();
    usb_camera_device->form.fmt.pix.field = V4L2_FIELD_ANY;
    usb_camera_device->form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    usb_camera_device->form.fmt.pix.pixelformat = pixel_format;
    usb_camera_device->form.fmt.pix.width = usb_camera_device->base.device_ratio.width;
    usb_camera_device->form.fmt.pix.height = usb_camera_device->base.device_ratio.height;

    return_value_if_fail(usb_camera_try_ioctl(usb_camera_device, VIDIOC_S_FMT, &usb_camera_device->form) != -1, RET_FAIL);

    if (pixel_format == usb_camera_device->form.fmt.pix.pixelformat) {
        uint32_t min = usb_camera_device->form.fmt.pix.width * 2;
        if (usb_camera_device->form.fmt.pix.bytesperline < min) {
            usb_camera_device->form.fmt.pix.bytesperline = min;
        }
        usb_camera_device->base.device_ratio.bpp = usb_camera_device->form.fmt.pix.bytesperline / usb_camera_device->form.fmt.pix.width;
        return RET_OK;
    } else {
        return RET_FAIL;
    }
}

static ret_t
usb_camera_auto_setup_mode_v4l2(usb_camera_device_t *usb_camera_device)
{
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    __u32 try_order[] = {
        V4L2_PIX_FMT_BGR24,
        V4L2_PIX_FMT_RGB24,
        V4L2_PIX_FMT_YVU420,
        V4L2_PIX_FMT_YUV420,
        V4L2_PIX_FMT_YUV411P,
        V4L2_PIX_FMT_YUYV,
        V4L2_PIX_FMT_UYVY,
        V4L2_PIX_FMT_NV12,
        V4L2_PIX_FMT_NV21,
        V4L2_PIX_FMT_SBGGR8,
        V4L2_PIX_FMT_SGBRG8,
        V4L2_PIX_FMT_SN9C10X,
#ifdef WITH_JPEP_DECODE
        V4L2_PIX_FMT_MJPEG,
        V4L2_PIX_FMT_JPEG,
#endif
        V4L2_PIX_FMT_Y16,
        V4L2_PIX_FMT_GREY
    };

    for (uint32_t i = 0; i < sizeof(try_order) / sizeof(__u32); i++) {
        if (usb_camera_try_palette_v4l2(usb_camera_device, try_order[i]) == RET_OK) {
            printf("usb_camera_auto_setup_mode_v4l2:%d \r\n", i);
            usb_camera_device->palette = try_order[i];
            return RET_OK;
        }
    }
    return RET_FAIL;
}

static ret_t
usb_camera_get_buffer_number(usb_camera_device_t *usb_camera_device)
{
    uint32_t buffer_number = DEFAULT_V4L_BUFFERS;
    do {
        usb_camera_device->req.count = buffer_number;
        usb_camera_device->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        usb_camera_device->req.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_REQBUFS,
                        &usb_camera_device->req)) {
            if (EINVAL == errno) {
                printf("/dev/video%d does not support memory mapping\n",
                       usb_camera_device->base.device_info.camera_id);
            } else {
                printf("VIDIOC_REQBUFS \n");
            }
            return RET_FAIL;
        }

        if (usb_camera_device->req.count < buffer_number) {
            if (buffer_number == 1) {
                printf("Insufficient buffer memory on /dev/video%d\n",
                       usb_camera_device->base.device_info.camera_id);
                return RET_FAIL;
            } else {
                buffer_number--;
                printf("Insufficient buffer memory on /dev/video%d -- decreaseing "
                       "buffers\n",
                       usb_camera_device->base.device_info.camera_id);
            }
        } else {
            break;
        }
    } while (true);
    return RET_OK;
}

static ret_t usb_camera_buffer_init(usb_camera_device_t *usb_camera_device)
{
    uint32_t max_length = 0;
    for (uint32_t n_buffers = 0; n_buffers < (uint32_t)usb_camera_device->req.count; ++n_buffers) {
        v4l2_buffer buf = v4l2_buffer();

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == usb_camera_try_ioctl(usb_camera_device, VIDIOC_QUERYBUF, &buf)) {
            printf("VIDIOC_QUERYBUF \n");
            return RET_FAIL;
        }

        usb_camera_device->buffers[n_buffers].length = buf.length;
        usb_camera_device->buffers[n_buffers].start =
            mmap(NULL, buf.length, PROT_READ, MAP_SHARED,
                 usb_camera_device->device_handle, buf.m.offset);

        if (MAP_FAILED == usb_camera_device->buffers[n_buffers].start) {
            printf("mmap \n");
            return RET_FAIL;
        }

        max_length = max_length > buf.length ? max_length : buf.length;

        if (max_length > 0) {
            usb_camera_device->image_data_buffers.start = malloc(max_length);
            usb_camera_device->image_data_buffers.length = max_length;
        }
    }
    return RET_OK;
}

bool_t usb_camera_streaming(usb_camera_device_t *usb_camera_device, bool_t startStream)
{
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    return usb_camera_try_ioctl(usb_camera_device, startStream ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, (void *)(&type));
}

static ret_t usb_camera_start(usb_camera_device_t *usb_camera_device)
{
    ret_t ret = RET_OK;
    for (uint32_t i = 0; i < ((uint32_t)usb_camera_device->req.count); i++) {
        struct v4l2_buffer buf = v4l2_buffer();
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(usb_camera_device->device_handle, VIDIOC_QBUF, &buf) == -1) {
            printf("VIDIOC_QBUF \n");
            ret = RET_FAIL;
        }
    }

    if (!usb_camera_streaming(usb_camera_device, TRUE)) {
        printf("VIDIOC_STREAMON \n");
        ret = RET_FAIL;
    }

    return ret;
}

static bool_t usb_camera_read_frame_v4l2(usb_camera_device_t *usb_camera_device)
{
    v4l2_buffer buf = v4l2_buffer();
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    while (!usb_camera_try_ioctl(usb_camera_device, VIDIOC_DQBUF, &buf)) {
        if (errno == EIO && !(buf.flags & (V4L2_BUF_FLAG_QUEUED | V4L2_BUF_FLAG_DONE))) {
            // Maybe buffer not in the queue? Try to put there
            if (!usb_camera_try_ioctl(usb_camera_device, VIDIOC_QBUF, &buf)) {
                return false;
            }
            continue;
        }
        /* display the error and stop processing */
        perror("VIDIOC_DQBUF");
        return false;
    }

    if (buf.index < usb_camera_device->req.count) {
        memcpy(usb_camera_device->image_data_buffers.start,
               usb_camera_device->buffers[buf.index].start, buf.bytesused);

        usb_camera_device->image_data_buffers.length = buf.bytesused;

        usb_camera_device->buffer_index = buf.index;

        //set timestamp in capture struct to be timestamp of most recent frame
        usb_camera_device->timestamp = buf.timestamp;

        if (!usb_camera_try_ioctl(usb_camera_device, VIDIOC_QBUF, &buf)) {
            perror("VIDIOC_QBUF");
        }
        return true;
    }
    return false;
}

static ret_t usb_camera_set_fps(usb_camera_device_t *usb_camera_device, int value)
{
    struct v4l2_streamparm streamparm = v4l2_streamparm();
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = __u32(value);
    if (!usb_camera_try_ioctl(usb_camera_device, VIDIOC_S_PARM, &streamparm) || !usb_camera_try_ioctl(usb_camera_device, VIDIOC_G_PARM, &streamparm)) {
        return RET_FAIL;
    }

    // fps = streamparm.parm.capture.timeperframe.denominator;
    return RET_OK;
}

static usb_camera_device_t *usb_camera_device_init(uint32_t device_id,
        uint32_t width,
        uint32_t height,
        bool_t is_mirror)
{
    usb_camera_device_t *usb_camera_device = TKMEM_ZALLOC(usb_camera_device_t);
    if (usb_camera_device != NULL) {
        usb_camera_device->base.device_info.camera_id = device_id;
        usb_camera_device->base.is_mirror = is_mirror;

        usb_camera_device->base.device_ratio.width = width;   // DEFAULT_WIDTH;
        usb_camera_device->base.device_ratio.height = height; // DEFAULT_HEIGHT;
    }
    return usb_camera_device;
}

void *usb_camera_start_device(uint32_t device_id, uint32_t width,
                              uint32_t height, bool_t is_mirror)
{
    ret_t ret = RET_OK;
    usb_camera_device_t *usb_camera_device =
        usb_camera_device_init(device_id, width, height, is_mirror);

    ret = usb_camera_try_init_v4l2(usb_camera_device);

    if (ret != RET_OK &&
        (usb_camera_device->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
        printf("HIGHGUI ERROR: V4L2: device /dev/video%d is unable to capture "
               "video memory.\r\n",
               device_id);
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_device->inp.index > 0) {
        usb_camera_device->inp = v4l2_input();
        usb_camera_device->inp.index = CHANNEL_NUMBER;
        if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_ENUMINPUT,
                        &usb_camera_device->inp)) {
            printf("HIGHGUI ERROR: V4L2: Aren't able to set channel number\r\n");
            usb_camera_v4l2_close_device(usb_camera_device);
            return NULL;
        }
    }

    usb_camera_device->form = v4l2_format();
    usb_camera_device->form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_G_FMT,
                    &usb_camera_device->form)) {
        printf("HIGHGUI ERROR: V4L2: Could not obtain specifics of capture "
               "window.\r\n");
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_auto_setup_mode_v4l2(usb_camera_device) != RET_OK) {
        printf("usb_camera_auto_setup_mode_v4l2 fail \r\n");
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_set_fps(usb_camera_device, 30) != RET_OK) {
        printf("usb_camera_set_fps fail \r\n");
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_get_buffer_number(usb_camera_device) != RET_OK) {
        printf("usb_camera_get_buffer_number fail \r\n");
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_buffer_init(usb_camera_device) != RET_OK) {
        printf("usb_camera_buffer_init fail \r\n");
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_start(usb_camera_device) != RET_OK) {
        printf("usb_camera_start fail \r\n");
        usb_camera_v4l2_close_device(usb_camera_device);
        return NULL;
    }

    return usb_camera_device;
}

static void *usb_camera_v4l2_open_device(uint32_t device_id, bool_t is_mirror)
{
    return usb_camera_start_device(device_id, USB_CAMERA_DEFAULT_WIDTH,
                                   USB_CAMERA_DEFAULT_HEIGHT, is_mirror);
}

static void *usb_camera_v4l2_open_device_with_width_and_height(uint32_t device_id,
        bool_t is_mirror,
        uint32_t width,
        uint32_t height)
{
    return usb_camera_start_device(device_id, width, height, is_mirror);
}

static void *usb_camera_v4l2_set_ratio(void *p_usb_camera_device, uint32_t width,
                                       uint32_t height)
{
    uint32_t device_id = 0;
    bool_t is_mirror = FALSE;
    usb_camera_device_t *usb_camera_device = NULL;

    return_value_if_fail(p_usb_camera_device != NULL, NULL);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    return_value_if_fail(
        usb_camera_check_ratio(p_usb_camera_device, width, height) == RET_OK,
        NULL);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    is_mirror = usb_camera_device->base.is_mirror;
    device_id = usb_camera_device->base.device_info.camera_id;
    usb_camera_v4l2_close_device(usb_camera_device);
    sleep(1);
    return usb_camera_start_device(device_id, width, height, is_mirror);
}

static ret_t usb_camera_v4l2_close_device(void *p_usb_camera_device)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    usb_camera_device->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_STREAMOFF,
                    &usb_camera_device->type)) {
        printf("Unable to stop the stream.");
    }

    for (unsigned int n_buffers_ = 0; n_buffers_ < usb_camera_device->req.count;
         ++n_buffers_) {
        if (-1 == munmap(usb_camera_device->buffers[n_buffers_].start,
                         usb_camera_device->buffers[n_buffers_].length)) {
            printf("munmap");
        }
    }

    if (usb_camera_device->image_data_buffers.start) {
        free(usb_camera_device->image_data_buffers.start);
        usb_camera_device->image_data_buffers.start = NULL;
        usb_camera_device->image_data_buffers.length = 0;
    }

    if (usb_camera_device->device_handle != -1) {
        close(usb_camera_device->device_handle);
    }
    TKMEM_FREE(p_usb_camera_device);

    return RET_OK;
}

static ret_t color_format_conver(unsigned char *src, unsigned long palette,
                                 unsigned int width, unsigned int height,
                                 unsigned int line_length,
                                 unsigned int bpp, unsigned char *dst,
                                 bitmap_format_t format, unsigned int dst_len,
                                 unsigned char is_mirror,
                                 unsigned char is_reverse)
{
    uint32_t error = -1;
    if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGRA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGBA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGR565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGB565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2BGRA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2BGR565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2RGBA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2RGB565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2BGRA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2BGR565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2RGBA, src, width,
                height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2RGB565, src,
                width, height, line_length, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else {
        printf("color_format_conversion fail \r\n");
        error = 1;
    }

    return error == 0 ? RET_OK : RET_FAIL;
}

static ret_t usb_camera_v4l2_get_video_image_data(void *p_usb_camera_device,
        unsigned char *data,
        bitmap_format_t format,
        uint32_t data_len)
{
    ret_t ret = RET_OK;
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    ret = usb_camera_read_frame_v4l2(usb_camera_device) ? RET_OK : RET_FAIL;

    if (ret == RET_OK &&
        usb_camera_device->image_data_buffers.length ==
        usb_camera_device->base.device_ratio.width *
        usb_camera_device->base.device_ratio.height *
        usb_camera_device->base.device_ratio.bpp) {
        return color_format_conver(
                   (unsigned char *)usb_camera_device->image_data_buffers.start,
                   usb_camera_device->palette, usb_camera_device->base.device_ratio.width,
                   usb_camera_device->base.device_ratio.height, usb_camera_device->base.device_ratio.width * usb_camera_device->base.device_ratio.bpp,
                   usb_camera_device->base.device_ratio.bpp, data, format, data_len,
                   usb_camera_device->base.is_mirror, TRUE);
    } else if (usb_camera_device->palette == V4L2_PIX_FMT_JPEG || usb_camera_device->palette == V4L2_PIX_FMT_MJPEG) {
        usb_camera_decoder_t *decoder = usb_camera_decoder_create((unsigned char *)usb_camera_device->image_data_buffers.start, usb_camera_device->image_data_buffers.length);
        if (decoder != NULL) {
            ret_t ret = usb_camera_decoder_read_data(decoder, data, data_len, 0, format, usb_camera_device->base.is_mirror, TRUE);
            usb_camera_decoder_destroy(decoder);
            return ret;
        } else {
            printf("usb_camera_v4l2_get_video_image_data -- jpeg decoder fail \r\n");
            return RET_FAIL;
        }
    } else {
        printf("usb_camera_v4l2_get_video_image_data -- buffer sizes do not match \r\n");
        return RET_FAIL;
    }
}

const usb_camera_device_base_vt_t v4l2_vt = {
    usb_camera_v4l2_get_all_devices_info,
    usb_camera_v4l2_get_devices_ratio_list_for_device_id,
    usb_camera_v4l2_get_devices_ratio_list,
    usb_camera_v4l2_open_device_with_width_and_height,
    usb_camera_v4l2_open_device,
    usb_camera_v4l2_set_ratio,
    usb_camera_v4l2_close_device,
    usb_camera_v4l2_get_video_image_data,
};

extern "C" const usb_camera_drive_info_t v4l2_usb_camera_drive_info = {
    "V4L2",
    &v4l2_vt,
};
