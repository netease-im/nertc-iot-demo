/**
 * File:   vlabel.h
 * Author: AWTK Develop Team
 * Brief:  垂直显示文本的控件。
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
 * 2020-10-01 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_VLABEL_H
#define TK_VLABEL_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class vlabel_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 垂直显示文本的控件。
 * 在xml中使用"vlabel"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <vlabel x="c" y="50" w="100" h="100" text="国庆快乐！"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <vlabel>
 *   <style name="default" font_size="32">
 *     <normal text_color="black" />
 *   </style>
 * </vlabel>
 * ```
 */
typedef struct _vlabel_t {
    widget_t widget;

    /**
     * @property {bool_t} right_to_left
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否从右向左排列。。
     */
    bool_t right_to_left;

} vlabel_t;

/**
 * @method vlabel_create
 * @annotation ["constructor", "scriptable"]
 * 创建vlabel对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} vlabel对象。
 */
widget_t *vlabel_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method vlabel_cast
 * 转换为vlabel对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget vlabel对象。
 *
 * @return {widget_t*} vlabel对象。
 */
widget_t *vlabel_cast(widget_t *widget);

/**
 * @method vlabel_set_right_to_left
 * 设置 是否从右向左排列。。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} right_to_left 是否从右向左排列。。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t vlabel_set_right_to_left(widget_t *widget, bool_t right_to_left);

#define VLABEL_PROP_RIGHT_TO_LEFT "right_to_left"

#define WIDGET_TYPE_VLABEL "vlabel"

#define VLABEL(widget) ((vlabel_t*)(vlabel_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(vlabel);

/*public for test*/
/**
 * @class vlabel_line_parser_t
 */
typedef struct _vlabel_line_parser_t {
    uint32_t size;
    uint32_t height;
    uint32_t font_size;
    const wchar_t *str;

    uint32_t total_lines;
    uint32_t total_paragraphs;

    /*current line*/
    const wchar_t *line;
    uint32_t line_size;
    bool_t is_force_break;
} vlabel_line_parser_t;

/**
 * @method vlabel_line_parser_init
 */
ret_t vlabel_line_parser_init(vlabel_line_parser_t *parser, const wchar_t *str, uint32_t size,
                              uint32_t font_size, uint32_t height);

/**
 * @method vlabel_line_parser_next
 */
ret_t vlabel_line_parser_next(vlabel_line_parser_t *parser);

END_C_DECLS

#endif /*TK_VLABEL_H*/
