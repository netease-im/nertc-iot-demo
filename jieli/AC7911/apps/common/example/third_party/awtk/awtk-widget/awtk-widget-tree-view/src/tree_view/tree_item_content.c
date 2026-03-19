/**
 * File:   tree_item_content.c
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
#include "tree_item_content.h"

static ret_t tree_item_content_get_prop(widget_t *widget, const char *name, value_t *v)
{
    tree_item_content_t *tree_item_content = TREE_ITEM_CONTENT(widget);
    return_value_if_fail(tree_item_content != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    return RET_NOT_FOUND;
}

static ret_t tree_item_content_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    tree_item_content_t *tree_item_content = TREE_ITEM_CONTENT(widget);
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    return RET_NOT_FOUND;
}

static ret_t tree_item_content_on_destroy(widget_t *widget)
{
    tree_item_content_t *tree_item_content = TREE_ITEM_CONTENT(widget);
    return_value_if_fail(widget != NULL && tree_item_content != NULL, RET_BAD_PARAMS);

    return RET_OK;
}

static ret_t tree_item_content_on_paint_self(widget_t *widget, canvas_t *c)
{
    tree_item_content_t *tree_item_content = TREE_ITEM_CONTENT(widget);

    (void)tree_item_content;

    return RET_OK;
}

static ret_t tree_item_content_on_event(widget_t *widget, event_t *e)
{
    tree_item_content_t *tree_item_content = TREE_ITEM_CONTENT(widget);
    return_value_if_fail(widget != NULL && tree_item_content != NULL, RET_BAD_PARAMS);

    (void)tree_item_content;

    return RET_OK;
}

const char *s_tree_item_content_properties[] = {NULL};

TK_DECL_VTABLE(tree_item_content) = {.size = sizeof(tree_item_content_t),
                                     .type = WIDGET_TYPE_TREE_ITEM_CONTENT,
                                     .clone_properties = s_tree_item_content_properties,
                                     .persistent_properties = s_tree_item_content_properties,
                                     .parent = TK_PARENT_VTABLE(widget),
                                     .create = tree_item_content_create,
                                     .on_paint_self = tree_item_content_on_paint_self,
                                     .set_prop = tree_item_content_set_prop,
                                     .get_prop = tree_item_content_get_prop,
                                     .on_event = tree_item_content_on_event,
                                     .on_destroy = tree_item_content_on_destroy
                                    };

widget_t *tree_item_content_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(tree_item_content), x, y, w, h);
    tree_item_content_t *tree_item_content = TREE_ITEM_CONTENT(widget);
    return_value_if_fail(tree_item_content != NULL, NULL);

    return widget;
}

widget_t *tree_item_content_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, tree_item_content), NULL);

    return widget;
}
