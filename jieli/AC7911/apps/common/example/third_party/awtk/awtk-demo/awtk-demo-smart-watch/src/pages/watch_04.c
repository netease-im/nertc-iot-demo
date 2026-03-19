#include "awtk.h"
#include "../common/navigator.h"
#include "../common/music_player.h"
#include "../custom_widgets/gesture.h"

static ret_t on_open_watch_select(void *ctx, event_t *e)
{
    return navigator_replace_with_context("watch_select", "watch_04");
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

static ret_t on_open_wathc_06(void *ctx, event_t *e)
{
    return navigator_replace("watch_06");
}

static ret_t on_calendar_click(void *ctx, event_t *e)
{
    return navigator_to("calendar");
}

static ret_t on_weather_click(void *ctx, event_t *e)
{
    return navigator_to("weather");
}

static ret_t on_music_click(void *ctx, event_t *e)
{
    return navigator_to("music");
}

static ret_t set_wday(widget_t *widget, date_time_t *dt)
{
    static const char *const wdays[] = {
        "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat",
    };
    widget_set_text_utf8(widget, wdays[dt->wday]);
    return RET_OK;
}

static ret_t set_day(widget_t *widget, date_time_t *dt)
{
    char str[3] = {0};
    tk_snprintf(str, sizeof(str), "%02d", dt->day);
    widget_set_text_utf8(widget, str);
    return RET_OK;
}

static ret_t update_time(widget_t *win)
{
    widget_t *widget = NULL;
    date_time_t dt;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    date_time_init(&dt);

    widget = widget_lookup(win, "wday", TRUE);
    if (widget != NULL) {
        set_wday(widget, &dt);
    }

    widget = widget_lookup(win, "day", TRUE);
    if (widget != NULL) {
        set_day(widget, &dt);
    }

    widget = widget_lookup(win, "sec", TRUE);
    if (widget != NULL) {
        widget_set_value_int(widget, dt.second * 6);
    }

    widget = widget_lookup(win, "min", TRUE);
    if (widget != NULL) {
        widget_set_value_int(widget, dt.minute * 6 + dt.second * 0.1);
    }

    widget = widget_lookup(win, "hour", TRUE);
    if (widget != NULL) {
        widget_set_value_int(widget, dt.hour * 30 + dt.minute * 0.5);
    }

    return RET_OK;
}

static ret_t on_update_data(const timer_info_t *timer)
{
    static const char *const leds[] = {"wechat", "phone", "run", "training", "heart"};
    widget_t *win = WIDGET(timer->ctx);
    widget_t *widget;
    uint32_t i = 0;
    uint32_t nr = sizeof(leds) / sizeof(char *);

    update_time(win);

    widget = widget_lookup(win, "music_progress", TRUE);
    if (widget != NULL) {
        if (music_player() != NULL) {
            widget_set_value_int(widget, music_player()->current_progress);
        } else {
            widget_set_value_int(widget, 0);
        }
    }

    for (i = 0; i < nr; i++) {
        widget = widget_lookup(win, leds[i], TRUE);
        if (widget != NULL && strstr(widget->style, "_pressed") != NULL) {
            str_t str;
            uint32_t next = (i == (nr - 1)) ? 0 : i + 1;

            str_init(&str, 0);
            str_set_with_len(&str, widget->style, strlen(widget->style) - strlen("_pressed"));
            widget_use_style(widget, str.str);

            widget = widget_lookup(win, leds[next], TRUE);
            if (widget != NULL) {
                str_set(&str, widget->style);
                str_append(&str, "_pressed");
                widget_use_style(widget, str.str);
            }

            str_reset(&str);
            break;
        }
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
            widget_on(widget, EVT_SLIDE_RIGHT, on_open_wathc_06, win);
        } else if (tk_str_eq(name, "music_progress")) {
            if (music_player() != NULL) {
                widget_set_value_int(widget, music_player()->current_progress);
            } else {
                widget_set_value_int(widget, 0);
            }
        } else if (tk_str_eq(name, "wday")) {
            date_time_t dt;
            date_time_init(&dt);
            set_wday(widget, &dt);
        } else if (tk_str_eq(name, "day")) {
            date_time_t dt;
            date_time_init(&dt);
            set_day(widget, &dt);
        } else if (tk_str_eq(name, "calendar")) {
            widget_on(widget, EVT_CLICK, on_calendar_click, win);
        } else if (tk_str_eq(name, "weather")) {
            widget_on(widget, EVT_CLICK, on_weather_click, win);
        } else if (tk_str_eq(name, "music")) {
            widget_on(widget, EVT_CLICK, on_music_click, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t watch_04_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);
    update_time(win);
    widget_add_timer(win, on_update_data, 1000);

    return RET_OK;
}
