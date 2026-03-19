/**
 * File:   title_value.h
 * Author: AWTK Develop Team
 * Brief:  用于显示标题和值的控件。
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
 * 2020-10-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TITLE_VALUE_H
#define TK_TITLE_VALUE_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class title_value_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 用于显示标题和值的控件。
 * 在xml中使用"title\_value"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <title_value x="c" y="50" w="100" h="100"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <title_value>
 *   <style name="default" font_size="32">
 *     <normal text_color="black" />
 *   </style>
 * </title_value>
 * ```
 */
typedef struct _title_value_t {
    widget_t widget;

    /**
     * @property {bool_t} grab_focus
     * @annotation ["set_prop","get_prop","readable"]
     * 是否抓住焦点。抓住焦点之后，方向键不再切换焦点。
     *
     *> 此时可以用上下键修改控件的值，主要在只有左右键和OK键时的硬件平台使用。
     */
    uint8_t grab_focus : 1;

    /**
     * @property {bool_t} return_key_to_grab_focus
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否用回车键来触发grab focus状态。
     *
     * > 在只有方向键和OK键时的硬件平台，配合grab_focus使用：
     * > * 先用方向键切换焦点，切换到指定的控件。
     * > * 再按OK键进入编辑状态，此时用方向键修改值。
     * > * 完成后按OK键，退出编辑状态。
     * > * 方向键继续用于切换焦点。
     */
    uint8_t return_key_to_grab_focus : 1;

} title_value_t;

/**
 * @method title_value_create
 * @annotation ["constructor", "scriptable"]
 * 创建title_value对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} title_value对象。
 */
widget_t *title_value_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method title_value_cast
 * 转换为title_value对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget title_value对象。
 *
 * @return {widget_t*} title_value对象。
 */
widget_t *title_value_cast(widget_t *widget);

/**
 * @method title_value_set_grab_focus
 * 设置控件是否抓住焦点。
 *
 * > 抓住焦点之后，方向键不再切换焦点。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} grab_focus 是否抓住焦点。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t title_value_set_grab_focus(widget_t *widget, bool_t grab_focus);

/**
 * @method title_value_set_return_key_to_grab_focus
 * 设置控件是否用回车键来触发grab focus状态。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} return_key_to_grab_focus 是否用回车键来触发grab focus状态。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t title_value_set_return_key_to_grab_focus(widget_t *widget, bool_t return_key_to_grab_focus);

#define WIDGET_TYPE_TITLE_VALUE "title_value"

#define TITLE_VALUE(widget) ((title_value_t*)(title_value_cast(WIDGET(widget))))

#define TITLE_VALUE_PROP_GRAB_FOCUS "grab_focus"
#define TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS "return_key_to_grab_focus"

#define TITLE_VALUE_STATE_FOCUS_GRABED "focus_grabed"

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(title_value);

END_C_DECLS

#endif /*TK_TITLE_VALUE_H*/

