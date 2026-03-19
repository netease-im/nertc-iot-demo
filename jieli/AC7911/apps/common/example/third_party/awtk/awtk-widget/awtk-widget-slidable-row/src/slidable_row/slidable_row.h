/**
 * File:   slidable_row.h
 * Author: AWTK Develop Team
 * Brief:  slidable row(仿微信列表项目，像左滑动显示“删除”按钮)。
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
 * 2020-07-28 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_SLIDABLE_ROW_H
#define TK_SLIDABLE_ROW_H

#include "base/widget.h"
#include "base/widget_animator.h"

BEGIN_C_DECLS
/**
 * @class slidable_row_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * slidable row(仿微信列表项目，像左滑动显示“删除”按钮)。
 *
 * 在xml中使用"slidable\_row"标签创建仿微信列表项目控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <slidable_row w="200" h="28">
 *   <label name="title" text="title"/>
 *   <button name="mark" w="100" style="mark_as_read" tr_text="Mark As Read"/>
 *   <confirmable_button name="confirm_remove" w="50" tr_text="Remove" popup_win_w="208"/>
 * </slidable_row>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <slidable_row>
 *   <style name="default" border_color="#E9E9E9">
 *     <normal bg_color="#F4F4F4"/>
 *   </style>
 * </slidable_row>
 * <button>
 *   <style name="mark_as_read" bg_color="#F4F4F4" border_color="#338FFF" text_color="#444444">
 *     <normal border_color="#C2C2C2"/>
 *     <disable bg_color="#F4F4F466" border_color="#C2C2C266" text_color="#44444466"/>
 *     <focused/>
 *     <over bg_color="#338FFF" text_color="#FFFFFF"/>
 *     <pressed bg_color="#1971DD" border_color="#1971DD" text_color="#FFFFFF"/>
 *   </style>
 * </button>
 * <confirmable_button>
 *   <style name="default" text_color="#ffffff">
 *     <normal bg_color="#cc0000"/>
 *   </style>
 * </confirmable_button>
 * ```
 */
typedef struct _slidable_row_t {
    widget_t widget;

    /**
     * @property {int32_t} xoffset
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * xoffset。
     */
    int32_t xoffset;

    /*private*/
    bool_t expaneded;
    bool_t pressed;
    point_t down;
    bool_t dragged;
    widget_animator_t *wa;
} slidable_row_t;

/**
 * @method slidable_row_create
 * @annotation ["constructor", "scriptable"]
 * 创建slidable_row对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} slidable_row对象。
 */
widget_t *slidable_row_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method slidable_row_cast
 * 转换为slidable_row对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget slidable_row对象。
 *
 * @return {widget_t*} slidable_row对象。
 */
widget_t *slidable_row_cast(widget_t *widget);

/**
 * @method slidable_row_set_xoffset
 * 设置 xoffset。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {int32_t} xoffset xoffset。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t slidable_row_set_xoffset(widget_t *widget, int32_t xoffset);

#define SLIDABLE_ROW_PROP_XOFFSET "slidable_row_xoffset"

#define WIDGET_TYPE_SLIDABLE_ROW "slidable_row"

#define SLIDABLE_ROW(widget) ((slidable_row_t*)(slidable_row_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(slidable_row);

END_C_DECLS

#endif /*TK_SLIDABLE_ROW_H*/
