/**
 * File:   tree_item_title.h
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

#ifndef TK_TREE_ITEM_TITLE_H
#define TK_TREE_ITEM_TITLE_H

#include "base/widget.h"

BEGIN_C_DECLS
/**
 * @class tree_item_title_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * TreeView。
 * 在xml中使用"tree\_view"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <tree_item_title text="hello" style="folder"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 *<tree_item_title>
 * <style name="default" border_color="#e9e9e9" text_color="#444444">
 *   <normal bg_color="#e0e0e0" />
 *   <pressed bg_color="#338fff" text_color="#ffffff" />
 *   <over bg_color="#f0f0f0" border_color="#338fff"/>
 * </style>
 *</tree_item_title>
 * ```
 */
typedef struct _tree_item_title_t {
    widget_t widget;

    /*private*/
    bool_t pressed;
} tree_item_title_t;

/**
 * @method tree_item_title_create
 * @annotation ["constructor", "scriptable"]
 * 创建tree_item_title对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} tree_item_title对象。
 */
widget_t *tree_item_title_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method tree_item_title_cast
 * 转换为tree_item_title对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget tree_item_title对象。
 *
 * @return {widget_t*} tree_item_title对象。
 */
widget_t *tree_item_title_cast(widget_t *widget);

#define WIDGET_TYPE_TREE_ITEM_TITLE "tree_item_title"

#define TREE_ITEM_TITLE(widget) ((tree_item_title_t*)(tree_item_title_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(tree_item_title);

#define TREE_ITEM_TITLE_WIDTH 80
#define TREE_ITEM_TITLE_HEIGHT 32

END_C_DECLS

#endif /*TK_TREE_ITEM_TITLE_H*/
