/**
 * File:   tree_view.c
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

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tree_view_register.h"
#include "base/widget_factory.h"
#include "tree_view/tree_view.h"
#include "tree_view/tree_item.h"
#include "tree_view/tree_item_title.h"
#include "tree_view/tree_item_content.h"

ret_t tree_view_register(void)
{
    widget_factory_register(widget_factory(), WIDGET_TYPE_TREE_VIEW, tree_view_create);
    widget_factory_register(widget_factory(), WIDGET_TYPE_TREE_ITEM, tree_item_create);
    widget_factory_register(widget_factory(), WIDGET_TYPE_TREE_ITEM_TITLE, tree_item_title_create);
    return widget_factory_register(widget_factory(), WIDGET_TYPE_TREE_ITEM_CONTENT,
                                   tree_item_content_create);
}

const char *tree_view_supported_render_mode(void)
{
    return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
