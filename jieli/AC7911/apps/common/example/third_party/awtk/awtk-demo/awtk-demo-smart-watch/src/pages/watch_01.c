#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

static ret_t on_open_watch_select(void *ctx, event_t *e)
{
    return navigator_replace_with_context("watch_select", "watch_01");
}

static ret_t on_open_message(void *ctx, event_t *e)
{
    return navigator_to("message");
}

static ret_t on_open_app_list(void *ctx, event_t *e)
{
    return navigator_to("app_list");
}

static ret_t on_open_shortcut(void *ctx, event_t *e)
{
    return navigator_to("quick");
}

static ret_t on_open_watch_02(void *ctx, event_t *e)
{
    return navigator_replace("watch_02");
}

static ret_t on_weather_click(void *ctx, event_t *e)
{
    return navigator_to("weather");
}

static ret_t on_open_weather(void *ctx, event_t *e)
{
    return navigator_to("weather");
}

static ret_t on_open_health(void *ctx, event_t *e)
{
    return navigator_to("health");
}

static ret_t update_guage(widget_t *win)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    date_time_t dt;
    date_time_init(&dt);
    widget_t *widget = NULL;

    widget = widget_lookup(win, "sec", TRUE);
    if (widget != NULL) {
        widget_set_value_int(widget, dt.second * 6);
    }

    widget = widget_lookup(win, "min", TRUE);
    if (widget != NULL) {
        widget_set_value(widget, dt.minute * 6 + dt.second * 0.1);
    }

    widget = widget_lookup(win, "hour", TRUE);
    if (widget != NULL) {
        widget_set_value(widget, dt.hour * 30 + dt.minute * 0.5);
    }

    return RET_OK;
}

static ret_t on_update_data(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    widget_t *widget;
    int32_t v = 100;

    widget = widget_lookup(win, "battery_progress", TRUE);
    if (widget != NULL) {
        v = widget_get_value_int(widget) - 1;
        if (v < 0) {
            v = 100;
        }
        widget_set_value_int(widget, v);
    }

    widget = widget_lookup(win, "battery_val", TRUE);
    if (widget != NULL) {
        str_t str;
        str_init(&str, 6);
        str_from_int(&str, v);
        str_append(&str, "%");
        widget_set_text_utf8(widget, str.str);
        str_reset(&str);
    }

    widget = widget_lookup(win, "health_progress", TRUE);
    if (widget != NULL) {
        v = widget_get_value_int(widget) + 100;
        if (v > 36000) {
            v = 0;
        }
        widget_set_value_int(widget, v);
    }

    widget = widget_lookup(win, "health_value", TRUE);
    if (widget != NULL) {
        str_t str;
        str_init(&str, 6);
        str_from_int(&str, v);
        widget_set_text_utf8(widget, str.str);
        str_reset(&str);
    }

    update_guage(win);

    return RET_REPEAT;
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
        if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_LONG_PRESS, on_open_watch_select, win);
            widget_on(widget, EVT_SLIDE_DOWN, on_open_message, win);
            widget_on(widget, EVT_SLIDE_LEFT, on_open_app_list, win);
            widget_on(widget, EVT_SLIDE_UP, on_open_shortcut, win);
            widget_on(widget, EVT_SLIDE_RIGHT, on_open_watch_02, win);
        } else if (tk_str_eq(name, "weather")) {
            widget_on(widget, EVT_CLICK, on_open_weather, win);
        } else if (tk_str_eq(name, "health")) {
            widget_on(widget, EVT_CLICK, on_open_health, win);
        } else if (tk_str_eq(name, "battery_progress")) {
            widget_set_value_int(widget, 100);
        } else if (tk_str_eq(name, "battery_val")) {
            widget_set_text_utf8(widget, "100%");
        } else if (tk_str_eq(name, "health_progress")) {
            widget_set_value_int(widget, 0);
        } else if (tk_str_eq(name, "health_value")) {
            widget_set_text_utf8(widget, "0");
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t watch_01_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    update_guage(win);
    widget_add_timer(win, on_update_data, 1000);

    return RET_OK;
}
