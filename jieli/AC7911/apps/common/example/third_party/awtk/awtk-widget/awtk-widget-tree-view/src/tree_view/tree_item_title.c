/**
 * File:   tree_item_title.c
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
#include "tree_item_title.h"

static ret_t tree_item_title_get_prop(widget_t *widget, const char *name, value_t *v)
{
    tree_item_title_t *tree_item_title = TREE_ITEM_TITLE(widget);
    return_value_if_fail(tree_item_title != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    return RET_NOT_FOUND;
}

static ret_t tree_item_title_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    return RET_NOT_FOUND;
}

static ret_t tree_item_title_on_destroy(widget_t *widget)
{
    tree_item_title_t *tree_item_title = TREE_ITEM_TITLE(widget);
    return_value_if_fail(widget != NULL && tree_item_title != NULL, RET_BAD_PARAMS);

    return RET_OK;
}

static ret_t tree_item_title_on_paint_self(widget_t *widget, canvas_t *c)
{
    return widget_paint_helper(widget, c, NULL, NULL);
}

const char *s_tree_item_title_properties[] = {NULL};

static ret_t tree_item_title_on_event(widget_t *widget, event_t *e)
{
    uint16_t type = e->type;
    tree_item_title_t *tree_item_title = TREE_ITEM_TITLE(widget);
    return_value_if_fail(tree_item_title != NULL, RET_BAD_PARAMS);

    switch (type) {
    case EVT_POINTER_DOWN: {
        tree_item_title->pressed = TRUE;
        widget_set_state(widget, WIDGET_STATE_PRESSED);
        widget_grab(widget->parent, widget);
        break;
    }
    case EVT_POINTER_DOWN_ABORT: {
        tree_item_title->pressed = FALSE;
        widget_ungrab(widget->parent, widget);
        widget_set_state(widget, WIDGET_STATE_NORMAL);
        break;
    }
    case EVT_POINTER_UP: {
        pointer_event_t *evt = (pointer_event_t *)e;
        if (tree_item_title->pressed && widget_is_point_in(widget, evt->x, evt->y, FALSE)) {
            pointer_event_t click;
            widget_dispatch(widget, pointer_event_init(&click, EVT_CLICK, widget, evt->x, evt->y));
        }

        tree_item_title->pressed = FALSE;
        widget_ungrab(widget->parent, widget);
        widget_set_state(widget, WIDGET_STATE_NORMAL);
        break;
    }
    case EVT_POINTER_LEAVE:
        widget_set_state(widget, WIDGET_STATE_NORMAL);
        break;
    case EVT_POINTER_ENTER:
        widget_set_state(widget, WIDGET_STATE_OVER);
        break;
    default:
        break;
    }

    return RET_OK;
}

TK_DECL_VTABLE(tree_item_title) = {.size = sizeof(tree_item_title_t),
                                   .type = WIDGET_TYPE_TREE_ITEM_TITLE,
                                   .clone_properties = s_tree_item_title_properties,
                                   .persistent_properties = s_tree_item_title_properties,
                                   .parent = TK_PARENT_VTABLE(widget),
                                   .create = tree_item_title_create,
                                   .on_paint_self = tree_item_title_on_paint_self,
                                   .set_prop = tree_item_title_set_prop,
                                   .get_prop = tree_item_title_get_prop,
                                   .on_event = tree_item_title_on_event,
                                   .on_destroy = tree_item_title_on_destroy
                                  };

widget_t *tree_item_title_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(tree_item_title), x, y, w, h);
    tree_item_title_t *tree_item_title = TREE_ITEM_TITLE(widget);
    return_value_if_fail(tree_item_title != NULL, NULL);

    return widget;
}

widget_t *tree_item_title_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, tree_item_title), NULL);

    return widget;
}
