/**
 * File:   tree_item_content.h
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

#ifndef TK_TREE_ITEM_CONTENT_H
#define TK_TREE_ITEM_CONTENT_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class tree_item_content_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * TreeView。
 * 在xml中使用"tree\_view"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 *  <tree_item_content>
 *    <tree_item_title text="1.world" />
 *    <tree_item_title text="2.world" />
 *    <tree_item_title text="3.world" />
 *  </tree_item_content>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <tree_item_content>
 *   <style name="default" font_size="32">
 *     <normal text_color="black" />
 *   </style>
 * </tree_item_content>
 * ```
 */
typedef struct _tree_item_content_t {
    widget_t widget;

} tree_item_content_t;

/**
 * @method tree_item_content_create
 * @annotation ["constructor", "scriptable"]
 * 创建tree_item_content对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} tree_item_content对象。
 */
widget_t *tree_item_content_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method tree_item_content_cast
 * 转换为tree_item_content对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget tree_item_content对象。
 *
 * @return {widget_t*} tree_item_content对象。
 */
widget_t *tree_item_content_cast(widget_t *widget);

#define WIDGET_TYPE_TREE_ITEM_CONTENT "tree_item_content"

#define TREE_ITEM_CONTENT(widget) ((tree_item_content_t*)(tree_item_content_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(tree_item_content);

END_C_DECLS

#endif /*TK_TREE_ITEM_CONTENT_H*/
