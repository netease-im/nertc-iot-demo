/**
 * File:   number_label.h
 * Author: AWTK Develop Team
 * Brief:  number_label
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-05-19 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_NUMBER_LABEL_H
#define TK_NUMBER_LABEL_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @class number_label_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 数值文本控件。
 *
 * 用于显示数值的文本控件。
 *
 * 可以通过指定属性 format 来改变小数的有效位数。
 * 可以通过指定属性 decimal\_font\_size\_scale 来减小小数部分的字体大小，以强调整数部分的重要性。
 *
 * number\_label\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于number\_label\_t控件。
 *
 * 在xml中使用"number\_label"标签创建数值文本控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <number_label x="c" y="50" w="100" h="40" value="-128" format="%.4lf" decimal_font_size_scale="0.5"/>
 * ```
 *
 * 在c代码中使用函数number\_label\_create创建数值文本控件。如：
 *
 * ```c
 *  widget_t* number_label = number_label_create(win, 10, 10, 128, 30);
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <number_label>
 *   <style name="default" font_size="32">
 *     <normal text_color="black" />
 *   </style>
 * </number_label>
 * ```
 */
typedef struct _number_label_t {
    widget_t widget;

    /**
     * @property {char*} format
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     *
     * 格式字符串。
     *
     */
    char *format;

    /**
     * @property {double} decimal_font_size_scale
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     *
     * 小数部分字体大小与整数部分字体大小之比。
     * 通常设置为0.5到1之间，让小数部分字体变小，用于强调整数部分。
     *
     */
    double decimal_font_size_scale;

    /**
     * @property {double} value
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 数值。
     *
     */
    double value;

    /**
     * @property {bool_t} readonly
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 编辑器是否为只读。
     */
    bool_t readonly;

    /**
     * @property {bool_t} loop
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 值是否循环。用上下键修改时，到达最小值时是否跳到最大值，到达最大值时是否跳到最小值。
     */
    bool_t loop;

    /**
     * @property {double} min
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 最小值。
     */
    double min;

    /**
     * @property {double} max
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 最大值。
     */
    double max;

    /**
     * @property {double} step
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 步长。
     * 用上下键修改时，一次增加和减少时的数值。
     */
    double step;

    /*private*/
} number_label_t;

/**
 * @method number_label_create
 * 创建number_label对象
 * @annotation ["constructor", "scriptable"]
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} 对象。
 */
widget_t *number_label_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method number_label_cast
 * 转换为number_label对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget number_label对象。
 *
 * @return {widget_t*} number_label对象。
 */
widget_t *number_label_cast(widget_t *widget);

/**
 * @method number_label_set_format
 * 设置格式字符串(用于将浮点数转换为字符串)。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} format 格式字符串。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_set_format(widget_t *widget, const char *format);

/**
 * @method number_label_set_decimal_font_size_scale
 * 设置小数部分字体大小与整数部分字体大小之比。
 *
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {double} decimal_font_size_scale 小数部分字体大小与整数部分字体大小之比。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_set_decimal_font_size_scale(widget_t *widget, double decimal_font_size_scale);

/**
 * @method number_label_set_readonly
 * 设置控件是否为只读。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} readonly 只读。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_set_readonly(widget_t *widget, bool_t readonly);

/**
 * @method number_label_set_loop
 * 设置 值是否循环
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} loop 值是否循环。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_set_loop(widget_t *widget, bool_t loop);

/**
 * @method number_label_set_limit
 * 设置取值范围和步长。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {double} min 最小值。
 * @param {double} max 最大值。
 * @param {double} step 步长。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_set_limit(widget_t *widget, double min, double max, double step);

/**
 * @method number_label_set_value
 * 设置数值。
 *
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {double} value 数值。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t number_label_set_value(widget_t *widget, double value);

#define NUMBER_LABEL_PROP_FORMAT "format"
#define NUMBER_LABEL_PROP_DECIMAL_FONT_SIZE_SCALE "decimal_font_size_scale"

#define WIDGET_TYPE_NUMBER_LABEL "number_label"
#define NUMBER_LABEL(widget) ((number_label_t*)(number_label_cast(WIDGET(widget))))
#define NUMBER_LABEL_DEFAULT_FORMAT "%.02lf"

END_C_DECLS

#endif /*TK_NUMBER_LABEL_H*/
