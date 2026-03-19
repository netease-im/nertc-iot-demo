/**
 * File:   tree_view.h
 * Author: AWTK Develop Team
 * Brief:  tree_view
 *
 * Copyright (c) 2018 - 2022  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2022-05-05 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TREE_VIEW_H
#define TK_TREE_VIEW_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @class tree_view_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * 树形视图控件。
 *
 * tree\_view\_t是[widget\_t](widget_t.md)的子类控件，widget\_t的函数均适用于tree\_view\_t控件。
 *
 * 在xml中使用"tree\_view"标签创建列表视图控件。如：
 *
 * ```xml
* <!-- ui -->
 * <tree_view x="0" y="0" w="48%" h="48%" item_height="25" spacing="5" auto_hide_scroll_bar="true">
 *   <scroll_view name="view" x="0" y="0" w="-12" h="-12">
 *     <tree_item value="true">
 *       <tree_item_title text="hello" style="folder"/>
 *       <tree_item_content>
 *         <tree_item_title text="1.world" style="file"/>
 *         <tree_item_title text="2.world" style="file"/>
 *         <tree_item_title text="3.world" style="file"/>
 *       </tree_item_content>
 *     </tree_item>
 *
 *     <tree_item>
 *       <tree_item_title text="awtk" style="folder"/>
 *       <tree_item_content>
 *         <tree_item_title text="1.world" style="file"/>
 *         <tree_item_title text="2.world" style="file"/>
 *         <tree_item_title text="3.world" style="file"/>
 *       </tree_item_content>
 *     </tree_item>
 *   </scroll_view>
 *   <scroll_bar_d name="hbar" x="0" y="b" w="-12" h="12" value="0"/>
 *   <scroll_bar_d name="vbar" x="right" y="0" w="12" h="-12" value="0"/>
 * </tree_view>
 * ```
 *
  * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 *<tree_view>
 * <style name="default" margin="10" bg_color="#f0f0f0">
 *   <normal />
 * </style>
 *</tree_view>
 * ```
 *
 *
 */
typedef struct _tree_view_t {
    widget_t widget;
    /**
     * @property {int32_t} item_height
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 列表项的高度。
     */
    int32_t item_height;

    /**
     * @property {int32_t} spacing
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 列表项之间的间距。
     */
    int32_t spacing;

    /**
     * @property {bool_t} auto_hide_scroll_bar
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 如果不需要滚动条时，自动隐藏滚动条。
     */
    bool_t auto_hide_scroll_bar;

    /*private*/
    widget_t *scroll_view;
    widget_t *vscroll_bar;
    widget_t *hscroll_bar;
} tree_view_t;

/**
 * @method tree_view_create
 * 创建tree_view对象
 * @annotation ["constructor", "scriptable"]
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} 对象。
 */
widget_t *tree_view_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method tree_view_set_item_height
 * 设置列表项的高度。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {int32_t} item_height 列表项的高度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t tree_view_set_item_height(widget_t *widget, int32_t item_height);

/**
 * @method tree_view_set_spacing
 * 设置列表项之间的间距。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {int32_t} spacing 列表项的之间的间距。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t tree_view_set_spacing(widget_t *widget, int32_t spacing);

/**
 * @method tree_view_set_auto_hide_scroll_bar
 * 设置是否自动隐藏滚动条。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} auto_hide_scroll_bar 是否自动隐藏滚动条。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t tree_view_set_auto_hide_scroll_bar(widget_t *widget, bool_t auto_hide_scroll_bar);

/**
 * @method tree_view_cast
 * 转换为tree_view对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget tree_view对象。
 *
 * @return {widget_t*} tree_view对象。
 */
widget_t *tree_view_cast(widget_t *widget);

#define TREE_VIEW(widget) ((tree_view_t*)(tree_view_cast(WIDGET(widget))))

#define WIDGET_TYPE_TREE_VIEW "tree_view"

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(tree_view);

END_C_DECLS

#endif /*TK_TREE_VIEW_H*/
