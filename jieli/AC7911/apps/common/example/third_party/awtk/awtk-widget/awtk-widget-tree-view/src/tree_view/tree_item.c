/**
 * File:   tree_item.c
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
#include "tree_item.h"
#include "tree_item_title.h"
#include "tree_item_content.h"

static widget_t *tree_item_get_container(widget_t *widget)
{
    widget_t *iter = widget;
    widget_t *win = widget_get_window(widget);

    while (iter != NULL) {
        const char *type = widget_get_type(iter);
        if (iter == win) {
            return iter;
        }

        if (tk_str_eq(type, WIDGET_TYPE_SCROLL_VIEW)) {
            return iter;
        }

        iter = iter->parent;
    }

    return widget;
}

static ret_t tree_item_layout_title(widget_t *widget, uint32_t item_height, uint32_t gap,
                                    uint32_t margin)
{
    if (widget->w == 0) {
        widget->w = TREE_ITEM_TITLE_WIDTH;
    }
    widget->h = item_height;
    widget_layout(widget);

    return RET_OK;
}

static ret_t tree_item_layout_content(widget_t *widget, uint32_t item_height, uint32_t gap,
                                      uint32_t margin)
{
    int32_t w = 0;
    int32_t y = margin;
    const char *type = widget_get_type(widget);

    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
    tree_item_layout(iter, item_height, gap, 0);
    iter->x = margin;
    iter->y = y;
    y += iter->h + gap;
    if (iter->w == 0) {
        iter->w = TREE_ITEM_TITLE_WIDTH;
    }
    w = tk_max_int(w, iter->w);
    WIDGET_FOR_EACH_CHILD_END();

    y = tk_max_int(item_height, y);

    if (tk_str_eq(type, WIDGET_TYPE_TREE_ITEM_CONTENT)) {
        widget->w = w;
        widget->h = y;
    } else if (widget_count_children(widget) == 0) {
        widget->h = item_height;
    }

    return RET_OK;
}

static ret_t tree_item_layout_self(widget_t *widget, uint32_t item_height, uint32_t gap,
                                   uint32_t margin)
{
    widget_t *title = NULL;
    widget_t *content = NULL;
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(tree_item != NULL, RET_BAD_PARAMS);

    return_value_if_fail(tree_item->title != NULL, RET_BAD_PARAMS);

    title = tree_item->title;
    content = tree_item->content;

    tree_item_layout_title(title, item_height, gap, margin);
    title->x = title->h;
    title->y = 0;

    if (content != NULL) {
        widget_set_visible(content, tree_item->value);
        widget_set_sensitive(content, tree_item->value);
    }

    if (tree_item->value && content != NULL) {
        tree_item_layout_content(content, item_height, gap, margin);
        content->x = title->h;
        content->y = title->h + gap;
        widget->h = content->y + content->h;
        widget->w = title->x + tk_max_int(title->w, content->w);
    } else {
        widget->w = title->x + title->w;
        widget->h = title->y + title->h;
    }

    return RET_OK;
}

ret_t tree_item_layout(widget_t *widget, uint32_t item_height, uint32_t gap, uint32_t margin)
{
    const char *type = widget_get_type(widget);
    if (tk_str_eq(type, WIDGET_TYPE_TREE_ITEM)) {
        return tree_item_layout_self(widget, item_height, gap, margin);
    } else if (tk_str_eq(type, WIDGET_TYPE_TREE_ITEM_TITLE)) {
        return tree_item_layout_title(widget, item_height, gap, margin);
    } else {
        return tree_item_layout_content(widget, item_height, gap, margin);
    }

    return RET_OK;
}

static ret_t tree_item_get_prop(widget_t *widget, const char *name, value_t *v)
{
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(tree_item != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, WIDGET_PROP_VALUE)) {
        value_set_bool(v, tree_item->value);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_STATE_FOR_STYLE)) {
        value_set_str(v, widget_get_state_for_style(widget, FALSE, tree_item->value));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t tree_item_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, WIDGET_PROP_VALUE)) {
        return tree_item_set_value(widget, value_bool(v));
    }

    return RET_NOT_FOUND;
}

static ret_t tree_item_on_destroy(widget_t *widget)
{
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(widget != NULL && tree_item != NULL, RET_BAD_PARAMS);

    return RET_OK;
}

static rect_t tree_item_get_icon_rect(tree_item_t *tree_item)
{
    int32_t w = tree_item->title != NULL ? tree_item->title->x : TREE_ITEM_TITLE_HEIGHT;
    int32_t h = tree_item->title != NULL ? tree_item->title->h : TREE_ITEM_TITLE_HEIGHT;

    return rect_init(0, 0, w, h);
}

static ret_t tree_item_on_paint_self(widget_t *widget, canvas_t *c)
{
    tree_item_t *tree_item = TREE_ITEM(widget);
    const char *icon = style_get_str(widget->astyle, STYLE_ID_ICON, NULL);

    if (icon != NULL) {
        bitmap_t img;
        rect_t r = tree_item_get_icon_rect(tree_item);

        if (widget_load_image(widget, icon, &img) == RET_OK) {
            int32_t x = 0;
            int32_t y = 0;
            int32_t w = 0;
            int32_t h = 0;
            int32_t start = 0;
            int32_t img_w = img.w / c->lcd->ratio;
            int32_t img_h = img.h / c->lcd->ratio;
            color_t line_color = color_init(0xff, 0, 0, 0xff);

            canvas_draw_icon_in_rect(c, &img, &r);

            line_color = style_get_color(widget->astyle, STYLE_ID_BORDER_COLOR, line_color);

            if (line_color.rgba.a) {
                canvas_set_stroke_color(c, line_color);

                if (tk_str_eq(widget_get_type(widget->parent), WIDGET_TYPE_TREE_ITEM_CONTENT)) {
                    w = (r.w - img_w) / 2;
                    y = r.h / 2;
                    x = 0;
                    canvas_draw_hline(c, x, y, w);
                }

                /*图标右边的线*/
                x = (r.w - img_w) / 2 + img_w;
                y = r.h / 2;
                w = r.w - x;
                canvas_draw_hline(c, x, y, w);

                x = r.w / 2;
                w = r.w - x;
                start = (r.h - img_h) / 2 + img_h;

                /*到每个子项目的连线*/
                if (tree_item->value && tree_item->content != NULL) {
                    WIDGET_FOR_EACH_CHILD_BEGIN(tree_item->content, iter, i)
                    y = iter->y + iter->h / 2 + tree_item->content->y;
                    if (tk_str_eq(widget_get_type(iter), WIDGET_TYPE_TREE_ITEM)) {
                        r = tree_item_get_icon_rect(tree_item);
                        if (widget_get_value_int(iter)) {
                            y = iter->y + r.h / 2 + tree_item->content->y;
                        }
                    }
                    h = y - start;
                    canvas_draw_vline(c, x, start, h);
                    canvas_draw_hline(c, x, y, w);
                    start = y;
                    WIDGET_FOR_EACH_CHILD_END()
                }
            }
        }
    }

    return RET_OK;
}

