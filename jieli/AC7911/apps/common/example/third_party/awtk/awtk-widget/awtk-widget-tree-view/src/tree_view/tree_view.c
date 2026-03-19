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
 * 2018-07-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/time_now.h"
#include "base/layout.h"

#include "tree_item.h"
#include "tree_view.h"
#include "scroll_view/scroll_bar.h"
#include "scroll_view/scroll_view.h"

static ret_t tree_view_init(widget_t *widget);
#define WIDGET_IS_VBAR(child) (tk_str_eq(child->name, "vbar") || child->w < child->h)

static ret_t tree_view_on_paint_self(widget_t *widget, canvas_t *c)
{
    return widget_paint_helper(widget, c, NULL, NULL);
}

static ret_t tree_view_get_prop(widget_t *widget, const char *name, value_t *v)
{
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, WIDGET_PROP_ITEM_HEIGHT)) {
        value_set_int(v, tree_view->item_height);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_SPACING)) {
        value_set_int(v, tree_view->spacing);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_AUTO_HIDE_SCROLL_BAR)) {
        value_set_bool(v, tree_view->auto_hide_scroll_bar);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t tree_view_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, WIDGET_PROP_ITEM_HEIGHT)) {
        tree_view->item_height = value_int(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_SPACING)) {
        tree_view->spacing = value_int(v);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_AUTO_HIDE_SCROLL_BAR)) {
        tree_view->auto_hide_scroll_bar = value_bool(v);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t tree_view_hanlde_wheel_event(tree_view_t *tree_view, event_t *e)
{
    wheel_event_t *evt = (wheel_event_t *)e;
    int32_t delta = -evt->dy;

    if (tree_view->vscroll_bar != NULL) {
        scroll_bar_add_delta(tree_view->vscroll_bar, delta);
        log_debug("wheel: %d\n", delta);
    }

    return RET_STOP;
}

static ret_t tree_view_on_event(widget_t *widget, event_t *e)
{
    ret_t ret = RET_OK;
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL, RET_BAD_PARAMS);

    switch (e->type) {
    case EVT_WINDOW_WILL_OPEN: {
        tree_view_init(widget);
        break;
    }
    case EVT_WHEEL: {
        ret = tree_view_hanlde_wheel_event(tree_view, e);
        break;
    }
    case EVT_KEY_DOWN: {
        key_event_t *evt = (key_event_t *)e;
        uint32_t item_height = tree_view->item_height;
        if (evt->key == TK_KEY_PAGEDOWN) {
            scroll_view_scroll_delta_to(tree_view->scroll_view, 0, widget->h, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_PAGEUP) {
            scroll_view_scroll_delta_to(tree_view->scroll_view, 0, -widget->h, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_UP) {
            scroll_view_scroll_delta_to(tree_view->scroll_view, 0, -item_height, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_DOWN) {
            scroll_view_scroll_delta_to(tree_view->scroll_view, 0, item_height, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_LEFT) {
            scroll_view_scroll_delta_to(tree_view->scroll_view, -item_height, 0, TK_ANIMATING_TIME);
            ret = RET_STOP;
        } else if (evt->key == TK_KEY_RIGHT) {
            scroll_view_scroll_delta_to(tree_view->scroll_view, item_height, 0, TK_ANIMATING_TIME);
            ret = RET_STOP;
        }
        break;
    }
    default:
        break;
    }
    return ret;
}

TK_DECL_VTABLE(tree_view) = {.type = WIDGET_TYPE_TREE_VIEW,
                             .size = sizeof(tree_view_t),
                             .parent = TK_PARENT_VTABLE(widget),
                             .create = tree_view_create,
                             .set_prop = tree_view_set_prop,
                             .get_prop = tree_view_get_prop,
                             .on_event = tree_view_on_event,
                             .on_paint_self = tree_view_on_paint_self
                            };

static ret_t scroll_bar_set_value_without_notify(widget_t *bar, int32_t value)
{
    if (bar != NULL) {
        emitter_disable(bar->emitter);
        scroll_bar_set_value(bar, value);
        emitter_enable(bar->emitter);
    }

    return RET_OK;
}

static ret_t tree_view_on_scroll_view_scroll(widget_t *widget, int32_t xoffset, int32_t yoffset)
{
    int32_t range = 0;
    int32_t value = 0;
    float_t percent = 0;
    scroll_view_t *scroll_view = NULL;
    scroll_bar_t *vscroll_bar = NULL;
    scroll_bar_t *hscroll_bar = NULL;
    tree_view_t *tree_view = TREE_VIEW(widget->parent);
    return_value_if_fail(tree_view != NULL, RET_BAD_PARAMS);

    scroll_view = SCROLL_VIEW(tree_view->scroll_view);
    return_value_if_fail(scroll_view != NULL, RET_BAD_PARAMS);

    if (tree_view->vscroll_bar != NULL) {
        vscroll_bar = SCROLL_BAR(tree_view->vscroll_bar);
        range = scroll_view->virtual_h - tree_view->scroll_view->h;
        percent = range > 0 ? (float_t)yoffset / (float_t)range : 0;
        value = percent * vscroll_bar->virtual_size;
        scroll_bar_set_value_without_notify(tree_view->vscroll_bar, value);
    }

    if (tree_view->hscroll_bar != NULL) {
        hscroll_bar = SCROLL_BAR(tree_view->hscroll_bar);
        range = scroll_view->virtual_w - tree_view->scroll_view->w;
        percent = range > 0 ? (float_t)xoffset / (float_t)range : 0;
        value = percent * hscroll_bar->virtual_size;
        scroll_bar_set_value_without_notify(tree_view->hscroll_bar, value);
    }

    return RET_OK;
}

static ret_t scroll_bar_to_scroll_view(scroll_bar_t *scroll_bar, scroll_view_t *scroll_view)
{
    int32_t v = scroll_bar->value;
    int32_t range = scroll_bar->virtual_size;
    float_t percent = range > 0 ? (float_t)v / (float_t)(range) : 0;

    if (WIDGET_IS_VBAR(WIDGET(scroll_bar))) {
        int32_t offset = percent * (scroll_view->virtual_h - scroll_view->widget.h);
        scroll_view_set_offset(WIDGET(scroll_view), scroll_view->xoffset, offset);
    } else {
        int32_t offset = percent * (scroll_view->virtual_w - scroll_view->widget.w);
        scroll_view_set_offset(WIDGET(scroll_view), offset, scroll_view->yoffset);
    }

    return RET_OK;
}

static ret_t tree_view_on_scroll_bar_value_changed(void *ctx, event_t *e)
{
    tree_view_t *tree_view = TREE_VIEW(ctx);
    scroll_bar_t *scroll_bar = SCROLL_BAR(e->target);
    scroll_view_t *scroll_view = SCROLL_VIEW(tree_view->scroll_view);
    return_value_if_fail(scroll_view != NULL && scroll_bar != NULL, RET_BAD_PARAMS);

    return scroll_bar_to_scroll_view(scroll_bar, scroll_view);
}

static ret_t tree_view_on_scroll_view_layout_children(widget_t *widget)
{
    int32_t w = 0;
    int32_t h = 0;
    tree_view_t *tree_view = TREE_VIEW(widget->parent);
    int32_t margin = style_get_int(widget->parent->astyle, STYLE_ID_MARGIN, 2);
    return_value_if_fail(tree_view != NULL, RET_BAD_PARAMS);

    tree_item_layout(widget, tree_view->item_height, tree_view->spacing, margin);

    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
    w = tk_max_int(w, iter->x + iter->w);
    h = tk_max_int(h, iter->y + iter->h);
    WIDGET_FOR_EACH_CHILD_END()

    w += margin;
    h += margin;

    scroll_view_set_virtual_w(widget, w);
    scroll_view_set_virtual_h(widget, h);

    if (tree_view->vscroll_bar != NULL) {
        scroll_bar_set_params(tree_view->vscroll_bar, h, 30);
    }

    if (tree_view->hscroll_bar != NULL) {
        scroll_bar_set_params(tree_view->hscroll_bar, w, 30);
    }

    if (widget->h >= h && tree_view->vscroll_bar != NULL) {
        scroll_bar_set_value(tree_view->vscroll_bar, 0);
    }
    if (widget->w >= w && tree_view->hscroll_bar != NULL) {
        scroll_bar_set_value(tree_view->hscroll_bar, 0);
    }

    if (tree_view->auto_hide_scroll_bar) {
        bool_t v = widget->h < h;
        bool_t h = widget->w < w;
        widget_t *tree_view_widget = WIDGET(tree_view);
        widget_t *vscroll_bar = tree_view->vscroll_bar;
        widget_t *hscroll_bar = tree_view->hscroll_bar;

        if (vscroll_bar != NULL) {
            widget_set_visible(vscroll_bar, v);
            widget_set_enable(vscroll_bar, v);
        }

        if (hscroll_bar != NULL) {
            widget_set_visible(hscroll_bar, h);
            widget_set_enable(hscroll_bar, h);
        }

        if (v != h) {
            if (v && vscroll_bar != NULL) {
                widget_resize(vscroll_bar, vscroll_bar->w, tree_view_widget->h);
            }

            if (h && hscroll_bar != NULL) {
                widget_resize(hscroll_bar, tree_view_widget->w, hscroll_bar->h);
            }
        } else if (vscroll_bar != NULL && hscroll_bar != NULL) {
            widget_resize(vscroll_bar, vscroll_bar->w, tree_view_widget->h - hscroll_bar->h);
            widget_resize(hscroll_bar, tree_view_widget->w - vscroll_bar->w, hscroll_bar->h);
        }
    }

    return RET_OK;
}

static ret_t tree_view_on_scroll_view_paint_children(widget_t *widget, canvas_t *c)
{
    int32_t left = 0;
    int32_t top = 0;
    int32_t bottom = 0;
    int32_t right = 0;
    int32_t max_w = canvas_get_width(c);
    int32_t max_h = canvas_get_height(c);

    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)

    if (!iter->visible) {
        iter->dirty = FALSE;
        continue;
    }

    left = c->ox + iter->x;
    top = c->oy + iter->y;
    bottom = top + iter->h;
    right = left + iter->w;

    if (top > max_h || left > max_w) {
        break;
    }

    if (bottom < 0 || right < 0) {
        iter->dirty = FALSE;
        continue;
    }

    if (!canvas_is_rect_in_clip_rect(c, left, top, right, bottom)) {
        iter->dirty = FALSE;
        continue;
    }

    widget_paint(iter, c);
    WIDGET_FOR_EACH_CHILD_END();

    return RET_OK;
}

static ret_t tree_view_init(widget_t *widget)
{
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL && widget != NULL, RET_BAD_PARAMS);

    if (tree_view->vscroll_bar != NULL) {
        widget_off_by_ctx(tree_view->vscroll_bar, widget);
        tree_view->vscroll_bar = NULL;
    }

    if (tree_view->hscroll_bar != NULL) {
        widget_off_by_ctx(tree_view->hscroll_bar, widget);
        tree_view->hscroll_bar = NULL;
    }

    if (tree_view->scroll_view != NULL) {
        scroll_view_t *scroll_view = SCROLL_VIEW(tree_view->scroll_view);
        scroll_view->on_scroll = NULL;
        scroll_view->on_layout_children = NULL;
        scroll_view->on_paint_children = NULL;
    }

    WIDGET_FOR_EACH_CHILD_BEGIN(widget, child, i)
    const char *type = widget_get_type(child);
    if (tk_str_eq(type, WIDGET_TYPE_SCROLL_VIEW)) {
        scroll_view_t *scroll_view = SCROLL_VIEW(child);
        tree_view->scroll_view = child;
        scroll_view->on_scroll = tree_view_on_scroll_view_scroll;
        scroll_view->on_layout_children = tree_view_on_scroll_view_layout_children;
        scroll_view->on_paint_children = tree_view_on_scroll_view_paint_children;
        scroll_view_set_recursive_only(child, FALSE);
    } else if (tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR) ||
               tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR_DESKTOP) ||
               tk_str_eq(type, WIDGET_TYPE_SCROLL_BAR_MOBILE)) {
        if (WIDGET_IS_VBAR(child)) {
            tree_view->vscroll_bar = child;
        } else {
            tree_view->hscroll_bar = child;
        }
        widget_on(child, EVT_VALUE_CHANGED, tree_view_on_scroll_bar_value_changed, widget);
    }
    WIDGET_FOR_EACH_CHILD_END();

    tree_view_on_scroll_view_layout_children(tree_view->scroll_view);

    return RET_CONTINUE;
}

widget_t *tree_view_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    return widget_create(parent, TK_REF_VTABLE(tree_view), x, y, w, h);
}

ret_t tree_view_set_item_height(widget_t *widget, int32_t item_height)
{
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL, RET_BAD_PARAMS);

    tree_view->item_height = item_height;

    return RET_OK;
}

ret_t tree_view_set_spacing(widget_t *widget, int32_t spacing)
{
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL, RET_BAD_PARAMS);

    tree_view->spacing = spacing;

    return RET_OK;
}

ret_t tree_view_set_auto_hide_scroll_bar(widget_t *widget, bool_t auto_hide_scroll_bar)
{
    tree_view_t *tree_view = TREE_VIEW(widget);
    return_value_if_fail(tree_view != NULL, RET_BAD_PARAMS);

    tree_view->auto_hide_scroll_bar = auto_hide_scroll_bar;

    return RET_OK;
}

widget_t *tree_view_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, tree_view), NULL);

    return widget;
}
