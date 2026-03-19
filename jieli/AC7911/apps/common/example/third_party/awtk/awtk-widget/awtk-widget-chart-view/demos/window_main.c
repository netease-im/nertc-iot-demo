#include "awtk.h"
#include "../src/chart_view_register.h"

extern ret_t application_init(void);
extern ret_t open_line_series_window(const char *name);
extern ret_t open_bar_series_window(const char *name);
extern ret_t open_pie_window(void);
extern ret_t open_tooltip_format_window(void);

static ret_t on_graph(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(e->target);
    (void)ctx;
    (void)e;

    return open_line_series_window(widget->name);
}

static ret_t on_histogram(void *ctx, event_t *e)
{
    widget_t *widget = WIDGET(e->target);
    (void)ctx;
    (void)e;

    return open_bar_series_window(widget->name);
}

static ret_t on_pie(void *ctx, event_t *e)
{
    (void)ctx;
    (void)e;

    return open_pie_window();
}

static ret_t on_tooltip_format(void *ctx, event_t *e)
{
    (void)ctx;
    (void)e;

    return open_tooltip_format_window();
}

static ret_t init_widget(void *ctx, const void *iter)
{
    widget_t *widget = WIDGET(iter);
    widget_t *win = widget_get_window(widget);

    if (widget->name != NULL) {
        const char *name = widget->name;
        if (strstr(name, "window_line_series") != NULL) {
            widget_on(widget, EVT_CLICK, on_graph, win);
        } else if (strstr(name, "window_bar_series") != NULL) {
            widget_on(widget, EVT_CLICK, on_histogram, win);
        } else if (tk_str_eq(name, "window_pie")) {
            widget_on(widget, EVT_CLICK, on_pie, win);
        } else if (tk_str_eq(name, "window_tooltip_format")) {
            widget_on(widget, EVT_CLICK, on_tooltip_format, win);
        }
    }

    (void)ctx;

    return RET_OK;
}

static void init_children_widget(widget_t *widget)
{
    widget_foreach(widget, init_widget, widget);
}

/**
 * 初始化
 */
ret_t application_init()
{
    chart_view_register();

    widget_t *system_bar = window_open("system_bar");
    widget_t *win = window_open("home_page");
    if (win) {
        init_children_widget(win);
        return RET_OK;
    }

    return RET_FAIL;
}

/**
 * 退出
 */
ret_t application_exit(void)
{
    log_debug("application_exit");
    return RET_OK;
}
