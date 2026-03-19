/**
 * File:   usb_camera_devices.h
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

#ifndef __USB_CAMERA_DEVICES_H__
#define __USB_CAMERA_DEVICES_H__

#include "awtk.h"
#include "tkc/slist.h"
#include "../decodes/usb_camera_decoder.h"

BEGIN_C_DECLS

// USB摄像头名字最大长度
#define USB_CAMERA_NAME_MAX_LENGTH 256

#define DRIVE_NAME_MAX_LENGTH 256

#define USB_CAMERA_DEFAULT_WIDTH 640

#define USB_CAMERA_DEFAULT_HEIGHT 480

typedef ret_t (*usb_camera_get_all_devices_info_t)(slist_t *devices_list);
typedef ret_t (*usb_camera_get_devices_ratio_list_for_device_id_t)(uint32_t device_id, slist_t *ratio_list);
typedef ret_t (*usb_camera_get_devices_ratio_list_t)(void *p_usb_camera_device, slist_t *ratio_list);
typedef void *(*usb_camera_open_device_with_width_and_height_t)(uint32_t device_id, bool_t is_mirror, uint32_t width, uint32_t height);
typedef void *(*usb_camera_open_device_t)(uint32_t device_id, bool_t is_mirror);
typedef void *(*usb_camera_set_ratio_t)(void *p_usb_camera_device, uint32_t width, uint32_t height);
typedef ret_t (*usb_camera_close_device_t)(void *p_usb_camera_device);
typedef ret_t (*usb_camera_get_video_image_data_t)(void *p_usb_camera_device, unsigned char *data, bitmap_format_t format, uint32_t data_len);

typedef struct _usb_camera_device_base_vt_t {
    usb_camera_get_all_devices_info_t get_all_devices_info;
    usb_camera_get_devices_ratio_list_for_device_id_t get_devices_ratio_list_for_device_id;
    usb_camera_get_devices_ratio_list_t get_devices_ratio_list;
    usb_camera_open_device_with_width_and_height_t open_device_with_width_and_height;
    usb_camera_open_device_t open_device;
    usb_camera_set_ratio_t set_ratio;
    usb_camera_close_device_t close_device;
    usb_camera_get_video_image_data_t get_video_image_data;
} usb_camera_device_base_vt_t;

/**
 * @class usb_camera_drive_info_t
 * @annotation ["scriptable"]
 * USB 摄像头设备驱动类型
 */
typedef struct _usb_camera_drive_info_t {
    /**
     * @property {const char*} name
     * @annotation ["readable"]
     * 驱动名字
     */
    const char *name;
    /* 虚表 */
    const usb_camera_device_base_vt_t *vt;
} usb_camera_drive_info_t;

/**
 * @class usb_camera_device_info_t
 * @annotation ["scriptable"]
 * USB 摄像头设备信息
 */
typedef struct _usb_camera_device_info_t {
    /**
     * @property {uint32_t} camera_id
     * @annotation ["readable"]
     * USB摄像头id
     */
    uint32_t camera_id;
    /**
     * @property {char*} camera_name
     * @annotation ["readable"]
     * USB摄像头的名字
     */
    char camera_name[USB_CAMERA_NAME_MAX_LENGTH];

#ifdef _WIN32
    /**
     * @property {wchar_t*} w_camera_name
     * @annotation ["readable"]
     * USB摄像头的名字（宽字节）
     */
    wchar_t w_camera_name[USB_CAMERA_NAME_MAX_LENGTH];
#endif

} usb_camera_device_info_t;

/**
 * @class usb_camera_ratio_t
 * @annotation ["scriptable"]
 * USB 摄像头分辨率信息
 */
typedef struct _usb_camera_ratio_t {
    /**
     * @property {uint32_t} id
     * @annotation ["readable"]
     * USB摄像头分辨率id
     */
    uint32_t id;
    /**
     * @property {uint32_t} bpp
     * @annotation ["readable"]
     * USB摄像头分辨率颜色位字节数
     */
    uint32_t bpp;
    /**
     * @property {uint32_t} width
     * @annotation ["readable"]
     * USB摄像头分辨率宽
     */
    uint32_t width;
    /**
     * @property {uint32_t} height
     * @annotation ["readable"]
     * USB摄像头分辨率高
     */
    uint32_t height;
} usb_camera_ratio_t;

