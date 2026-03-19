#include "awtk.h"
#include "cache_view/cache_view.h"
#include "cache_view_register.h"

static ret_t on_close(void *ctx, event_t *e)
{
    tk_quit();

    return RET_OK;
}

static ret_t on_disable(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *check_button = WIDGET(e->target);
    widget_t *cache_view = widget_lookup_by_type(win, WIDGET_TYPE_CACHE_VIEW, TRUE);
    bool_t disable = widget_get_value(check_button);

    log_debug("disable_cache=%d\n", disable);
    CACHE_VIEW(cache_view)->disable_cache = disable;

    return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    cache_view_register();

    widget_t *win = window_open("main");
    window_manager_set_show_fps(window_manager(), TRUE);
    widget_child_on(win, "close", EVT_CLICK, on_close, win);
    widget_child_on(win, "disable", EVT_VALUE_CHANGED, on_disable, win);

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
