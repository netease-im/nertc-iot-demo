#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

static cnt = 0;

static ret_t on_back(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t on_heart_rate_checking(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);
    ret_t ret = RET_REPEAT;

    cnt ++;
    if (cnt == 5) {
        widget_t *widget;
        int32_t rate = 60 + random() % 61;
        int32_t *result = widget_get_prop_pointer(win, "result");

        if (result != NULL) {
            *result = rate;
        }

        widget = widget_lookup(win, "stat", TRUE);
        if (widget != NULL) {
            str_t str;
            str_init(&str, 0);
            str_append(&str, "检测结果:");
            str_append_int(&str, rate);
            widget_set_text_utf8(widget, str.str);
            str_reset(&str);
        }
    } else if (cnt > 5 && cnt < 9) {
        widget_t *widget = widget_lookup(win, "tip", TRUE);
        if (widget != NULL) {
            char text[2] = {0};
            str_t str;
            str_init(&str, 0);
            str_append(&str, "即将关闭");
            tk_itoa(text, 2, 9 - cnt);
            str_append(&str, text);
            widget_set_text_utf8(widget, str.str);
            str_reset(&str);
        }
    } else if (cnt == 10) {
        navigator_back();
        ret = RET_REMOVE;
    }

    return ret;
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
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t heart_rate_check_init(widget_t *win, void *ctx)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_set_prop_pointer(win, "result", ctx);
    widget_foreach(win, visit_init_child, win);

    cnt = 0;
    widget_add_timer(win, on_heart_rate_checking, 1000);

    return RET_OK;
}