/**
 * @class usb_camera_device_base_t
 * @annotation ["scriptable"]
 * USB 摄像头信息基类
 */
typedef struct _usb_camera_device_base_t {
    /**
     * @property {bool_t} is_mirror
     * @annotation ["readable"]
     * USB摄像头是否镜像
     */
    bool_t is_mirror;
    /**
     * @property {usb_camera_ratio_t} device_ratio
     * @annotation ["readable"]
     * USB摄像头分辨率信息
     */
    usb_camera_ratio_t device_ratio;
    /**
     * @property {usb_camera_device_info_t} device_info
     * @annotation ["readable"]
     * USB摄像头设备信息
     */
    usb_camera_device_info_t device_info;
} usb_camera_device_base_t;

/**
 * @method usb_camera_get_all_devices_info
 * @annotation ["constructor"]
 * 获取USB摄像头设备信息列表
 * @param {slist_t*} devices_list slist列表头
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败
 */
ret_t usb_camera_get_all_devices_info(slist_t *devices_list);

/**
 * @method usb_camera_get_devices_ratio_list
 * @annotation ["constructor"]
 * 通过设备句柄获取 USB 摄像头分辨率列表
 * @param {void*} p_usb_camera_device USB摄像头句柄
 * @param {slist_t*} ratio_list slist列表头
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败
 */
ret_t usb_camera_get_devices_ratio_list(void *p_usb_camera_device,
                                        slist_t *ratio_list);

/**
 * @method usb_camera_get_devices_ratio_list_for_device_id
 * @annotation ["constructor"]
 * 通过设备 ID 来获取 USB 摄像头分辨率列表
 * @param {uint32_t} device_id USB摄像头设备Id
 * @param {slist_t*} ratio_list slist列表头
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败
 */
ret_t usb_camera_get_devices_ratio_list_for_device_id(uint32_t device_id,
        slist_t *ratio_list);

/**
 * @method usb_camera_check_ratio
 * @annotation ["constructor"]
 * 检查USB摄像头是否支持该分辨率
 * @param {void*} usb_camera_device USB摄像头句柄
 * @param {uint32_t} width 分辨率的宽
 * @param {uint32_t} height 分辨率的高
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败
 */
ret_t usb_camera_check_ratio(void *usb_camera_device, uint32_t width,
                             uint32_t height);

/**
 * @method usb_camera_open_device
 * @annotation ["constructor"]
 * 打开USB摄像头
 * @param {uint32_t} device_id USB摄像头设备Id
 * @param {bool_t} is_mirror 是否是镜像
 *
 * @return {void*} 成功返回USB摄像头句柄，失败返回NULL。
 */
void *usb_camera_open_device(uint32_t device_id, bool_t is_mirror);

/**
 * @method usb_camera_open_device_with_width_and_height
 * @annotation ["constructor"]
 * 打开USB摄像头，并设置分辨率，如果摄像头不支持该分辨率，则使用默认分辨率
 * @param {uint32_t} device_id USB摄像头设备Id
 * @param {bool_t} is_mirror 是否是镜像
 * @param {uint32_t} width 设置分辨率的宽
 * @param {uint32_t} height 设置分辨率的高
 *
 * @return {void*} 成功返回USB摄像头句柄，失败返回NULL。
 */
void *usb_camera_open_device_with_width_and_height(uint32_t device_id,
        bool_t is_mirror,
        uint32_t width,
        uint32_t height);

/**
 * @method usb_camera_set_ratio
 * @annotation ["constructor"]
 * 设置USB摄像头分辨率，如果摄像头不支持该分辨率，就设置不成功，之前的USB摄像头句柄还是有效果
 * @param {void*} p_usb_camera_device USB摄像头句柄
 * @param {uint32_t} width 设置分辨率的宽
 * @param {uint32_t} height 设置分辨率的高
 *
 * @return {void*}
 *成功返回新设置的USB摄像头句柄，之前的USB摄像头句柄会被释放，失败返回NULL。
 */
void *usb_camera_set_ratio(void *p_usb_camera_device, uint32_t width,
                           uint32_t height);

