/**
 * File:   slidable_row.c
 * Author: AWTK Develop Team
 * Brief:  slidable row(仿微信列表项目，像左滑动显示“删除”按钮)。
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
 * 2020-07-28 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "slidable_row.h"
#include "widget_animators/widget_animator_prop.h"

static int32_t slidable_row_get_max_xoffset(widget_t *widget);
static ret_t slidable_row_scroll_to_xoffset(widget_t *widget, int32_t xoffset);

ret_t slidable_row_set_xoffset(widget_t *widget, int32_t xoffset)
{
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    return_value_if_fail(slidable_row != NULL, RET_BAD_PARAMS);

    slidable_row->xoffset = xoffset;
    widget_layout_children(widget);

    return RET_OK;
}

static ret_t slidable_row_get_prop(widget_t *widget, const char *name, value_t *v)
{
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    return_value_if_fail(slidable_row != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(SLIDABLE_ROW_PROP_XOFFSET, name)) {
        value_set_int32(v, slidable_row->xoffset);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t slidable_row_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(SLIDABLE_ROW_PROP_XOFFSET, name)) {
        slidable_row_set_xoffset(widget, value_int32(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t slidable_row_on_detach_parent(widget_t *widget, widget_t *parent)
{
    widget_t *win = widget_get_window(parent);
    if (win != NULL) {
        widget_off_by_ctx(win, widget);
    }

    return RET_OK;
}

static ret_t slidable_row_on_destroy(widget_t *widget)
{
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    return_value_if_fail(widget != NULL && slidable_row != NULL, RET_BAD_PARAMS);

    return RET_OK;
}

static ret_t slidable_row_on_pointer_down_abort(widget_t *widget, event_t *e)
{
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);

    slidable_row->pressed = FALSE;
    slidable_row->dragged = FALSE;
    widget_ungrab(widget->parent, widget);
    if (slidable_row->expaneded) {
        int32_t max_xoffset = slidable_row_get_max_xoffset(widget);
        slidable_row_set_xoffset(widget, max_xoffset);
    } else {
        slidable_row_set_xoffset(widget, 0);
    }

    return RET_OK;
}

static ret_t slidable_row_on_pointer_down(widget_t *widget, event_t *e)
{
    pointer_event_t *evt = (pointer_event_t *)e;
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);

    slidable_row->pressed = TRUE;
    slidable_row->dragged = FALSE;
    slidable_row->down.x = evt->x;
    slidable_row->down.y = evt->y;

    widget_grab(widget->parent, widget);

    return RET_OK;
}

static ret_t slidable_row_on_pointer_move(widget_t *widget, event_t *e)
{
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    int32_t max_xoffset = slidable_row_get_max_xoffset(widget);
    pointer_event_t *evt = (pointer_event_t *)e;
    xy_t dx = slidable_row->down.x - evt->x;

    if (tk_abs(dx) > TK_DRAG_THRESHOLD) {
        slidable_row->dragged = TRUE;
    }

    if (slidable_row->expaneded) {
        if (dx < 0) {
            slidable_row_set_xoffset(widget, max_xoffset + dx);
            widget_layout_children(widget);
        } else {
            slidable_row_set_xoffset(widget, max_xoffset);
        }
    } else {
        if (dx > 0) {
            dx = tk_min(dx, max_xoffset);
            slidable_row_set_xoffset(widget, dx);
            widget_layout_children(widget);
        } else {
            slidable_row_set_xoffset(widget, 0);
        }
    }

    return RET_OK;
}

static ret_t slidable_row_on_scroll_done(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    slidable_row_t *slidable_row = SLIDABLE_ROW(ctx);
    return_value_if_fail(widget != NULL && slidable_row != NULL, RET_BAD_PARAMS);

    slidable_row->wa = NULL;
    slidable_row->pressed = FALSE;
    slidable_row->dragged = FALSE;

    return RET_REMOVE;
}

static ret_t slidable_row_on_parent_pointer_down(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    widget_t *parent = widget->parent;
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);

    if (parent->target != widget) {
        slidable_row->expaneded = FALSE;
        slidable_row_scroll_to_xoffset(widget, 0);

        return RET_REMOVE;
    }

    return RET_OK;
}

static ret_t slidable_row_scroll_to_xoffset(widget_t *widget, int32_t xoffset)
{
    widget_t *win = widget_get_window(widget);
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    int32_t max_xoffset = slidable_row_get_max_xoffset(widget);

    slidable_row->expaneded = xoffset == max_xoffset;
    if (slidable_row->expaneded) {
        widget_on(win, EVT_POINTER_DOWN, slidable_row_on_parent_pointer_down, widget);
    }

    if (xoffset == slidable_row->xoffset) {
        return RET_OK;
    }

    slidable_row->wa = widget_animator_prop_create(widget, TK_ANIMATING_TIME, 0, EASING_SIN_INOUT, SLIDABLE_ROW_PROP_XOFFSET);
    return_value_if_fail(slidable_row->wa != NULL, RET_OOM);

    widget_animator_prop_set_params(slidable_row->wa, slidable_row->xoffset, xoffset);
    widget_animator_on(slidable_row->wa, EVT_ANIM_END, slidable_row_on_scroll_done, slidable_row);
    widget_animator_start(slidable_row->wa);

    return RET_OK;
}

static ret_t slidable_row_on_pointer_up(widget_t *widget, event_t *e)
{
    int32_t xoffset_end = 0;
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    int32_t max_xoffset = slidable_row_get_max_xoffset(widget);
    pointer_event_t *evt = (pointer_event_t *)e;
    xy_t dx = slidable_row->down.x - evt->x;

    slidable_row->dragged = FALSE;
    slidable_row->pressed = FALSE;
    widget_ungrab(widget->parent, widget);

    if (dx == 0) {
        if (slidable_row->expaneded) {
            slidable_row_scroll_to_xoffset(widget, 0);
        }
        return RET_OK;
    }

    if (slidable_row->expaneded) {
        if (slidable_row->xoffset > (0.8f * max_xoffset)) {
            xoffset_end = max_xoffset;
        } else {
            xoffset_end = 0;
        }
    } else {
        if (slidable_row->xoffset > (0.2f * max_xoffset)) {
            xoffset_end = max_xoffset;
        } else {
            xoffset_end = 0;
        }
    }

    return slidable_row_scroll_to_xoffset(widget, xoffset_end);
}

static ret_t slidable_row_on_event(widget_t *widget, event_t *e)
{
    ret_t ret = RET_OK;
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    return_value_if_fail(widget != NULL && slidable_row != NULL, RET_BAD_PARAMS);

    switch (e->type) {
    case EVT_RESET: {
        slidable_row->pressed = FALSE;
        slidable_row->dragged = FALSE;
        slidable_row->expaneded = FALSE;
        slidable_row_set_xoffset(widget, 0);
        widget_ungrab(widget->parent, widget);
        return RET_OK;
    }
    case EVT_POINTER_DOWN: {
        if (slidable_row->wa != NULL) {
            return RET_OK;
        }

        slidable_row_on_pointer_down(widget, e);
        break;
    }
    case EVT_POINTER_DOWN_ABORT: {
        slidable_row_on_pointer_down_abort(widget, e);
        break;
    }
    case EVT_POINTER_MOVE: {
        if (slidable_row->wa != NULL) {
            return RET_OK;
        }

        if (slidable_row->pressed) {
            slidable_row_on_pointer_move(widget, e);
            ret = slidable_row->dragged ? RET_STOP : RET_OK;
        }
        break;
    }
    case EVT_POINTER_UP: {
        if (slidable_row->wa != NULL) {
            return RET_STOP;
        }

        ret = slidable_row->dragged ? RET_STOP : RET_OK;
        slidable_row_on_pointer_up(widget, e);
        break;
    }
    default:
        break;
    }

    return ret;
}

const char *s_slidable_row_properties[] = {SLIDABLE_ROW_PROP_XOFFSET, NULL};

static int32_t slidable_row_get_expaneded_x(widget_t *widget, widget_t *child)
{
    int32_t x = 0;
    bool_t meet = FALSE;
    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
    if (iter == child) {
        meet = TRUE;
    }
    if (meet) {
        x += iter->w;
    }
    WIDGET_FOR_EACH_CHILD_END()

    return x;
}

static int32_t slidable_row_get_max_xoffset(widget_t *widget)
{
    int32_t max_xoffset = 0;
    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
    if (i > 0) {
        max_xoffset += iter->w;
    }
    WIDGET_FOR_EACH_CHILD_END()

    return max_xoffset;
}

static ret_t slidable_row_on_layout_children(widget_t *widget)
{
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    int32_t max_xoffset = slidable_row_get_max_xoffset(widget);
    double percent = (double)(slidable_row->xoffset) / (double)max_xoffset;

    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
    int32_t x = 0;
    if (i == 0) {
        iter->w = widget->w;
        x = -slidable_row->xoffset;
    } else {
        int32_t expanded_x = slidable_row_get_expaneded_x(widget, iter);
        x = widget->w - expanded_x * percent;
    }

    widget_move_resize(iter, x, 0, iter->w, widget->h);
    widget_layout(iter);
    WIDGET_FOR_EACH_CHILD_END()

    return RET_OK;
}

TK_DECL_VTABLE(slidable_row) = {.size = sizeof(slidable_row_t),
                                .type = WIDGET_TYPE_SLIDABLE_ROW,
                                .clone_properties = s_slidable_row_properties,
                                .persistent_properties = s_slidable_row_properties,
                                .parent = TK_PARENT_VTABLE(widget),
                                .create = slidable_row_create,
                                .set_prop = slidable_row_set_prop,
                                .get_prop = slidable_row_get_prop,
                                .on_event = slidable_row_on_event,
                                .on_detach_parent = slidable_row_on_detach_parent,
                                .on_layout_children = slidable_row_on_layout_children,
                                .on_destroy = slidable_row_on_destroy
                               };

widget_t *slidable_row_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(slidable_row), x, y, w, h);
    slidable_row_t *slidable_row = SLIDABLE_ROW(widget);
    return_value_if_fail(slidable_row != NULL, NULL);

    return widget;
}

widget_t *slidable_row_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, slidable_row), NULL);

    return widget;
}
