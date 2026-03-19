#include "awtk.h"
#include "../common/navigator.h"

static ret_t on_close_click(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t on_style_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    char text[64] = "";
    widget_t *win = WIDGET(ctx);
    widget_t *target = (widget_t *)e->target;
    widget_get_text_utf8(target, text, ARRAY_SIZE(text));

    widget_use_style(win, text);

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
        if (tk_str_eq(name, "style")) {
            widget_on(widget, EVT_CLICK, on_style_click, win);
        } else if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_close_click, NULL);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t background_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
