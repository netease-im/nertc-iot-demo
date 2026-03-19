#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"
#include "chart_view/bar_series_minmax.h"

static int32_t s_last_rate = 65;
static uint64_t s_last_check_time = 0;

static ret_t on_back(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t on_start_check_click(void *ctx, event_t *e)
{
    s_last_check_time = time_now_s();
    return navigator_to_with_context("heart_rate_check", &s_last_rate);
}

static ret_t on_update_heart_rate_data(widget_t *win)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_t *widget;
    uint32_t tn = time_now_s();
    uint32_t hour = (tn - s_last_check_time) / 60 / 60;
    uint32_t min = (tn - s_last_check_time) / 60;
    char text[4] = {0};

    widget = widget_lookup(win, "last_rate", TRUE);
    if (widget != NULL) {
        str_t str;
        str_init(&str, 0);
        str_from_int(&str, s_last_rate);
        widget_set_text_utf8(widget, str.str);
        str_reset(&str);
    }

    widget = widget_lookup(win, "last_check_time", TRUE);
    if (widget != NULL) {
        if (hour == 0 && min == 0) {
            widget_set_text_utf8(widget, "");
        } else {
            str_t str;
            str_init(&str, 0);
            str_from_int(&str, hour > 0 ? hour : min);
            widget_set_text_utf8(widget, str.str);
            str_reset(&str);
        }
    }

    widget = widget_lookup(win, "last_check_time_unit", TRUE);
    if (widget != NULL) {
        if (hour == 0 && min == 0) {
            widget_set_text_utf8(widget, "刚刚");
        } else if (hour > 0) {
            widget_set_text_utf8(widget, "小时前");
        } else {
            widget_set_text_utf8(widget, "分钟前");
        }
    }

    float_t curr_rate = 0;
    widget = widget_lookup(win, "rate_series", TRUE);
    if (widget != NULL) {
        float_t max = 60 + random() % 40;
        float_t min = 60 + random() % 40;
        curr_rate = (max + min) / 2;
        series_data_minmax_t data = {min, max};
        series_push(widget, &data, 1);
    }

    widget = widget_lookup(win, "curr_rate", TRUE);
    if (widget != NULL) {
        tk_itoa(text, 4, (int32_t)curr_rate);
        widget_set_text_utf8(widget, text);
    }

    return RET_OK;
}

static ret_t on_update_heart_rate_timer(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);

    on_update_heart_rate_data(win);

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
            widget_on(widget, EVT_SLIDE_RIGHT, on_back, win);
        } else if (tk_str_eq(name, "start_check")) {
            widget_on(widget, EVT_CLICK, on_start_check_click, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t heart_rate_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    if (s_last_check_time == 0) {
        s_last_check_time = time_now_s();
    }

    widget_foreach(win, visit_init_child, win);

    widget_add_timer(win, on_update_heart_rate_timer, 1000);

    return RET_OK;
}

