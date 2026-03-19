/**
 * File:   table_header.h
 * Author: AWTK Develop Team
 * Brief:  table_header
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
 * 2020-07-18 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TABLE_HEADER_H
#define TK_TABLE_HEADER_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class table_header_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * table\_header。一个简单的容器控件，用来放置表头中的子控件。
 *
 * 它本身不提供布局功能，仅提供具有语义的标签，让xml更具有可读性。
 * 子控件的布局可用layout\_children属性指定。
 * 请参考[布局参数](https://github.com/zlgopen/awtk/blob/master/docs/layout.md)。
 *
 * table\_header\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于table\_header\_t控件。
 *
 * 在xml中使用"table\_header"标签创建table\_header。如：
 *
 * ```xml
 * <!-- ui -->
 * <table_header x="0" y="0" w="200" h="30"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如背景颜色等。如：
 *
 * ```xml
 * <!-- style -->
 * <table_header>
 *   <style name="default">
 *     <normal bg_color="#e0e0e0"/>
 *   </style>
 * </table_header>
 * ```
 */
typedef struct _table_header_t {
    widget_t widget;

} table_header_t;

/**
 * @method table_header_create
 * @annotation ["constructor", "scriptable"]
 * 创建table_header对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} table_header对象。
 */
widget_t *table_header_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method table_header_cast
 * 转换为table_header对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget table_header对象。
 *
 * @return {widget_t*} table_header对象。
 */
widget_t *table_header_cast(widget_t *widget);

#define WIDGET_TYPE_TABLE_HEADER "table_header"

#define TABLE_HEADER(widget) ((table_header_t*)(table_header_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(table_header);

END_C_DECLS

#endif /*TK_TABLE_HEADER_H*/
