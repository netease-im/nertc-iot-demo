#include <awtk.h>
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"
#include <widget_animators/widget_animator_move.h>

#define ITEM_PROP_PRESSED "pressed"
#define ITEM_REMOVE_THRESHOLD 50

static ret_t on_item_exit_animate_end(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(ctx);
    widget_destroy(widget->parent);
    return RET_OK;
}

static ret_t on_item_move(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(e->target);
    widget_set_prop_bool(widget, ITEM_PROP_PRESSED, TRUE);
    return RET_OK;
}

static ret_t on_item_pointer_down(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(e->target);
    widget_set_prop_bool(widget, ITEM_PROP_PRESSED, FALSE);
    return RET_OK;
}

static ret_t on_item_pointer_move(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(e->target);
    if (widget_get_prop_bool(widget, ITEM_PROP_PRESSED, FALSE)) {
        return RET_STOP;
    }
    return RET_OK;
}

static ret_t on_item_pointer_up(void *ctx, event_t *e)
{
    pointer_event_t *evt = (pointer_event_t *)e;
    widget_t *widget = WIDGET(e->target);
    draggable_t *draggable = DRAGGABLE(widget_lookup_by_type(widget, WIDGET_TYPE_DRAGGABLE, TRUE));
    bool_t pressed = widget_get_prop_bool(widget, ITEM_PROP_PRESSED, FALSE);
    widget_animator_t *wa;
    return_value_if_fail(draggable != NULL, RET_BAD_PARAMS);

    xy_t dx = draggable->down.x - evt->x;
    if (pressed && dx > ITEM_REMOVE_THRESHOLD) {
        wa = widget_find_animator(widget, "exit");
        if (wa != NULL) {
            widget_animator_move_set_params(wa, widget->x, 0, -widget->w, 0);
            widget_animator_start(wa);
        }
    } else {
        wa = widget_find_animator(widget, "reset");
        if (wa != NULL) {
            widget_animator_move_set_params(wa, widget->x, 0, 0, 0);
            widget_animator_start(wa);
        }
    }

    widget_set_prop_bool(widget, ITEM_PROP_PRESSED, FALSE);

    return RET_OK;
}

static ret_t on_gesture_slide_right(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t visit_init_child(void *ctx, const void *iter)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(iter);
    const char *name = widget->name;

    if (name != NULL && *name != '\0') {
        if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_SLIDE_RIGHT, on_gesture_slide_right, win);
        } else if (tk_str_eq(name, "item_view")) {
            widget_on(widget, EVT_MOVE, on_item_move, widget);
            widget_on(widget, EVT_POINTER_DOWN, on_item_pointer_down, widget);
            widget_on(widget, EVT_POINTER_MOVE, on_item_pointer_move, widget);
            widget_on(widget, EVT_POINTER_UP, on_item_pointer_up, widget);

            widget_animator_t *wa = widget_find_animator(widget, "exit");
            widget_animator_on(wa, EVT_ANIM_END, on_item_exit_animate_end, (void *)widget);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t message_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
