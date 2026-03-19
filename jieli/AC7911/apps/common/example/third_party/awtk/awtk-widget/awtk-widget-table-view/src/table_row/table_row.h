/**
 * File:   table_row.h
 * Author: AWTK Develop Team
 * Brief:  table_row
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
 * 2020-07-17 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TABLE_ROW_H
#define TK_TABLE_ROW_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class table_row_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * table\_row。表示表格的一行。
 *
 * 它本身不提供布局功能，仅提供具有语义的标签，让xml更具有可读性。
 * 子控件的布局可用layout\_children属性指定。
 * 请参考[布局参数](https://github.com/zlgopen/awtk/blob/master/docs/layout.md)。
 *
 * table\_row\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于table\_row\_t控件。
 *
 * 在xml中使用"table\_row"标签创建table\_row。
 *
 * table\_row一般放在table\_client对象中，创建一个对象即可，table\_client以此为模版，根据需要创建table\_row对象。
 *
 * 如：
 *
 * ```xml
 * <!-- ui -->
 * <table_row x="0" y="0" w="200" h="30"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如背景颜色等。如：
 *
 * ```xml
 * <!-- style -->
 * <table_row>
 *   <style name="default" border_color="#d8d8d8" border="bottom">
 *     <normal bg_color="#fcfcfc"/>
 *   </style>
 * </table_row>
 * ```
 */
typedef struct _table_row_t {
    widget_t widget;

    /**
     * @property {uint32_t} index
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 行的编号。
     */
    uint32_t index;

    /*private*/
    point_t down;
} table_row_t;

/**
 * @event {pointer_event_t} EVT_CLICK
 * 点击事件。
 */

/**
 * @method table_row_create
 * @annotation ["constructor", "scriptable"]
 * 创建table_row对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} table_row对象。
 */
widget_t *table_row_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method table_row_cast
 * 转换为table_row对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget table_row对象。
 *
 * @return {widget_t*} table_row对象。
 */
widget_t *table_row_cast(widget_t *widget);

/**
 * @method table_row_set_index
 * 设置 行的编号。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} index 行的编号。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_row_set_index(widget_t *widget, uint32_t index);

#define TABLE_ROW_PROP_INDEX "index"

#define WIDGET_TYPE_TABLE_ROW "table_row"

#define TABLE_ROW(widget) ((table_row_t*)(table_row_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(table_row);

END_C_DECLS

#endif /*TK_TABLE_ROW_H*/
