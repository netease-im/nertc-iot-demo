/**
 * File:   table_view.h
 * Author: AWTK Develop Team
 * Brief:  表格视图。
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
 * 2020-07-15 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TABLE_VIEW_H
#define TK_TABLE_VIEW_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class table_view_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 表格视图。
 *
 * table\_view\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于table\_view\_t控件。
 *
 * 在xml中使用"table\_view"标签创建table\_view。
 *
 * table\_view中一般放table\_header、table\_client和滚动条控件。
 *
 * table\_header和滚动条为可选，table\_client为必选。
 *
 * 如：
 *
 * ```xml
 * <!-- ui -->
 * <table_view x="10" y="10" w="200" h="200">
 *   <table_header x="0" y="0" w="-12" h="30" name="table_header" children_layout="default(r=1,c=0,s=5,m=5)">
 *     <label w="30%" text="Name"/>
 *     <label w="40%" text="Value"/>
 *     <label w="30%" text="Action"/>
 *   </table_header>
 *
 *   <table_client name="table_client" x="0"  y="30" w="-12" h="-30" row_height="40">
 *     <table_row children_layout="default(r=1,c=0,s=5,m=5)" w="100%">
 *       <label name="name"  w="30%" h="100%" text="name"/>
 *       <edit name="value" w="40%" h="100%"/>
 *       <button name="remove" w="30%" h="100%" text="Remove"/>
 *     </table_row>
 *   </table_client>
 *   <scroll_bar_d name="scroll_bar" x="r" y="0" w="12" h="100%" value="0"/>
 * </table_view>
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
 * <table_client>
 *   <style name="default">
 *     <normal/>
 *   </style>
 * </table_client>
 * <table_row>
 *   <style name="default" border_color="#d8d8d8" border="bottom">
 *     <normal bg_color="#fcfcfc"/>
 *   </style>
 * </table_row>
 * <table_view>
 *   <style name="default" border_color="#c2c2c2">
 *     <normal bg_color="#f4f4f4"/>
 *   </style>
 * </table_view>
 * ```
 */
typedef struct _table_view_t {
    widget_t widget;

    /*private*/
    widget_t *client;
    widget_t *vbar;
} table_view_t;

/**
 * @method table_view_create
 * @annotation ["constructor", "scriptable"]
 * 创建table_view对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} table_view对象。
 */
widget_t *table_view_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method table_view_cast
 * 转换为table_view对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget table_view对象。
 *
 * @return {widget_t*} table_view对象。
 */
widget_t *table_view_cast(widget_t *widget);

#define WIDGET_TYPE_TABLE_VIEW "table_view"

#define TABLE_VIEW(widget) ((table_view_t*)(table_view_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(table_view);

END_C_DECLS

#endif /*TK_TABLE_VIEW_H*/
