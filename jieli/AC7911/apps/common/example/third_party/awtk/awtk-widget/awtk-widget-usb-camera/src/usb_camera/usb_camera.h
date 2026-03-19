/**
 * File:   usb_camera.h
 * Author: AWTK Develop Team
 * Brief:  USB 摄像头控件
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

#ifndef TK_USB_CAMERA_H
#define TK_USB_CAMERA_H

#include "base/widget.h"
#include "devices/usb_camera_devices.h"
#include "ext_widgets/mutable_image/mutable_image.h"

BEGIN_C_DECLS
/**
 * @class usb_camera_t
 * @parent mutable_image_t
 * @annotation ["scriptable","design","widget"]
 * USB 摄像头控件
 *
 * 该控件为 USB 摄像头控件，默认支持 window 和 linux 的摄像头驱动，如果需要支持其他的平台驱动的话，请重载 src/usb_camera/devices/usb_camera_devices.h 的函数。
 *
 * 在xml中使用"usb\_camera"标签创建usb\_camera控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <usb_camera name="usb_camera" x="0" y="0" w="200" h="200"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <usb_camera>
 *   <style name="default">
 *     <normal />
 *   </style>
 * </usb_camera>
 * ```
 *
 */
typedef struct _usb_camera_t {
    mutable_image_t base;

    /**
     * @property {uint32_t} camera_id
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 当前摄像头ID。
     */
    uint32_t camera_id;

    /**
     * @property {uint32_t} camera_width
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 当前摄像头数据流宽度。
     */
    uint32_t camera_width;

    /**
     * @property {uint32_t} camera_height
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 当前摄像头数据流高度。
     */
    uint32_t camera_height;

    /**
     * @property {bool_t} mirror
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 当前摄像头数据是否镜像。
     */
    bool_t mirror;

    /**
     * @property {slist_t} devices_info_list
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 接入摄像头列表。
     */
    slist_t devices_info_list;

    /**
     * @property {slist_t} devices_ratio_list
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 当前摄像头分辨率列表。
     */
    slist_t devices_ratio_list;

    /*private*/
    bool_t is_play;
    bool_t is_open;

    uint8_t *buff;
    void *p_device;

} usb_camera_t;

/**
 * @method usb_camera_create
 * @annotation ["constructor", "scriptable"]
 * 创建usb_camera对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} usb_camera对象。
 */
widget_t *usb_camera_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method usb_camera_cast
 * 转换为usb_camera对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget usb_camera对象。
 *
 * @return {widget_t*} usb_camera对象。
 */
widget_t *usb_camera_cast(widget_t *widget);

/**
 * @method usb_camera_set_camera_id
 * 设置当前摄像头ID。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} camera_id 当前摄像头ID。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_set_camera_id(widget_t *widget, uint32_t camera_id);

/**
 * @method usb_camera_set_camera_width
 * 设置当前摄像头数据流宽度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} width 当前摄像头数据流宽度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_set_camera_width(widget_t *widget, uint32_t width);

/**
 * @method usb_camera_set_camera_height
 * 设置当前摄像头数据流高度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} height 当前摄像头数据流高度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_set_camera_height(widget_t *widget, uint32_t height);

/**
 * @method usb_camera_set_camera_mirror
 * 设置当前摄像头数据是否镜像。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} mirror 当前摄像头数据是否镜像。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_set_camera_mirror(widget_t *widget, bool_t mirror);

/**
 * @method usb_camera_set_camera_width_and_height
 * 修改当前已经打开的摄像头的分辨率。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} width 当前摄像头数据流宽度。
 * @param {uint32_t} height 当前摄像头数据流高度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_set_camera_width_and_height(widget_t *widget, uint32_t width, uint32_t height);

/**
 * @method usb_camera_open
 * 按照当前分辨率打开当前摄像头。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_open(widget_t *widget);

/**
 * @method usb_camera_play
 * 显示当前摄像头的数据。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_play(widget_t *widget);

/**
 * @method usb_camera_stop
 * 停止显示当前摄像头的数据。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_stop(widget_t *widget);

/**
 * @method usb_camera_close
 * 关闭当前摄像头。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t usb_camera_close(widget_t *widget);

/**
 * @method usb_camera_enum_all_drives
 * 获取当前摄像头驱动数组
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t*} ret_len 返回数组长度。
 *
 * @return {const usb_camera_drive_info_t*} 返回系统摄像头驱动数组。
 */
const usb_camera_drive_info_t *usb_camera_enum_all_drives(widget_t *widget, uint32_t *ret_len);

/**
 * @method usb_camera_set_global_drive_index
 * @annotation ["constructor"]
 * 根据指定序号设置使用该驱动
 * @param {uint32_t*} ret_len 返回列表长度
 *
 * @return {const usb_camera_drive_info_t*} 返回系统摄像头驱动列表。
 */
ret_t usb_camera_set_global_drive_index(widget_t *widget, uint32_t index);

/**
 * @method usb_camera_enum_all_devices
 * 获取当前摄像头设备列表
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {slist_t*} 成功返回列表，失败返回NULL。
 */
slist_t *usb_camera_enum_all_devices(widget_t *widget);

/**
 * @method usb_camera_enum_device_all_ratio
 * 获取当前摄像头设备的支持的摄像头分辨率列表
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {slist_t*}  成功返回列表，失败返回NULL。
 */
slist_t *usb_camera_enum_device_all_ratio(widget_t *widget);

/**
 * @method usb_camera_is_play
 * 获取当前摄像头是否已经播放
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t}  成功返回TRUE，失败返回FALSE。
 */
bool_t usb_camera_is_play(widget_t *widget);

/**
 * @method usb_camera_is_open
 * 获取当前摄像头是否已经打开
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {bool_t}   成功返回TRUE，失败返回FALSE。
 */
bool_t usb_camera_is_open(widget_t *widget);

#define USB_CAMERA_PROP_MIRROR "mirror"
#define USB_CAMERA_PROP_CAMERA_ID "camera_id"
#define USB_CAMERA_PROP_CAMERA_WIDTH "camera_width"
#define USB_CAMERA_PROP_CAMERA_HEIGHT "camera_height"

#define WIDGET_TYPE_USB_CAMERA "usb_camera"

#define USB_CAMERA(widget) ((usb_camera_t*)(usb_camera_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(usb_camera);

/* private */
ret_t usb_camerat_register_from_base_class();


END_C_DECLS

#endif /*TK_USB_CAMERA_H*/
