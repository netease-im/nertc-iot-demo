#include "awtk.h"
#include "rlottie/rlottie.h"
#include "rlottie_register.h"

static ret_t on_play(void *ctx, event_t *e)
{
    widget_t *rlottie = widget_lookup(WIDGET(ctx), "rlottie", TRUE);
    rlottie_play(rlottie);

    return RET_OK;
}

static ret_t on_stop(void *ctx, event_t *e)
{
    widget_t *rlottie = widget_lookup(WIDGET(ctx), "rlottie", TRUE);
    rlottie_stop(rlottie);

    return RET_OK;
}

static ret_t on_pause(void *ctx, event_t *e)
{
    widget_t *rlottie = widget_lookup(WIDGET(ctx), "rlottie", TRUE);
    rlottie_pause(rlottie);

    return RET_OK;
}

static ret_t on_asset_changed(void *ctx, event_t *e)
{
    widget_t *assets = widget_lookup(WIDGET(ctx), "assets", TRUE);
    widget_t *rlottie = widget_lookup(WIDGET(ctx), "rlottie", TRUE);
    const char *name = combo_box_get_text(assets);

    tk_mem_dump();
    rlottie_set_url(rlottie, name);
    log_debug("%s\n", name);
    tk_mem_dump();

    return RET_OK;
}

ret_t application_init(void)
{
    rlottie_register();

    widget_t *win = window_open("main");
    widget_child_on(win, "play", EVT_CLICK, on_play, win);
    widget_child_on(win, "pause", EVT_CLICK, on_pause, win);
    widget_child_on(win, "stop", EVT_CLICK, on_stop, win);
    widget_child_on(win, "assets", EVT_VALUE_CHANGED, on_asset_changed, win);

    window_manager_set_show_fps(window_manager(), TRUE);

    /* rlottie_set_url(widget_lookup(WIDGET(win), "rlottie", TRUE), "done.json"); */

    return RET_OK;
}

ret_t application_exit(void)
{
    log_debug("application_exit\n");
    return RET_OK;
}
