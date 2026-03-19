#include "awtk.h"
#include "../common/navigator.h"

static ret_t progress_bar_animate_delta(widget_t *win, const char *name, int32_t delta);

/**
 * 正在编辑事件
 */
static ret_t on_edit_value_changing(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *target = WIDGET(e->target);
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "edit_label", TRUE);

    assert(label != NULL);

    widget_set_text(label, target->text.str);

    return RET_OK;
}

static ret_t on_changing_slider_value_changing(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = WIDGET(ctx);
    widget_t *bar = widget_lookup(win, "changing_bar", TRUE);
    widget_t *slider = WIDGET(e->target);
    value_t v;

    assert(bar != NULL);

    value_set_int32(&v, widget_get_value(slider));
    progress_bar_set_value(bar, value_int32(&v));

    return RET_OK;
}

static ret_t on_changed_slider_value_changed(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = WIDGET(ctx);
    widget_t *bar = widget_lookup(win, "changed_bar", TRUE);
    widget_t *slider = WIDGET(e->target);
    value_t v;

    assert(bar != NULL);

    value_set_int32(&v, widget_get_value(slider));
    progress_bar_set_value(bar, value_int32(&v));

    return RET_OK;
}

/**
 * 递减按钮事件
 */
static ret_t on_dec_btn_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = WIDGET(ctx);
    (void)e;

    progress_bar_animate_delta(win, "click_bar", -10);

    return RET_OK;
}

/**
 * 递增按钮事件
 */
static ret_t on_inc_btn_click(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    (void)e;

    progress_bar_animate_delta(win, "click_bar", 10);

    return RET_OK;
}

static ret_t on_click_bar_value_changed(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = WIDGET(ctx);
    widget_t *label = widget_lookup(win, "click_bar_label", TRUE);
    widget_t *bar = WIDGET(e->target);
    char text[32];
    value_t v;

    assert(bar != NULL);
    assert(label != NULL);

    value_set_int32(&v, widget_get_value(bar));
    tk_snprintf(text, sizeof(text), "%d%%", value_int32(&v));
    widget_set_text_utf8(label, text);

    return RET_OK;
}

static ret_t on_close_click(void *ctx, event_t *e)
{
    return navigator_back();
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
        if (tk_str_eq(name, "edit")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_edit_value_changing, win);
        } else if (tk_str_eq(name, "changing_slider")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_changing_slider_value_changing, win);
        } else if (tk_str_eq(name, "changed_slider")) {
            widget_on(widget, EVT_VALUE_CHANGED, on_changed_slider_value_changed, win);
        } else if (tk_str_eq(name, "dec_btn")) {
            widget_on(widget, EVT_CLICK, on_dec_btn_click, win);
        } else if (tk_str_eq(name, "inc_btn")) {
            widget_on(widget, EVT_CLICK, on_inc_btn_click, win);
        } else if (tk_str_eq(name, "click_bar")) {
            widget_on(widget, EVT_VALUE_CHANGED, on_click_bar_value_changed, win);
        } else if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_close_click, NULL);
        }
    }

    return RET_OK;
}

/**
 * 进度条动画
 */
static ret_t progress_bar_animate_delta(widget_t *win, const char *name, int32_t delta)
{
    widget_t *progress_bar = widget_lookup(win, name, TRUE);
    assert(progress_bar != NULL);

    int32_t value = (PROGRESS_BAR(progress_bar)->value + delta);
    value = tk_min(100, value);
    value = tk_max(0, value);
    widget_animate_value_to(progress_bar, value, 500);

    return RET_OK;
}

/**
 * 定时器事件（定时增加progress_bar的值）
 */
static ret_t on_add_bar_value(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    widget_t *bar = widget_lookup(win, "click_bar", TRUE);
    value_t v;
    int32_t val;

    assert(bar != NULL);

    value_set_int32(&v, widget_get_value(bar));
    val = value_int32(&v);
    if (val >= 100) {
        progress_bar_set_value(bar, 0);
    }
    progress_bar_animate_delta(win, "click_bar", 10);

    return RET_REPEAT;
}

/**
 * 初始化窗口
 */
ret_t basic_init(widget_t *win, void *ctx)
{
    value_t v;
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    /*添加定时器 */
    widget_add_timer(win, on_add_bar_value, 1500);

    return RET_OK;
}
