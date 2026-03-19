#include "awtk.h"
#include "../common/navigator.h"
#include "../common/music_player.h"
#include "../custom_widgets/gesture.h"

static ret_t on_back(void *ctx, event_t *e)
{
    return navigator_back();
}

static ret_t update_music_data(widget_t *win)
{
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    widget_t *widget = NULL;
    music_player_t *player = music_player();

    widget = widget_lookup(win, "progress", TRUE);
    if (widget != NULL) {
        widget_set_value_int(widget, player->current_progress);
    }

    widget = widget_lookup(win, "song_name", TRUE);
    if (widget != NULL) {
        widget_set_text_utf8(widget, player->songs);
    }

    widget = widget_lookup(win, "mode", TRUE);
    if (widget != NULL) {
        widget_use_style(widget, music_play_mode_to_string(player->play_mode));
    }

    widget = widget_lookup(win, "volume", TRUE);
    if (widget != NULL) {
        widget_set_value_int(widget, player->volume);
    }

    widget = widget_lookup(win, "play", TRUE);
    if (widget != NULL) {
        if (player->is_playing) {
            widget_use_style(widget, "pause");
        } else {
            widget_use_style(widget, "play");
        }
    }

    return RET_OK;
}

static ret_t on_change_to_prev_song_animate(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = NULL;
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    music_player_to_prev(player);
    update_music_data(win);

    widget = widget_lookup(win, "song_name", TRUE);
    if (widget != NULL) {
        widget_start_animator(widget, "prev_enter");
    }

    return RET_OK;
}

static ret_t on_change_to_prev_song(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget;
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    widget = widget_lookup(win, "song_name", TRUE);
    if (widget != NULL) {
        widget_start_animator(widget, "prev_enter_before");
    }

    return RET_OK;
}

static ret_t on_change_to_next_song_animate(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = NULL;
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    music_player_to_next(player);
    update_music_data(win);

    widget = widget_lookup(win, "song_name", TRUE);
    if (widget != NULL) {
        widget_start_animator(widget, "next_enter");
    }

    return RET_OK;
}

static ret_t on_change_to_next_song(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = NULL;
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    widget = widget_lookup(win, "song_name", TRUE);
    if (widget != NULL) {
        widget_start_animator(widget, "next_enter_before");
    }

    return RET_OK;
}

static ret_t on_play_or_pause_music(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(e->target);
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    if (widget != NULL) {
        if (player->is_playing) {
            music_player_pause(player);
        } else {
            music_player_play(player);
        }

        update_music_data(win);
    }

    return RET_OK;
}


static ret_t on_change_play_mode(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(e->target);
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    if (widget != NULL) {
        music_player_switch_play_mode(player);
        update_music_data(win);
    }

    return RET_OK;
}

static ret_t on_change_volume(void *ctx, event_t *e)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(e->target);
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    if (widget != NULL) {
        music_player_set_volumn(player, widget_get_value(widget));
        update_music_data(win);
    }

    return RET_OK;
}

static ret_t on_update_data(const timer_info_t *timer)
{
    widget_t *win = WIDGET(timer->ctx);

    update_music_data(win);

    return RET_REPEAT;
}

/**
 * 初始化窗口的子控件
 */
static ret_t visit_init_child(void *ctx, const void *iter)
{
    widget_t *win = WIDGET(ctx);
    widget_t *widget = WIDGET(iter);
    const char *name = widget->name;
    music_player_t *player = music_player();
    ENSURE(player != NULL);

    // 初始化指定名称的控件（设置属性或注册事件），请保证控件名称在窗口上唯一
    if (name != NULL && *name != '\0') {
        if (tk_str_eq(name, "gesture")) {
            widget_on(widget, EVT_SLIDE_RIGHT, on_back, win);
        } else if (tk_str_eq(name, "prev_song")) {
            widget_on(widget, EVT_CLICK, on_change_to_prev_song, win);
        } else if (tk_str_eq(name, "play")) {
            widget_on(widget, EVT_CLICK, on_play_or_pause_music, win);
            if (player->is_playing) {
                widget_use_style(widget, "pause");
            }
        } else if (tk_str_eq(name, "next_song")) {
            widget_on(widget, EVT_CLICK, on_change_to_next_song, win);
        } else if (tk_str_eq(name, "mode")) {
            widget_on(widget, EVT_CLICK, on_change_play_mode, win);
            widget_use_style(widget, music_play_mode_to_string(player->play_mode));
        } else if (tk_str_eq(name, "volume")) {
            widget_on(widget, EVT_VALUE_CHANGED, on_change_volume, win);
            widget_set_value_int(widget, player->volume);
        } else if (tk_str_eq(name, "progress")) {
            widget_set_value_int(widget, player->current_progress);
        } else if (tk_str_eq(name, "song_name")) {
            widget_set_text_utf8(widget, player->songs);

            widget_animator_t *wa = NULL;
            wa = widget_find_animator(widget, "next_enter_before");
            if (wa != NULL) {
                widget_animator_on(wa, EVT_ANIM_END, on_change_to_next_song_animate, win);
            }

            wa = widget_find_animator(widget, "prev_enter_before");
            if (wa != NULL) {
                widget_animator_on(wa, EVT_ANIM_END, on_change_to_prev_song_animate, win);
            }
        }
    }

    return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t music_init(widget_t *win, void *ctx)
{
    (void)ctx;
    return_value_if_fail(win != NULL, RET_BAD_PARAMS);

    if (music_player() == NULL) {
        music_player_t *player = music_player_create();
        if (player == NULL) {
            log_debug("create music player failed\n");
        } else {
            music_player_set(player);
        }
    }

    widget_foreach(win, visit_init_child, win);
    update_music_data(win);
    widget_add_timer(win, on_update_data, 1000);

    return RET_OK;
}
