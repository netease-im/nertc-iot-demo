#include "awtk.h"
#include "../common/navigator.h"
#include "../custom_widgets/gesture.h"

static ret_t on_back(void *ctx, event_t *e)
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
        if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_SLIDE_RIGHT, on_back, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t app_default_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
