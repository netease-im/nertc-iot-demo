#include "awtk.h"
#include "../src/qr/qr.h"
#include "../src/qr_register.h"

static ret_t on_close(void *ctx, event_t *e)
{
    tk_quit();

    return RET_OK;
}

static ret_t on_scanned(const timer_info_t *info)
{
    widget_t *win = WIDGET(info->ctx);
    widget_t *qr = widget_lookup_by_type(win, WIDGET_TYPE_QR, TRUE);

    /*
     * 自定义状态对designer不太友好，所以用disable状态模拟scanned状态。
     */
    widget_set_enable(qr, FALSE);

    return RET_REMOVE;
}

/**
 * 初始化
 */
ret_t application_init(void)
{
    qr_register();

    widget_t *win = window_open("main");
    widget_child_on(win, "close", EVT_CLICK, on_close, win);
    widget_add_timer(win, on_scanned, 3000);

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
