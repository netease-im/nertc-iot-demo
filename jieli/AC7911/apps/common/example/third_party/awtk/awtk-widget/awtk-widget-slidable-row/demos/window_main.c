#include "awtk.h"
#include "slidable_row_register.h"

static ret_t on_close(void *ctx, event_t *e)
{
    tk_quit();

    return RET_OK;
}

static ret_t on_label_point_up(void *ctx, event_t *e)
{
    widget_t *label = WIDGET(e->target);
    printf("on_label_point_up \r\n");
    return RET_OK;
}

static ret_t on_confirm_remove(void *ctx, event_t *e)
{
    widget_t *button = WIDGET(e->target);

    return widget_destroy_async(button->parent);
}

static ret_t on_remove(void *ctx, event_t *e)
{
    ret_t ret = dialog_confirm("Confirm", "Are you sure to remove?");
    widget_t *button = WIDGET(e->target);

    if (ret == RET_OK) {
        widget_destroy_async(button->parent);
        return RET_OK;
    } else {
        return RET_OK;
    }
}

static ret_t on_mark_as_read(void *ctx, event_t *e)
{
    log_debug("on_mark_as_read\n");
    return RET_OK;
}

static ret_t on_widget(void *ctx, const void *data)
{
    widget_t *widget = WIDGET(data);

    if (tk_str_eq(widget->name, "remove")) {
        widget_on(widget, EVT_CLICK, on_remove, NULL);
    } else if (tk_str_eq(widget->name, "mark")) {
        widget_on(widget, EVT_CLICK, on_mark_as_read, NULL);
    } else if (tk_str_eq(widget->name, "confirm_remove")) {
        widget_on(widget, EVT_CLICK, on_confirm_remove, NULL);
    } else if (tk_str_eq(widget->name, "title")) {
        widget_on(widget, EVT_POINTER_UP, on_label_point_up, NULL);
    }

    return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    slidable_row_register();

    widget_t *win = window_open("main");
    widget_child_on(win, "close", EVT_CLICK, on_close, NULL);
    widget_foreach(win, on_widget, win);

    return RET_OK;
}

/**
 * 退出
 */
ret_t application_exit(void)
{
    log_debug("application_exit\n");
    return RET_OK;
}
