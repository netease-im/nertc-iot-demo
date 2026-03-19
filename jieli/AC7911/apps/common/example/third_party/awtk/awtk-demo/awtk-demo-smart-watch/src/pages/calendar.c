#include "awtk.h"
#include "base/date_time_format.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

#define CALENDAR_PROP_DATE "date"

static ret_t calendar_update_labels(widget_t *win, date_time_t *dt)
{
    widget_t *widget = widget_lookup(win, "labels", TRUE);

    if (widget != NULL) {
        date_time_t *today = date_time_create();
        int32_t days = date_time_get_days(dt->year, dt->month);
        int32_t wday = date_time_get_wday(dt->year, dt->month, 1);
        char text[3] = {0};

        WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
        if (strstr(iter->name, "day") != NULL) {
            int32_t day = tk_atoi(iter->name + 3) - wday;
            if (day >= 0 && day < days) {
                tk_itoa(text, sizeof(text), day + 1);
                widget_set_text_utf8(iter, text);
                bool_t is_today = dt->year == today->year && dt->month == today->month && day + 1 == today->day;
                widget_use_style(iter, is_today ? "calendar_today" : "calendar");
            } else {
                widget_set_text_utf8(iter, "");
            }
        }
        WIDGET_FOR_EACH_CHILD_END();

        date_time_destroy(today);
    }

    return RET_OK;
}

static ret_t calendar_update_date(widget_t *win, date_time_t *dt)
{
    widget_t *widget = widget_lookup(win, "date", TRUE);

    if (widget != NULL) {
        wstr_t wstr;
        wstr_init(&wstr, 32);
        wstr_format_date_time(&wstr, "Y/MM", dt);
        widget_set_text(widget, wstr.str);
        wstr_reset(&wstr);
    }

    return RET_OK;
}

static ret_t on_labels_up_enter_animate_end(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    date_time_t *dt = widget_get_prop_pointer(win, CALENDAR_PROP_DATE);

    if (win != NULL && dt != NULL) {
        calendar_update_date(win, dt);
    }

    return RET_OK;
}

static ret_t on_labels_up_exit_animate_end(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = widget_lookup(win, "labels", TRUE);
    date_time_t *dt = widget_get_prop_pointer(win, CALENDAR_PROP_DATE);

    if (dt != NULL) {
        dt->day = 1;
        dt->month --;
        if (dt->month <= 0) {
            dt->month = 12;
            dt->year --;
        }
        dt->wday = date_time_get_wday(dt->year, dt->month, dt->day);
        calendar_update_labels(win, dt);
    } else {
        log_debug("Not found CALENDAR_PROP_DATE");
    }

    if (widget != NULL) {
        widget_start_animator(widget, "up_enter");
    }

    return RET_OK;
}

static ret_t on_change_to_last_month(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = widget_lookup(win, "labels", TRUE);
    if (widget != NULL) {
        widget_start_animator(widget, "up_exit");
    }
    return RET_OK;
}

static ret_t on_labels_down_enter_animate_end(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    date_time_t *dt = widget_get_prop_pointer(win, CALENDAR_PROP_DATE);

    if (win != NULL && dt != NULL) {
        calendar_update_date(win, dt);
    }

    return RET_OK;
}

static ret_t on_labels_down_exit_animate_end(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = widget_lookup(win, "labels", TRUE);
    date_time_t *dt = widget_get_prop_pointer(win, CALENDAR_PROP_DATE);

    if (dt != NULL) {
        dt->day = 1;
        dt->month ++;
        if (dt->month > 12) {
            dt->month = 1;
            dt->year ++;
        }
        dt->wday = date_time_get_wday(dt->year, dt->month, dt->day);
        calendar_update_labels(win, dt);
    } else {
        log_debug("Not found CALENDAR_PROP_DATE");
    }

    if (widget != NULL) {
        widget_start_animator(widget, "down_enter");
    }

    return RET_OK;
}

static ret_t on_change_to_next_month(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = widget_lookup(win, "labels", TRUE);
    if (widget != NULL) {
        widget_start_animator(widget, "down_exit");
    }
    return RET_OK;
}

static ret_t on_change_to_today(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    date_time_t *dt = widget_get_prop_pointer(win, CALENDAR_PROP_DATE);
    if (dt != NULL) {
        date_time_init(dt);
        calendar_update_date(win, dt);
        calendar_update_labels(win, dt);
    } else {
        log_debug("Not found CALENDAR_PROP_DATE");
    }
    return RET_OK;
}

static ret_t on_back(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t on_window_close(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    date_time_t *dt = widget_get_prop_pointer(win, CALENDAR_PROP_DATE);
    if (dt != NULL) {
        date_time_destroy(dt);
        widget_set_prop_pointer(win, CALENDAR_PROP_DATE, NULL);
    }
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
        if (tk_str_eq(name, "last_month")) {
            widget_on(widget, EVT_CLICK, on_change_to_last_month, win);
        } else if (tk_str_eq(name, "today")) {
            widget_on(widget, EVT_CLICK, on_change_to_today, win);
        } else if (tk_str_eq(name, "next_month")) {
            widget_on(widget, EVT_CLICK, on_change_to_next_month, win);
        } else if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_SLIDE_UP, on_change_to_last_month, win);
            widget_on(widget, EVT_SLIDE_DOWN, on_change_to_next_month, win);
            widget_on(widget, EVT_SLIDE_RIGHT, on_back, win);
        } else if (tk_str_eq(name, "labels")) {
            widget_animator_t *animator = NULL;

            animator = widget_find_animator(widget, "up_exit");
            if (animator != NULL) {
                widget_animator_on(animator, EVT_ANIM_END, on_labels_up_exit_animate_end, win);
            }

            animator = widget_find_animator(widget, "up_enter");
            if (animator != NULL) {
                widget_animator_on(animator, EVT_ANIM_END, on_labels_up_enter_animate_end, win);
            }

            animator = widget_find_animator(widget, "down_exit");
            if (animator != NULL) {
                widget_animator_on(animator, EVT_ANIM_END, on_labels_down_exit_animate_end, win);
            }

            animator = widget_find_animator(widget, "down_enter");
            if (animator != NULL) {
                widget_animator_on(animator, EVT_ANIM_END, on_labels_down_enter_animate_end, win);
            }
        } else if (tk_str_eq(name, "calendar")) {
            widget_on(widget, EVT_WINDOW_CLOSE, on_window_close, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t calendar_init(widget_t *win, void *ctx)
{
    date_time_t *dt;
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    dt = date_time_create();
    calendar_update_date(win, dt);
    calendar_update_labels(win, dt);
    widget_set_prop_pointer(win, CALENDAR_PROP_DATE, dt);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
