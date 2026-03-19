#include "awtk.h"
#include "../common/navigator.h"

static ret_t on_clone_self_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *widget = WIDGET(e->target);
    widget_t *clone = widget_clone(widget, widget->parent);
    (void)ctx;

    widget_on(clone, EVT_CLICK, on_clone_self_click, clone);

    return RET_OK;
}

static ret_t on_remove_self_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *widget = WIDGET(e->target);
    (void)ctx;

    widget_remove_child(widget->parent, widget);
    widget_destroy(widget);

    return RET_OK;
}

static ret_t on_list_item_click(void *ctx, event_t *e)
{
    // TODO: 在此添加控件事件处理程序代码
    widget_t *win = WIDGET(ctx);
    widget_t *list_item = WIDGET(e->target);
    widget_t *label = widget_lookup(win, "list_view_label", TRUE);
    value_t v;

    value_set_wstr(&v, widget_get_text(list_item));
    widget_set_text(label, value_wstr(&v));

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
        if (tk_str_eq(name, "clone_self")) {
            widget_on(widget, EVT_CLICK, on_clone_self_click, win);
        } else if (tk_str_eq(name, "remove_self")) {
            widget_on(widget, EVT_CLICK, on_remove_self_click, win);
        } else if (tk_str_eq(name, "close")) {
            widget_on(widget, EVT_CLICK, on_close_click, NULL);
        } else if (tk_str_eq(name, "list_item")) {
            widget_on(widget, EVT_CLICK, on_list_item_click, win);
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t listview_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_foreach(win, visit_init_child, win);

    return RET_OK;
}
