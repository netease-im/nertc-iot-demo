#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

static ret_t on_calendar_click(void *ctx, event_t *e)
{
    return navigator_to("calendar");
}

static ret_t on_weather_click(void *ctx, event_t *e)
{
    return navigator_to("weather");
}

static ret_t on_heart_rate_click(void *ctx, event_t *e)
{
    return navigator_to("heart_rate");
}

static ret_t on_email_click(void *ctx, event_t *e)
{
    return navigator_to("app_default");
}

static ret_t on_music_click(void *ctx, event_t *e)
{
    return navigator_to("music");
}

static ret_t on_health_click(void *ctx, event_t *e)
{
    return navigator_to("health");
}

static ret_t on_back(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t on_window_close(void *ctx, event_t *e)
{
    widget_t *wm = window_manager();
    widget_t *win = WIDGET(ctx);
    widget_t *widget = widget_lookup(win, "slide_menu", TRUE);

    widget_set_prop_int(wm, NAVIGATOR_PROP_APP_INDEX, widget_get_value_int(widget));
    image_manager_unload_unused(image_manager(), 0);

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
        if (tk_str_eq(name, "slide_menu")) {
            widget_t *wm = window_manager();
            widget_set_value_int(widget, widget_get_prop_int(wm, NAVIGATOR_PROP_APP_INDEX, 0));
        } else if (tk_str_eq(name, "calendar")) {
            widget_on(widget, EVT_CLICK, on_calendar_click, win);
        } else if (tk_str_eq(name, "weather")) {
            widget_on(widget, EVT_CLICK, on_weather_click, win);
        } else if (tk_str_eq(name, "heart_rate")) {
            widget_on(widget, EVT_CLICK, on_heart_rate_click, win);
        } else if (tk_str_eq(name, "email")) {
            widget_on(widget, EVT_CLICK, on_email_click, win);
        } else if (tk_str_eq(name, "music")) {
            widget_on(widget, EVT_CLICK, on_music_click, win);
        } else if (tk_str_eq(name, "health")) {
            widget_on(widget, EVT_CLICK, on_health_click, win);
        } else if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_SLIDE_RIGHT, on_back, win);
        } else if (tk_str_eq(name, "app_list")) {
            widget_on(widget, EVT_WINDOW_CLOSE, on_window_close, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t app_list_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
