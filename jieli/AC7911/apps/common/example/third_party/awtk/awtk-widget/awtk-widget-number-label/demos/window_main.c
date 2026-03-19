#include "awtk.h"
#include "../src/number_label_register.h"

static ret_t on_close(void *ctx, event_t *e)
{
    tk_quit();

    return RET_OK;
}

static ret_t on_value_changed(void *ctx, event_t *e)
{
    widget_t *target = WIDGET(e->target);
    value_change_event_t *evt = value_change_event_cast(e);

    log_debug("%s changed: %lf=>%lf\n", target->name, value_double(&(evt->old_value)), value_double(&(evt->new_value)));

    return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    number_label_register();

    widget_t *win = window_open("main");
    widget_child_on(win, "close", EVT_CLICK, on_close, NULL);
    widget_child_on(win, "num7", EVT_VALUE_CHANGED, on_value_changed, NULL);
    widget_child_on(win, "num8", EVT_VALUE_CHANGED, on_value_changed, NULL);

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
