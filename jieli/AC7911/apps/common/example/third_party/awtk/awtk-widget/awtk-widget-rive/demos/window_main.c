#include "awtk.h"
#include "rive/rive.h"
#include "rive_register.h"

static ret_t on_play(void *ctx, event_t *e)
{
    widget_t *rive = widget_lookup(WIDGET(ctx), "rive", TRUE);
    rive_play(rive);

    return RET_OK;
}

static ret_t on_stop(void *ctx, event_t *e)
{
    widget_t *rive = widget_lookup(WIDGET(ctx), "rive", TRUE);
    rive_stop(rive);

    return RET_OK;
}

static ret_t on_pause(void *ctx, event_t *e)
{
    widget_t *rive = widget_lookup(WIDGET(ctx), "rive", TRUE);
    rive_pause(rive);

    return RET_OK;
}

static ret_t on_asset_changed(void *ctx, event_t *e)
{
    widget_t *assets = widget_lookup(WIDGET(ctx), "assets", TRUE);
    widget_t *rive = widget_lookup(WIDGET(ctx), "rive", TRUE);
    const char *name = combo_box_get_text(assets);

    tk_mem_dump();
    rive_set_url(rive, name);
    log_debug("%s\n", name);
    tk_mem_dump();

    return RET_OK;
}

ret_t application_init(void)
{
    rive_register();

    widget_t *win = window_open("main");
    widget_child_on(win, "play", EVT_CLICK, on_play, win);
    widget_child_on(win, "pause", EVT_CLICK, on_pause, win);
    widget_child_on(win, "stop", EVT_CLICK, on_stop, win);
    widget_child_on(win, "assets", EVT_VALUE_CHANGED, on_asset_changed, win);

    window_manager_set_show_fps(window_manager(), TRUE);

    return RET_OK;
}

ret_t application_exit(void)
{
    log_debug("application_exit\n");
    return RET_OK;
}