static ret_t tree_item_on_event(widget_t *widget, event_t *e)
{
    uint16_t type = e->type;
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(tree_item != NULL, RET_BAD_PARAMS);

    switch (type) {
    case EVT_POINTER_DOWN: {
        tree_item->pressed = TRUE;
        widget_set_state(widget, WIDGET_STATE_PRESSED);
        widget_grab(widget->parent, widget);
        break;
    }
    case EVT_POINTER_DOWN_ABORT: {
        tree_item->pressed = FALSE;
        widget_ungrab(widget->parent, widget);
        widget_set_state(widget, WIDGET_STATE_NORMAL);
        break;
    }
    case EVT_CLICK: {
        tree_item_set_value(widget, !(tree_item->value));
        break;
    }
    case EVT_POINTER_UP: {
        pointer_event_t *evt = (pointer_event_t *)e;
        point_t p = {evt->x, evt->y};
        rect_t r = tree_item_get_icon_rect(tree_item);

        widget_to_local(widget, &p);
        if (tree_item->pressed && rect_contains(&r, p.x, p.y)) {
            pointer_event_t click;
            widget_dispatch(widget, pointer_event_init(&click, EVT_CLICK, widget, evt->x, evt->y));
        }

        tree_item->pressed = FALSE;
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

const char *s_tree_item_properties[] = {NULL};

static ret_t tree_item_on_add_child(widget_t *widget, widget_t *child)
{
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(widget != NULL && tree_item != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(widget_get_type(child), WIDGET_TYPE_TREE_ITEM_TITLE)) {
        assert(tree_item->title == NULL);
        tree_item->title = child;
    } else if (tk_str_eq(widget_get_type(child), WIDGET_TYPE_TREE_ITEM_CONTENT)) {
        assert(tree_item->content == NULL);
        tree_item->content = child;
    } else {
        assert(!"not supported type");
    }

    return RET_FAIL;
}

static ret_t tree_item_on_remove_child(widget_t *widget, widget_t *child)
{
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(widget != NULL && tree_item != NULL, RET_BAD_PARAMS);

    if (tree_item->title == child) {
        tree_item->title = NULL;
    }

    if (tree_item->content == child) {
        tree_item->content = NULL;
    }

    return RET_FAIL;
}

static ret_t tree_item_on_paint_border(widget_t *widget, canvas_t *c)
{
    return RET_OK;
}

TK_DECL_VTABLE(tree_item) = {.size = sizeof(tree_item_t),
                             .type = WIDGET_TYPE_TREE_ITEM,
                             .clone_properties = s_tree_item_properties,
                             .persistent_properties = s_tree_item_properties,
                             .parent = TK_PARENT_VTABLE(widget),
                             .create = tree_item_create,
                             .on_paint_self = tree_item_on_paint_self,
                             .set_prop = tree_item_set_prop,
                             .get_prop = tree_item_get_prop,
                             .on_event = tree_item_on_event,
                             .on_add_child = tree_item_on_add_child,
                             .on_remove_child = tree_item_on_remove_child,
                             .on_paint_border = tree_item_on_paint_border,
                             .on_destroy = tree_item_on_destroy
                            };

widget_t *tree_item_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(tree_item), x, y, w, h);
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(tree_item != NULL, NULL);

    return widget;
}

widget_t *tree_item_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, tree_item), NULL);

    return widget;
}

ret_t tree_item_set_value(widget_t *widget, bool_t value)
{
    tree_item_t *tree_item = TREE_ITEM(widget);
    return_value_if_fail(tree_item != NULL, RET_BAD_PARAMS);

    log_debug("%d => %d\n", tree_item->value, value);
    tree_item->value = value;

    if (widget_is_window_opened(widget)) {
        widget_t *container = tree_item_get_container(widget);
        widget_set_need_relayout(container);
    }

    return RET_OK;
}
