/**
 * File:   cache_view.h
 * Author: AWTK Develop Team
 * Brief:  cache view
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
 * 2020-06-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_CACHE_VIEW_H
#define TK_CACHE_VIEW_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class cache_view_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 用于缓存子控件的控件。
 */
typedef struct _cache_view_t {
    widget_t widget;

    /**
     * @property {uint32_t} update_interval
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 更新缓存的时间间隔。
     */
    uint32_t update_interval;

    /**
     * @property {bool_t} disable_cache
     * @annotation ["readable","writable"]
     * 禁止缓存(主要用于对比测试)。
     */
    bool_t disable_cache;

    /*private*/
    uint64_t last_update;
    bitmap_t *bitmap;
    uint32_t idle_update_id;
} cache_view_t;

/**
 * @method cache_view_create
 * @annotation ["constructor", "scriptable"]
 * 创建cache_view对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} cache_view对象。
 */
widget_t *cache_view_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method cache_view_cast
 * 转换为cache_view对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget cache_view对象。
 *
 * @return {widget_t*} cache_view对象。
 */
widget_t *cache_view_cast(widget_t *widget);

/**
 * @method cache_view_set_update_interval
 * 设置 更新缓存的时间间隔。。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget widget对象。
 * @param {uint32_t} update_interval 更新缓存的时间间隔。。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t cache_view_set_update_interval(widget_t *widget, uint32_t update_interval);

#define CACHE_VIEW_PROP_UPDATE_INTERVAL "update_interval"

#define WIDGET_TYPE_CACHE_VIEW "cache_view"

#define CACHE_VIEW(widget) ((cache_view_t*)(cache_view_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(cache_view);

END_C_DECLS

#endif /*TK_CACHE_VIEW_H*/
