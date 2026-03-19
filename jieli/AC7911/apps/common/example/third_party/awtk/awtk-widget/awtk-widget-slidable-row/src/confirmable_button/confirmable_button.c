/**
 * File:   confirmable_button.c
 * Author: AWTK Develop Team
 * Brief:  可确认的按钮。点击按钮时，弹出一个窗口，再次点击时，才触发CLICK事件。
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
 * 2020-07-30 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "widgets/popup.h"
#include "confirmable_button.h"
#include "base/widget_vtable.h"
#include "widget_animators/widget_animator_prop2.h"

ret_t confirmable_button_set_popup_win_w(widget_t *widget, int32_t popup_win_w)
{
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    return_value_if_fail(confirmable_button != NULL, RET_BAD_PARAMS);

    confirmable_button->popup_win_w = popup_win_w;

    return RET_OK;
}

ret_t confirmable_button_set_popup_to_right(widget_t *widget, bool_t popup_to_right)
{
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    return_value_if_fail(confirmable_button != NULL, RET_BAD_PARAMS);

    confirmable_button->popup_to_right = popup_to_right;

    return RET_OK;
}

static ret_t confirmable_button_get_prop(widget_t *widget, const char *name, value_t *v)
{
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    return_value_if_fail(confirmable_button != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(CONFIRMABLE_BUTTON_PROP_POPUP_WIN_W, name)) {
        value_set_int32(v, confirmable_button->popup_win_w);
        return RET_OK;
    } else if (tk_str_eq(CONFIRMABLE_BUTTON_PROP_POPUP_TO_RIGHT, name)) {
        value_set_bool(v, confirmable_button->popup_to_right);
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t confirmable_button_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(CONFIRMABLE_BUTTON_PROP_POPUP_WIN_W, name)) {
        confirmable_button_set_popup_win_w(widget, value_int32(v));
        return RET_OK;
    } else if (tk_str_eq(CONFIRMABLE_BUTTON_PROP_POPUP_TO_RIGHT, name)) {
        confirmable_button_set_popup_to_right(widget, value_bool(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t confirmable_button_on_destroy(widget_t *widget)
{
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    return_value_if_fail(widget != NULL && confirmable_button != NULL, RET_BAD_PARAMS);

    confirmable_button->in_popup = FALSE;

    return RET_OK;
}

static ret_t confirmable_button_on_layout_children(widget_t *widget)
{
    WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
    iter->x = 0;
    iter->y = 0;
    iter->w = widget->w;
    iter->h = widget->h;
    widget_layout(iter);
    WIDGET_FOR_EACH_CHILD_END()

    return RET_OK;
}

static ret_t popup_on_resize(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    prop_change_event_t *evt = prop_change_event_cast(e);

    if (tk_str_eq(evt->name, WIDGET_PROP_W)) {
        widget_layout_children(widget);
    }

    return RET_OK;
}

static ret_t confirmable_button_on_clicked(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    pointer_event_t click = *pointer_event_cast(e);

    click.e.target = ctx;
    widget_dispatch(widget, (event_t *)&click);

    return RET_OK;
}

static ret_t confirmable_button_confirm(widget_t *widget)
{
    point_t p = {0, 0};
    widget_t *popup = NULL;
    widget_t *button_in_popup = NULL;
    widget_t *button = widget;
    widget_t *win = widget_get_window(widget);
    const char *theme = widget_get_prop_str(win, WIDGET_PROP_THEME, NULL);
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    int32_t popup_w = tk_max(confirmable_button->popup_win_w, widget->w);

    widget_to_screen(widget, &p);
    popup = popup_create(NULL, p.x, p.y, widget->w, widget->h);
    return_value_if_fail(popup != NULL, RET_OOM);

    widget_set_prop_str(popup, WIDGET_PROP_THEME, theme  != NULL ? theme : win->name);
    popup_set_close_when_click(popup, TRUE);
    button_in_popup = widget_clone(button, popup);
    widget_set_self_layout_params(button_in_popup, "0", "0", "100%", "100%");
    widget_use_style(button_in_popup, button->style);
    widget_layout(popup);

    CONFIRMABLE_BUTTON(button_in_popup)->in_popup = TRUE;
    widget_on(button_in_popup, EVT_CLICK, confirmable_button_on_clicked, button);

    if (popup_w != popup->w) {
        int32_t end_x = 0;
        int32_t end_w = popup_w;
        widget_animator_t *wa = widget_animator_prop2_create(
                                    popup, TK_ANIMATING_TIME, 0, EASING_SIN_INOUT, WIDGET_PROP_X, WIDGET_PROP_W);
        return_value_if_fail(wa != NULL, RET_OOM);
        if (confirmable_button->popup_to_right) {
            end_x = popup->x;
        } else {
            end_x = popup->x - popup_w + popup->w;
        }

        widget_on(popup, EVT_PROP_CHANGED, popup_on_resize, popup);
        widget_animator_prop2_set_params(wa, popup->x, popup->w, end_x, end_w);
        widget_animator_start(wa);
    }

    return RET_OK;
}

static ret_t confirmable_button_on_event(widget_t *widget, event_t *e)
{
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    return_value_if_fail(widget != NULL && confirmable_button != NULL, RET_BAD_PARAMS);

    switch (e->type) {
    case EVT_POINTER_DOWN: {
        pointer_event_t *evt = pointer_event_cast(e);
        confirmable_button->pressed = TRUE;
        confirmable_button->down.x = evt->x;
        confirmable_button->down.y = evt->y;
        return RET_STOP;
    }
    case EVT_POINTER_DOWN_ABORT: {
        confirmable_button->pressed = FALSE;
        break;
    }
    case EVT_POINTER_UP: {
        pointer_event_t *evt = pointer_event_cast(e);
        int dx = confirmable_button->down.x - evt->x;
        int dy = confirmable_button->down.y - evt->y;

        if (confirmable_button->pressed && dx < TK_DRAG_THRESHOLD && dy < TK_DRAG_THRESHOLD) {
            if (confirmable_button->in_popup) {
                pointer_event_t evt = *(pointer_event_t *)e;
                evt.e = event_init(EVT_CLICK, widget);
                evt.e.size = sizeof(pointer_event_t);
                widget_dispatch(widget, (event_t *)&evt);
            } else {
                confirmable_button_confirm(widget);
                return RET_STOP;
            }
        }
    }
    }
    (void)confirmable_button;

    return RET_OK;
}

const char *s_confirmable_button_properties[] = {CONFIRMABLE_BUTTON_PROP_POPUP_WIN_W,
                                                 CONFIRMABLE_BUTTON_PROP_POPUP_TO_RIGHT, NULL
                                                };

TK_DECL_VTABLE(confirmable_button) = {.size = sizeof(confirmable_button_t),
                                      .type = WIDGET_TYPE_CONFIRMABLE_BUTTON,
                                      .clone_properties = s_confirmable_button_properties,
                                      .persistent_properties = s_confirmable_button_properties,
                                      .parent = TK_PARENT_VTABLE(widget),
                                      .create = confirmable_button_create,
                                      .on_paint_self = widget_on_paint_self_default,
                                      .on_layout_children = confirmable_button_on_layout_children,
                                      .set_prop = confirmable_button_set_prop,
                                      .get_prop = confirmable_button_get_prop,
                                      .on_event = confirmable_button_on_event,
                                      .on_destroy = confirmable_button_on_destroy
                                     };

widget_t *confirmable_button_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(confirmable_button), x, y, w, h);
    confirmable_button_t *confirmable_button = CONFIRMABLE_BUTTON(widget);
    return_value_if_fail(confirmable_button != NULL, NULL);

    confirmable_button->in_popup = FALSE;
    confirmable_button->popup_to_right = FALSE;

    return widget;
}

widget_t *confirmable_button_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, confirmable_button), NULL);

    return widget;
}
