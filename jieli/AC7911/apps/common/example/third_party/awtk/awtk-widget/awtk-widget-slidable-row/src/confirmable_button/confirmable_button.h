/**
 * File:   confirmable_button.h
 * Author: AWTK Develop Team
 * Brief:  可确认的按钮。点击按钮时，弹出一个窗口，再次点击时，才触发CLICK事件。
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
 * 2020-07-30 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_CONFIRMABLE_BUTTON_H
#define TK_CONFIRMABLE_BUTTON_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class confirmable_button_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 可确认的按钮。点击按钮时，弹出一个窗口，再次点击时，才触发CLICK事件。
 *
 * 在xml中使用"confirmable\_button"标签创建可确认的按钮控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <confirmable_button w="120" h="36" tr_text="ConfirmableButton" popup_win_w="208"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <confirmable_button>
 *   <style name="default" text_color="#ffffff">
 *     <normal bg_color="#cc0000"/>
 *   </style>
 * </confirmable_button>
 * ```
 */
typedef struct _confirmable_button_t {
    widget_t widget;

    /**
     * @property {int32_t} popup_win_w
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 弹出窗口的宽度。
     */
    int32_t popup_win_w;

    /**
     * @property {bool_t} popup_to_right
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 向右弹出(默认FALSE，向左弹出)。
     */
    bool_t popup_to_right;

    /*private*/
    point_t down;
    bool_t pressed;
    bool_t in_popup;
} confirmable_button_t;

/**
 * @method confirmable_button_create
 * @annotation ["constructor", "scriptable"]
 * 创建confirmable_button对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} confirmable_button对象。
 */
widget_t *confirmable_button_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method confirmable_button_cast
 * 转换为confirmable_button对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget confirmable_button对象。
 *
 * @return {widget_t*} confirmable_button对象。
 */
widget_t *confirmable_button_cast(widget_t *widget);

/**
 * @method confirmable_button_set_popup_win_w
 * 设置 弹出窗口的宽度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {int32_t} popup_win_w 弹出窗口的宽度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t confirmable_button_set_popup_win_w(widget_t *widget, int32_t popup_win_w);

/**
 * @method confirmable_button_set_popup_to_right
 * 设置 向右弹出(默认FALSE，向左弹出)。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} popup_to_right 向右弹出(默认FALSE，向左弹出)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t confirmable_button_set_popup_to_right(widget_t *widget, bool_t popup_to_right);

#define CONFIRMABLE_BUTTON_PROP_POPUP_WIN_W "popup_win_w"
#define CONFIRMABLE_BUTTON_PROP_POPUP_TO_RIGHT "popup_to_right"

#define WIDGET_TYPE_CONFIRMABLE_BUTTON "confirmable_button"

#define CONFIRMABLE_BUTTON(widget) \
  ((confirmable_button_t*)(confirmable_button_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(confirmable_button);

END_C_DECLS

#endif /*TK_CONFIRMABLE_BUTTON_H*/
