#include "awtk.h"
#include "navigator.h"

extern ret_t weather_init(widget_t *win, void *ctx);
extern ret_t watch_06_init(widget_t *win, void *ctx);
extern ret_t music_init(widget_t *win, void *ctx);
extern ret_t heart_rate_init(widget_t *win, void *ctx);
extern ret_t health_init(widget_t *win, void *ctx);
extern ret_t calendar_init(widget_t *win, void *ctx);
extern ret_t app_default_init(widget_t *win, void *ctx);
extern ret_t app_list_init(widget_t *win, void *ctx);
extern ret_t watch_04_init(widget_t *win, void *ctx);
extern ret_t watch_select_init(widget_t *win, void *ctx);
extern ret_t quick_init(widget_t *win, void *ctx);
extern ret_t watch_01_init(widget_t *win, void *ctx);
extern ret_t watch_02_init(widget_t *win, void *ctx);
extern ret_t heart_rate_check_init(widget_t *win, void *ctx);
extern ret_t message_init(widget_t *win, void *ctx);

static ret_t navigator_window_init(const char *name, widget_t *win, void *ctx)
{
    if (tk_str_eq(name, "weather")) {
        return weather_init(win, ctx);
    } else if (tk_str_eq(name, "watch_06")) {
        return watch_06_init(win, ctx);
    } else if (tk_str_eq(name, "music")) {
        return music_init(win, ctx);
    } else if (tk_str_eq(name, "heart_rate")) {
        return heart_rate_init(win, ctx);
    } else if (tk_str_eq(name, "health")) {
        return health_init(win, ctx);
    } else if (tk_str_eq(name, "calendar")) {
        return calendar_init(win, ctx);
    } else if (tk_str_eq(name, "app_default")) {
        return app_default_init(win, ctx);
    } else if (tk_str_eq(name, "app_list")) {
        return app_list_init(win, ctx);
    } else if (tk_str_eq(name, "watch_04")) {
        return watch_04_init(win, ctx);
    } else if (tk_str_eq(name, "watch_select")) {
        return watch_select_init(win, ctx);
    } else if (tk_str_eq(name, "quick")) {
        return quick_init(win, ctx);
    } else if (tk_str_eq(name, "watch_01")) {
        return watch_01_init(win, ctx);
    } else if (tk_str_eq(name, "watch_02")) {
        return watch_02_init(win, ctx);
    } else if (tk_str_eq(name, "heart_rate_check")) {
        return heart_rate_check_init(win, ctx);
    } else if (tk_str_eq(name, "message")) {
        return message_init(win, ctx);
    }
    return RET_OK;
}

static ret_t navigator_window_open_and_close(const char *name,
        widget_t *to_close, void *ctx)
{
    ret_t ret = RET_OK;
    widget_t *wm = window_manager();
    widget_t *win = widget_child(wm, name);

    if (win != NULL) {
        bool_t single_instance = widget_get_prop_bool(win, WIDGET_PROP_SINGLE_INSTANCE, FALSE);
        if (single_instance) {
            window_manager_close_window_force(wm, win);
        }
    }

    win = window_open_and_close(name, to_close);
    return_value_if_fail(win != NULL, RET_FAIL);

    ret = navigator_window_init(name, win, ctx);
    return_value_if_fail(ret == RET_OK, ret);

    if (widget_is_dialog(win) &&
        widget_get_prop_bool(win, NAVIGATOR_PROP_DIALOG_TO_MODAL, TRUE)) {
        return (ret_t)dialog_modal(win);
    }

    return ret;
}

ret_t navigator_to(const char *target)
{
    return navigator_to_with_context(target, NULL);
}

ret_t navigator_to_with_context(const char *target, void *ctx)
{
    return_value_if_fail(target != NULL && * target != '\0', RET_BAD_PARAMS);

    return navigator_window_open_and_close(target, NULL, ctx);
}

ret_t navigator_replace(const char *target)
{
    widget_t *curr_win = NULL;
    widget_t *wm = window_manager();
    return_value_if_fail(target != NULL && * target != '\0', RET_BAD_PARAMS);

    curr_win = window_manager_get_top_main_window(wm);
    if (curr_win != NULL) {
        log_debug("close current window: %s\n", curr_win->name);
    }

    return navigator_window_open_and_close(target, curr_win, NULL);
}

ret_t navigator_replace_with_context(const char *target, void *ctx)
{
    widget_t *curr_win = NULL;
    widget_t *wm = window_manager();
    return_value_if_fail(target != NULL && * target != '\0', RET_BAD_PARAMS);

    curr_win = window_manager_get_top_main_window(wm);
    if (curr_win != NULL) {
        log_debug("close current window: %s\n", curr_win->name);
    }

    return navigator_window_open_and_close(target, curr_win, ctx);
}

ret_t navigator_switch_to(const char *target, bool_t close_current)
{
    widget_t *target_win;
    widget_t *wm = window_manager();
    return_value_if_fail(target != NULL && * target != '\0', RET_BAD_PARAMS);

    target_win = widget_child(wm, target);
    if (target_win != NULL) {
        widget_t *curr_win = window_manager_get_top_window(wm);
        return window_manager_switch_to(wm, curr_win, target_win, close_current);
    } else {
        return navigator_to(target);
    }
}

ret_t navigator_back_to_home(void)
{
    return window_manager_back_to_home(window_manager());
}

ret_t navigator_back(void)
{
    return window_manager_back(window_manager());
}

ret_t navigator_close(const char *target)
{
    widget_t *win;
    widget_t *wm = window_manager();
    return_value_if_fail(target != NULL && * target != '\0', RET_BAD_PARAMS);

    win = widget_child(wm, target);
    return_value_if_fail(win != NULL, RET_FAIL);

    return window_manager_close_window_force(wm, win);
}

ret_t navigator_request_close(const char *target)
{
    widget_t *win;
    widget_t *wm = window_manager();
    return_value_if_fail(target != NULL && * target != '\0', RET_BAD_PARAMS);

    win = widget_child(wm, target);
    return_value_if_fail(win != NULL, RET_FAIL);

    return widget_dispatch_simple_event(win, EVT_REQUEST_CLOSE_WINDOW);
}

ret_t navigator_global_widget_on(uint32_t type, event_func_t on_event, void *ctx)
{
    widget_t *wm = window_manager();
    widget_on(wm, type, on_event, ctx);

    return RET_OK;
}
