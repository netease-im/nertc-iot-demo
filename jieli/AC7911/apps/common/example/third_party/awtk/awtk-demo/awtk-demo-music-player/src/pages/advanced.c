#include "awtk.h"
#include "../common/navigator.h"
#include "../common/knob_function.h"

#define BUFF_LEN 32

static bool_t s_is_down_dialog;
static int32_t g_evt_pointer_up_num;
static int32_t g_evt_pointer_down_num;

static ret_t on_close(void *ctx, event_t *e)
{
    widget_t *dialog = (widget_t *)ctx;
    (void)e;

    return dialog_quit(dialog, RET_QUIT);
}

static ret_t on_quit_anim(void *ctx, event_t *e)
{
    widget_t *dialog = (widget_t *)ctx;
    widget_animator_t *animator = NULL;
    (void)e;
    widget_t *wm = window_manager();

    if (wm->w == 480) {
        widget_create_animator(dialog, "move(x_from=240, x_to=480, duration=500)");
    } else {
        widget_create_animator(dialog, "move(x_from=400, x_to=800, duration=500)");
    }
    animator = widget_find_animator(dialog, "move");
    widget_animator_on(animator, EVT_ANIM_END, on_close, dialog);
    widget_off(dialog, g_evt_pointer_up_num);
    widget_off(dialog, g_evt_pointer_down_num);

    return RET_OK;
}

static ret_t on_dialog_state(void *ctx, event_t *e)
{
    widget_t *dialog = (widget_t *)ctx;
    pointer_event_t evt = *(pointer_event_t *)e;
    if (e->type == EVT_POINTER_DOWN) {
        if (evt.x > (dialog->x + dialog->w) || evt.x < dialog->x || evt.y < dialog->y ||
            evt.y > (dialog->y + dialog->h)) {
            s_is_down_dialog = TRUE;
        }
    } else if (s_is_down_dialog) {
        widget_t *close = widget_lookup(dialog, "close", TRUE);
        if (evt.x > (dialog->x + dialog->w) || evt.x < dialog->x || evt.y < dialog->y ||
            evt.y > (dialog->y + dialog->h)) {
            evt.e = event_init(EVT_CLICK, close);
            evt.e.size = sizeof(evt);
            widget_dispatch(close, (event_t *)&evt);
            widget_invalidate(close, NULL);
        }
    }
    return RET_OK;
}

static ret_t on_guage_pointer_leave(void *ctx, event_t *e)
{
    widget_t *widget_guage_pointer = WIDGET(ctx);
    value_t val;

    value_set_bool(&val, FALSE);
    widget_set_prop(widget_guage_pointer, "s_pmove", &val);

    return RET_OK;
}

static ret_t on_guage_pointer_up(void *ctx, event_t *e)
{
    widget_t *widget_guage_pointer = WIDGET(ctx);
    value_t val;

    widget_ungrab(widget_guage_pointer->parent, widget_guage_pointer);
    value_set_bool(&val, FALSE);
    widget_set_prop(widget_guage_pointer, "s_pmove", &val);

    return RET_OK;
}

static ret_t on_guage_pointer_move(void *ctx, event_t *e)
{
    char buf_cir[BUFF_LEN] = {0};
    pointer_event_t *evt = (pointer_event_t *)e;
    point_t p = {evt->x, evt->y};
    widget_t *widget_guage_pointer = WIDGET(ctx);
    widget_t *win = widget_get_window(widget_guage_pointer);

    sprintf(buf_cir, "circle_%s", strtok(widget_guage_pointer->name, "_"));
    widget_t *circle = widget_lookup(win, buf_cir, TRUE);
    widget_to_local(win, &p);
    if (evt->pressed) {
        knob_angle_change(circle, widget_guage_pointer, p);
    }

    return RET_OK;
}

/**
 * 记录指针down时产生的初始数值
 */
static ret_t on_guage_pointer_down(void *ctx, event_t *e)
{
    widget_t *widget_guage_pointer = WIDGET(ctx);
    widget_t *win = widget_get_window(widget_guage_pointer);
    pointer_event_t *evt = (pointer_event_t *)e;
    point_t p = {evt->x, evt->y};
    widget_grab(widget_guage_pointer->parent, widget_guage_pointer);
    widget_to_local(win, &p);
    knob_down_init(widget_guage_pointer, p, strtok(widget_guage_pointer->name, "_"));

    return RET_OK;
}

/**
 * 初始化窗口的子控件
 */
static ret_t visit_init_child(void *ctx, const void *iter)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(iter);
    const char *name = widget->name;

    // 初始化指定名称的控件（设置属性或注册事件），请保证控件名称在窗口上唯一
    if (name != NULL && *name != '\0') {
        if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_quit_anim, win);
        } else if (tk_str_eq(name, "low_guage_pointer")) {
            widget_on(widget, EVT_POINTER_DOWN, on_guage_pointer_down, widget);
            widget_on(widget, EVT_POINTER_MOVE, on_guage_pointer_move, widget);
            widget_on(widget, EVT_POINTER_UP, on_guage_pointer_up, widget);
            widget_on(widget, EVT_POINTER_LEAVE, on_guage_pointer_leave, widget);
        } else if (tk_str_eq(name, "high_guage_pointer")) {
            widget_on(widget, EVT_POINTER_DOWN, on_guage_pointer_down, widget);
            widget_on(widget, EVT_POINTER_MOVE, on_guage_pointer_move, widget);
            widget_on(widget, EVT_POINTER_UP, on_guage_pointer_up, widget);
            widget_on(widget, EVT_POINTER_LEAVE, on_guage_pointer_leave, widget);
        } else if (tk_str_eq(name, "round_guage_pointer")) {
            widget_on(widget, EVT_POINTER_DOWN, on_guage_pointer_down, widget);
            widget_on(widget, EVT_POINTER_MOVE, on_guage_pointer_move, widget);
            widget_on(widget, EVT_POINTER_UP, on_guage_pointer_up, widget);
            widget_on(widget, EVT_POINTER_LEAVE, on_guage_pointer_leave, widget);
        } else if (tk_str_eq(name, "deep_guage_pointer")) {
            widget_on(widget, EVT_POINTER_DOWN, on_guage_pointer_down, widget);
            widget_on(widget, EVT_POINTER_MOVE, on_guage_pointer_move, widget);
            widget_on(widget, EVT_POINTER_UP, on_guage_pointer_up, widget);
            widget_on(widget, EVT_POINTER_LEAVE, on_guage_pointer_leave, widget);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t advanced_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    s_is_down_dialog = FALSE;
    g_evt_pointer_up_num = widget_on(win, EVT_POINTER_UP, on_dialog_state, win);
    g_evt_pointer_down_num = widget_on(win, EVT_POINTER_DOWN, on_dialog_state, win);
    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
