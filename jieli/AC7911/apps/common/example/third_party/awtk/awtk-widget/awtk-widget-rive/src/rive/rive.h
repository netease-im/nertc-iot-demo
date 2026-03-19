/**
 * File:   rive.h
 * Author: AWTK Develop Team
 * Brief:  Lottie animations player
 *
 * Copyright (c) 2021 - 2021 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2021-06-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_RIVE_H
#define TK_RIVE_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class rive_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * Lottie animations player
 * 在xml中使用"rive"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <rive x="c" y="50" w="100" h="100"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <rive>
 *   <style name="default" border_color="red">
 *     <normal/>
 *   </style>
 * </rive>
 * ```
 */
typedef struct _rive_t {
    widget_t widget;

    /**
     * @property {char*} url
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 资源。
     */
    char *url;

    /**
     * @property {bool_t} running
     * @annotation ["readable","scriptable"]
     * 是否在播放。
     */
    bool_t running;

    /**
     * @property {double} fps
     * @annotation ["readable","scriptable"]
     * 动画的帧率。
     */
    double fps;

    /**
     * @property {uint32_t} width
     * @annotation ["readable","scriptable"]
     * 动画的默认宽度。
     */
    uint32_t width;

    /**
     * @property {uint32_t} height
     * @annotation ["readable","scriptable"]
     * 动画的默认高度。
     */
    uint32_t height;

    /**
     * @property {double} duration
     * @annotation ["readable","scriptable"]
     * 动画的时长(s)
     */
    double duration;

    /**
     * @property {uint32_t} total_frames
     * @annotation ["readable","scriptable"]
     * 动画的总帧数。
     */
    uint32_t total_frames;

    /*private*/
    bitmap_t *bitmap;
    uint32_t index;
    uint32_t timer_id;
    void *info;
} rive_t;

/**
 * @method rive_create
 * @annotation ["constructor", "scriptable"]
 * 创建rive对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} rive对象。
 */
widget_t *rive_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method rive_play
 * 播放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget rive对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t rive_play(widget_t *widget);

/**
 * @method rive_stop
 * 停止(并重置index为0)。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget rive对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t rive_stop(widget_t *widget);

/**
 * @method rive_pause
 * 暂停。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget rive对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t rive_pause(widget_t *widget);

/**
 * @method rive_cast
 * 转换为rive对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget rive对象。
 *
 * @return {widget_t*} rive对象。
 */
widget_t *rive_cast(widget_t *widget);

/**
 * @method rive_set_url
 * 设置资源的URL。
 *
 * > 使用本地文件时，请在file://后紧跟文件名。如:file://./data/heart.json
 * > 使用资源时，指定文件名即可，请把资源文件放到design/default/data/目录里。
 *
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {const char*} url 资源。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t rive_set_url(widget_t *widget, const char *url);

#define RIVE_PROP_URL "url"

#define WIDGET_TYPE_RIVE "rive"

#define RIVE(widget) ((rive_t*)(rive_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(rive);

END_C_DECLS

#endif /*TK_RIVE_H*/
