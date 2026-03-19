/**
 * File:   table_client.h
 * Author: AWTK Develop Team
 * Brief:  表格视图数据区。
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

#ifndef TK_TABLE_CLIENT_H
#define TK_TABLE_CLIENT_H

#include "base/widget.h"
#include "base/velocity.h"
#include "base/widget_animator.h"

BEGIN_C_DECLS

typedef ret_t (*table_client_on_load_data_t)(void *ctx, uint32_t row_index, widget_t *row);
typedef ret_t (*table_client_on_create_row_t)(void *ctx, uint32_t row_index, widget_t *row);
typedef ret_t (*table_client_on_prepare_row_t)(void *ctx, widget_t *client, uint32_t prepare_cnt);

/**
 * @class table_client_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * table\_client。表示表格的数据区。
 *
 * table\_client\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于table\_client\_t控件。
 *
 * 在xml中使用"table\_client"标签创建table\_client。
 *
 * 一般放在table\_client对象中放一个table\_row即可，table\_client以此为模版，根据需要创建table\_row对象。
 *
 * 如：
 *
 * ```xml
 * <!-- ui -->
 * <table_client x="0" y="0" w="200" h="200"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如背景颜色等。如：
 *
 * ```xml
 * <!-- style -->
 * <table_client>
 *   <style name="default">
 *     <normal />
 *   </style>
 * </table_client>
 * ```
 */
typedef struct _table_client_t {
    widget_t widget;

    /**
     * @property {uint32_t} row_height
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 行高。
     */
    uint32_t row_height;

    /**
     * @property {uint32_t} rows
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 最大行数。
     */
    uint32_t rows;

    /**
     * @property {int32_t} yoffset
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 偏移量。
     */
    int32_t yoffset;

    /**
     * @property {bool_t} yslidable
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 是否允许y方向滑动。
     */
    bool_t yslidable;

    /**
     * @property {float_t} yspeed_scale
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * y偏移速度比例。
     */
    float_t yspeed_scale;

    /*private*/
    uint32_t start_index;

    point_t down;
    bool_t pressed;
    bool_t dragged;
    velocity_t velocity;
    int32_t yoffset_end;
    int32_t yoffset_save;
    widget_animator_t *wa;
    bool_t first_move_after_down;

    /*用于加载数据的回调函数*/
    void *on_load_data_ctx;
    table_client_on_load_data_t on_load_data;

    /*创建行时的回调函数，可以注册事件处理函数*/
    void *on_create_row_ctx;
    table_client_on_create_row_t on_create_row;

    /*预处理行（创建行）的回调函数，可以注册事件处理函数*/
    void *on_prepare_row_ctx;
    table_client_on_prepare_row_t on_prepare_row;

    uint32_t load_data_idle_id;
} table_client_t;

/**
 * @event {event_t} EVT_SCROLL_START
 * 开始滚动事件。
 */

/**
 * @event {event_t} EVT_SCROLL_END
 * 结束滚动事件。
 */

/**
 * @event {event_t} EVT_SCROLL
 * 滚动事件。
 */

/**
 * @method table_client_create
 * @annotation ["constructor", "scriptable"]
 * 创建table_client对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} table_client对象。
 */
widget_t *table_client_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method table_client_cast
 * 转换为table_client对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget table_client对象。
 *
 * @return {widget_t*} table_client对象。
 */
widget_t *table_client_cast(widget_t *widget);

/**
 * @method table_client_set_row_height
 * 设置 行高。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} row_height 行高。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_row_height(widget_t *widget, uint32_t row_height);

/**
 * @method table_client_set_rows
 * 设置 最大行数。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} rows 最大行数。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_rows(widget_t *widget, uint32_t rows);

/**
 * @method table_client_set_yoffset
 * 设置 偏移量。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {int32_t} yoffset 偏移量。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_yoffset(widget_t *widget, int32_t yoffset);

/**
 * @method table_client_set_yslidable
 * 设置 是否允许y方向滑动。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {bool_t} yslidable 是否允许y方向滑动。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_yslidable(widget_t *widget, bool_t yslidable);

/**
 * @method table_client_set_yspeed_scale
 * 设置 y偏移速度比例。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {float_t} yspeed_scale y偏移速度比例。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_yspeed_scale(widget_t *widget, float_t yspeed_scale);

/**
 * @method table_client_set_on_load_data
 * 设置 加载数据的回调函数。
 * @param {widget_t*} widget widget对象。
 * @param {table_client_on_load_data_t} on_load_data 回调函数。
 * @param {void*} ctx 回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_on_load_data(widget_t *widget, table_client_on_load_data_t on_load_data,
                                    void *ctx);

/**
 * @method table_client_set_on_create_row
 * 设置 创建行时的回调函数，在回调函数中可以注册控件的事件。
 * @param {widget_t*} widget widget对象。
 * @param {table_client_on_create_row_t} on_create_row 回调函数。
 * @param {void*} ctx 回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_on_create_row(widget_t *widget, table_client_on_create_row_t on_create_row,
                                     void *ctx);

/**
 * @method table_client_set_on_prepare_row
 * 设置 预处理行（创建行）的回调函数，在回调函数中可以创建行控件。
 * @param {widget_t*} widget widget对象。
 * @param {table_client_on_prepare_row_t} on_prepare_row 回调函数。
 * @param {void*} ctx 回调函数的上下文。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_set_on_prepare_row(widget_t *widget,
                                      table_client_on_prepare_row_t on_prepare_row, void *ctx);

/**
 * @method table_client_get_virtual_h
 * 获取虚拟高度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {int64_t} 返回虚拟高度。
 */
int64_t table_client_get_virtual_h(widget_t *widget);

/**
 * @method table_client_scroll_to_row
 * 滚动到指定行。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {int32_t} row 行号。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_scroll_to_row(widget_t *widget, int32_t row);

/**
 * @method table_client_scroll_to_yoffset
 * 滚动到指定偏移位置。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {int32_t} yoffset 偏移量。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_scroll_to_yoffset(widget_t *widget, int32_t yoffset);

/**
 * @method table_client_ensure_children
 * 确保子控件已经创建。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_ensure_children(widget_t *widget);

/**
 * @method table_client_reload
 * 数据变化时让table client重新加载数据。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t table_client_reload(widget_t *widget);

#define TABLE_CLIENT_PROP_ROWS "rows"
#define TABLE_CLIENT_PROP_YOFFSET "yoffset"
#define TABLE_CLIENT_PROP_YSLIDABLE "yslidable"
#define TABLE_CLIENT_PROP_YSPEED_SCALE "yspeed_scale"
#define TABLE_CLIENT_PROP_ROW_HEIGHT "row_height"

#define WIDGET_TYPE_TABLE_CLIENT "table_client"

#define TABLE_CLIENT(widget) ((table_client_t*)(table_client_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(table_client);

END_C_DECLS

#endif /*TK_TABLE_CLIENT_H*/
