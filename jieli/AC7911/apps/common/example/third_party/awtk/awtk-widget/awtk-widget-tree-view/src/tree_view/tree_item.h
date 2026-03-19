/**
 * File:   tree_item.h
 * Author: AWTK Develop Team
 * Brief:  TreeView。
 *
 * Copyright (c) 2022 - 2022 Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2020-05-31 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_TREE_ITEM_H
#define TK_TREE_ITEM_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class tree_item_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * TreeView。
 * 在xml中使用"tree\_view"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 *     <tree_item value="true">
 *       <tree_item_title text="hello" style="folder"/>
 *       <tree_item_content>
 *         <tree_item_title text="1.world" />
 *         <tree_item_title text="2.world" />
 *         <tree_item_title text="3.world" />
 *       </tree_item_content>
 *     </tree_item>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 *<tree_item>
 * <style name="default" icon_at="left" text_color="#444444" border_color="#d0d0d0">
 *   <normal icon="plus_square" />
 *   <pressed icon="plus_square" />
 *   <over icon="plus_square" />
 *   <focused icon="plus_square" />
 *   <normal_of_checked icon="minus_square" />
 *   <pressed_of_checked icon="minus_square" />
 *   <over_of_checked icon="minus_square" />
 *   <focused_of_checked icon="minus_square" />
 * </style>
 *</tree_item>
 * ```
 */
typedef struct _tree_item_t {
    widget_t widget;
    /**
     * @property {bool_t} value
     * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
     * 值(展开为TRUE，折叠为FALSE)。
     */
    bool_t value;

    /*private*/
    bool_t pressed;
    widget_t *title;
    widget_t *content;
} tree_item_t;

/**
 * @method tree_item_create
 * @annotation ["constructor", "scriptable"]
 * 创建tree_item对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} tree_item对象。
 */
widget_t *tree_item_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method tree_item_cast
 * 转换为tree_item对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget tree_item对象。
 *
 * @return {widget_t*} tree_item对象。
 */
widget_t *tree_item_cast(widget_t *widget);

/**
 * @method tree_item_set_value
 * 设置控件的值。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget tree_item对象。
 * @param {bool_t}  value 值(展开为TRUE，折叠为FALSE)。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t tree_item_set_value(widget_t *widget, bool_t value);

/**
 * @method tree_item_layout
 * 布局。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget tree_item对象。
 * @param {uint32_t} item_height 每一项的高度。
 * @param {uint32_t} gap 每一项的间距。
 * @param {uint32_t} margin 边距。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t tree_item_layout(widget_t *widget, uint32_t item_height, uint32_t gap, uint32_t margin);

#define WIDGET_TYPE_TREE_ITEM "tree_item"

#define TREE_ITEM(widget) ((tree_item_t*)(tree_item_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(tree_item);

END_C_DECLS

#endif /*TK_TREE_ITEM_H*/