/**
 * @method usb_camera_get_ratio
 * @annotation ["constructor"]
 * 获取USB摄像头分辨率
 * @param {void*} p_usb_camera_device USB摄像头句柄
 * @param {uint32_t*} width 输出分辨率的宽
 * @param {uint32_t*} height 输出分辨率的高
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_get_ratio(void *p_usb_camera_device, uint32_t *width,
                           uint32_t *height);

/**
 * @method usb_camera_set_mirror
 * @annotation ["constructor"]
 * 设置USB摄像头是否是镜像
 * @param {void*} p_usb_camera_device USB摄像头句柄
 * @param {bool_t} is_mirror 是否是镜像
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_set_mirror(void *p_usb_camera_device, bool_t is_mirror);

/**
 * @method usb_camera_close_device
 * @annotation ["constructor"]
 * 关闭USB摄像头
 * @param {void*} p_usb_camera_device USB摄像头句柄
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_close_device(void *p_usb_camera_device);

/**
 * @method usb_camera_get_video_image_data
 * @annotation ["constructor"]
 * 获取USB摄像头图像数据
 * @param {void*} p_usb_camera_device USB摄像头句柄
 * @param {unsigned char*} data 输出图像字节数组
 * @param {bitmap_format_t} format 输出图像格式
 * @param {uint32_t} data_len data参数的字节的长度
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_get_video_image_data(void *p_usb_camera_device,
                                      unsigned char *data,
                                      bitmap_format_t format,
                                      uint32_t data_len);

/**
 * @method usb_camera_prepare_image_fun
 * @annotation ["constructor"]
 * 准备图片的回调函数
 * @param {void*} ctx USB摄像头句柄
 * @param {bitmap_t*} image 输出图像
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_prepare_image_fun(void *ctx, bitmap_t *image);

/**
 * @method usb_camera_prepare_image_create_image_fun
 * @annotation ["constructor"]
 * 创建 bitmap 的回调函数
 * @param {void*} ctx USB摄像头句柄
 * @param {bitmap_format_t} format lcd 格式
 * @param {bitmap_t*} old_image 输出图像
 *
 * @return {bitmap_t*} 成功返回 bitmap，失败返回 NULL。
 */
bitmap_t *usb_camera_prepare_image_create_image_fun(void *ctx, bitmap_format_t format,
        bitmap_t *old_image);

/**
 * @method usb_camera_devices_info_list_destroy
 * @annotation ["constructor"]
 * USB摄像头设备类型释放函数，给slist对象使用的
 * @param {void*} data USB摄像头设备类型
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_devices_info_list_destroy(void *data);

/**
 * @method usb_camera_devices_info_list_compare
 * @annotation ["constructor"]
 * USB摄像头设备类型比较函数，给slist对象使用的
 * @param {const void*} a USB摄像头设备类型
 * @param {const void*} b USB摄像头设备类型
 *
 * @return {int} 成功返回0，失败返回1。
 */
int usb_camera_devices_info_list_compare(const void *a, const void *b);

/**
 * @method usb_camera_device_ratio_list_destroy
 * @annotation ["constructor"]
 * USB摄像头分辨率类型释放函数，给slist对象使用的
 * @param {void*} data USB摄像头分辨率类型
 *
 * @return {ret_t} 成功返回RET_OK，返回其他表示失败。
 */
ret_t usb_camera_device_ratio_list_destroy(void *data);

/**
 * @method usb_camera_device_ratio_list_compare
 * @annotation ["constructor"]
 * USB摄像头分辨率类型比较函数，给slist对象使用的
 * @param {const void*} a USB摄像头分辨率类型
 * @param {const void*} b USB摄像头分辨率类型
 *
 * @return {int} 成功返回0，失败返回1。
 */
int usb_camera_device_ratio_list_compare(const void *a, const void *b);

/**
 * @method usb_camera_device_get_drive_list
 * @annotation ["constructor"]
 * 获取系统摄像头驱动数组
 * @param {uint32_t*} ret_len 返回数组长度
 *
 * @return {const usb_camera_drive_info_t*} 返回系统摄像头驱动数组。
 */
const usb_camera_drive_info_t *usb_camera_device_get_drive_list(uint32_t *ret_len);

/**
 * @method usb_camera_device_set_global_drive_index
 * @annotation ["constructor"]
 * 根据指定序号设置使用该驱动
 * @param {uint32_t*} ret_len 返回列表长度
 *
 * @return {const usb_camera_drive_info_t*} 返回系统摄像头驱动列表。
 */
ret_t usb_camera_device_set_global_drive_index(uint32_t index);

END_C_DECLS

#endif // !__USB_CAMERA_DEVICES_H__
