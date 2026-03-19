/**
 * File:   title_value.c
 * Author: AWTK Develop Team
 * Brief:  用于显示标题和值的控件。
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
 * 2020-10-09 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "title_value.h"

static ret_t title_value_get_prop(widget_t *widget, const char *name, value_t *v)
{
    title_value_t *title_value = TITLE_VALUE(widget);
    return_value_if_fail(title_value != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, TITLE_VALUE_PROP_GRAB_FOCUS)) {
        value_set_bool(v, title_value->grab_focus);
        return RET_OK;
    } else if (tk_str_eq(name, TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS)) {
        value_set_bool(v, title_value->return_key_to_grab_focus);
        return RET_OK;
    } else if (tk_str_eq(name, WIDGET_PROP_STATE_FOR_STYLE)) {
        if (!widget->enable) {
            value_set_str(v, WIDGET_STATE_DISABLE);
        } else if (widget->focused) {
            if (title_value->grab_focus) {
                value_set_str(v, TITLE_VALUE_STATE_FOCUS_GRABED);
            } else {
                value_set_str(v, WIDGET_STATE_FOCUSED);
            }
        } else {
            value_set_str(v, WIDGET_STATE_NORMAL);
        }
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t title_value_set_prop(widget_t *widget, const char *name, const value_t *v)
{
    return_value_if_fail(widget != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

    if (tk_str_eq(name, TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS)) {
        title_value_set_return_key_to_grab_focus(widget, value_bool(v));
        return RET_OK;
    } else if (tk_str_eq(name, TITLE_VALUE_PROP_GRAB_FOCUS)) {
        title_value_set_grab_focus(widget, value_bool(v));
        return RET_OK;
    }

    return RET_NOT_FOUND;
}

static ret_t title_value_on_destroy(widget_t *widget)
{
    title_value_t *title_value = TITLE_VALUE(widget);
    return_value_if_fail(widget != NULL && title_value != NULL, RET_BAD_PARAMS);

    return RET_OK;
}

static ret_t title_value_on_paint_self(widget_t *widget, canvas_t *c)
{
    widget_paint_helper(widget, c, NULL, NULL);

    return RET_OK;
}

static widget_t *title_value_get_value_widget(widget_t *widget)
{
    if (widget_count_children(widget) < 1) {
        return NULL;
    }

    return widget_get_child(widget, 0);
}

static ret_t title_value_on_event(widget_t *widget, event_t *e)
{
    ret_t ret = RET_OK;
    title_value_t *title_value = TITLE_VALUE(widget);
    widget_t *value = title_value_get_value_widget(widget);

    switch (e->type) {
    case EVT_KEY_DOWN: {
        if (title_value->grab_focus && value != NULL) {
            widget_dispatch(value, e);
            ret = RET_STOP;
        }
        break;
    }
    case EVT_KEY_UP: {
        key_event_t *evt = (key_event_t *)e;
        if (title_value->return_key_to_grab_focus && key_code_is_enter(evt->key)) {
            title_value_set_grab_focus(widget, !(title_value->grab_focus));
            ret = RET_STOP;
        }
        break;
    }
    }

    return ret;
}

ret_t title_value_set_grab_focus(widget_t *widget, bool_t grab_focus)
{
    title_value_t *title_value = TITLE_VALUE(widget);
    return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

    title_value->grab_focus = grab_focus;
    widget_set_need_update_style(widget);

    return RET_OK;
}

ret_t title_value_set_return_key_to_grab_focus(widget_t *widget, bool_t return_key_to_grab_focus)
{
    title_value_t *title_value = TITLE_VALUE(widget);
    return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

    if (return_key_to_grab_focus) {
        widget->focusable = TRUE;
    }
    title_value->return_key_to_grab_focus = return_key_to_grab_focus;

    return RET_OK;
}

const char *s_title_value_properties[] = {TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS, NULL};

TK_DECL_VTABLE(title_value) = {.size = sizeof(title_value_t),
                               .disallow_children_focusable = TRUE,
                               .type = WIDGET_TYPE_TITLE_VALUE,
                               .clone_properties = s_title_value_properties,
                               .persistent_properties = s_title_value_properties,
                               .parent = TK_PARENT_VTABLE(widget),
                               .create = title_value_create,
                               .on_paint_self = title_value_on_paint_self,
                               .set_prop = title_value_set_prop,
                               .get_prop = title_value_get_prop,
                               .on_event = title_value_on_event,
                               .on_destroy = title_value_on_destroy
                              };

static ret_t title_value_on_key_before_children(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    title_value_t *title_value = TITLE_VALUE(widget);

    if (title_value->return_key_to_grab_focus) {
        widget->target = NULL;
        widget->key_target = NULL;
    }

    return RET_OK;
}

widget_t *title_value_create(widget_t *parent, xy_t x, xy_t y, wh_t w, wh_t h)
{
    widget_t *widget = widget_create(parent, TK_REF_VTABLE(title_value), x, y, w, h);
    title_value_t *title_value = TITLE_VALUE(widget);
    return_value_if_fail(title_value != NULL, NULL);

    widget_on(widget, EVT_KEY_DOWN_BEFORE_CHILDREN, title_value_on_key_before_children, widget);

    return widget;
}

widget_t *title_value_cast(widget_t *widget)
{
    return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, title_value), NULL);

    return widget;
}
