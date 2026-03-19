#include "awtk.h"
#include "../common/navigator.h"

static int32_t hour_value = 0;
static int32_t min_value = 0;
static int32_t sec_value = 0;

static ret_t save_text_selector(widget_t *win)
{
    widget_t *hour_selector = widget_lookup(win, "hour_selector", TRUE);
    if (hour_selector) {
        hour_value = text_selector_get_value(hour_selector);
    }

    widget_t *min_selector = widget_lookup(win, "min_selector", TRUE);
    if (min_selector) {
        min_value = text_selector_get_value(min_selector);
    }

    widget_t *sec_selector = widget_lookup(win, "sec_selector", TRUE);
    if (sec_selector) {
        sec_value = text_selector_get_value(sec_selector);
    }

    return RET_OK;
}

static ret_t on_ok_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = (widget_t *)ctx;
    save_text_selector(win);
    return dialog_quit(win, RET_OK);
}

static ret_t on_cancle_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = (widget_t *)ctx;
    return dialog_quit(win, RET_FAIL);
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
        if (tk_str_eq(name, "ok")) {
            widget_on(widget, EVT_CLICK, on_ok_click, win);
        } else if (tk_str_eq(name, "cancle")) {
            widget_on(widget, EVT_CLICK, on_cancle_click, win);
        } else if (tk_str_eq(name, "hour_selector")) {
#ifdef TEXT_SELECTOR_PROP_VALUE_ANIMATOR
            widget_set_prop_bool(widget, TEXT_SELECTOR_PROP_VALUE_ANIMATOR, FALSE);
#endif
            text_selector_set_value(widget, hour_value);
        } else if (tk_str_eq(name, "min_selector")) {
#ifdef TEXT_SELECTOR_PROP_VALUE_ANIMATOR
            widget_set_prop_bool(widget, TEXT_SELECTOR_PROP_VALUE_ANIMATOR, FALSE);
#endif
            text_selector_set_value(widget, min_value);
        } else if (tk_str_eq(name, "sec_selector")) {
#ifdef TEXT_SELECTOR_PROP_VALUE_ANIMATOR
            widget_set_prop_bool(widget, TEXT_SELECTOR_PROP_VALUE_ANIMATOR, FALSE);
#endif
            text_selector_set_value(widget, sec_value);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t timing_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
