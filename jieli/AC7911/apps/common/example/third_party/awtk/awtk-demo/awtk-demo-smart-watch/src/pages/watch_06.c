#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"
#include "../common/music_player.h"
#include "chart_view/bar_series_minmax.h"

static ret_t on_open_watch_select(void *ctx, event_t *e)
{
    return navigator_replace_with_context("watch_select", "watch_06");
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

static ret_t on_open_wathc_01(void *ctx, event_t *e)
{
    return navigator_replace("watch_01");
}

static ret_t on_open_weather(void *ctx, event_t *e)
{
    return navigator_to("weather");
}

//static ret_t on_open_calendar(void* ctx, event_t* e) {
//  return navigator_to("calendar");
//}

//static ret_t on_open_health(void* ctx, event_t* e) {
//  return navigator_to("health");
//}

//static ret_t on_open_music(void* ctx, event_t* e) {
//  return navigator_to("music");
//}

static ret_t on_calendar_click(void *ctx, event_t *e)
{
    return navigator_to("calendar");
}

static ret_t on_health_click(void *ctx, event_t *e)
{
    return navigator_to("health");
}

static ret_t on_music_click(void *ctx, event_t *e)
{
    return navigator_to("music");
}

static ret_t on_update_data(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    widget_t *widget;

    widget = widget_lookup(win, "weather", TRUE);
    if (widget != NULL && !tk_str_eq(widget->state, WIDGET_STATE_OVER)) {
        char style[64] = {0};
        tk_snprintf(style, sizeof(style) - 1, "watch_06_weather_%02d", random() % 4);
        widget_use_style(widget, style);
    }

    widget = widget_lookup(win, "rate_series", TRUE);
    if (widget != NULL) {
        series_data_minmax_t data = {60 + random() % 40, 60 + random() % 40};
        series_push(widget, &data, 1);
    }

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
            widget_on(widget, EVT_SLIDE_RIGHT, on_open_wathc_01, win);
        } else if (tk_str_eq(name, "weather")) {
            widget_on(widget, EVT_PAINT, on_open_weather, win);
        } else if (tk_str_eq(name, "calendar")) {
            widget_on(widget, EVT_CLICK, on_calendar_click, win);
        } else if (tk_str_eq(name, "health")) {
            widget_on(widget, EVT_CLICK, on_health_click, win);
        } else if (tk_str_eq(name, "music")) {
            widget_on(widget, EVT_CLICK, on_music_click, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t watch_06_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);
    widget_add_timer(win, on_update_data, 1000);

    return RET_OK;
}
