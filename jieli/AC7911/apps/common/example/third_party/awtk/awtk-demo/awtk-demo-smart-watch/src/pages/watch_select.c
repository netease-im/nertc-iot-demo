#include "awtk.h"
#include "../common/navigator.h"

static ret_t on_watch_click(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(e->target);
    char name[TK_NAME_LEN + 1] = {0};

    widget_get_text_utf8(widget, name, TK_NAME_LEN);
    return navigator_to(name);
}

static ret_t set_current_watch(widget_t *win, const char *watch_text)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    char name[TK_NAME_LEN + 1] = {0};

    widget_t *menu = widget_lookup(win, "slide_menu", TRUE);
    if (menu != NULL) {
        WIDGET_FOR_EACH_CHILD_BEGIN(menu, iter, i)

        widget_get_text_utf8(iter, name, TK_NAME_LEN);
        if (tk_str_eq(watch_text, name)) {
            widget_set_value_int(menu, i);
            break;
        }

        WIDGET_FOR_EACH_CHILD_END()
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
        if (tk_str_eq(name, "watch")) {
            widget_on(widget, EVT_CLICK, on_watch_click, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t watch_select_init(widget_t *win, void *ctx)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);
    set_current_watch(win, (char *)ctx);

    return RET_OK;
}
