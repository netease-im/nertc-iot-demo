#include "music_player.h"

static char *s_songs[] = { "Shape of you", "That Girl",           "Listen",
                           "Wolves",       "Trouble is a firend", "Fire"
                         };

const char *music_play_mode_to_string(music_play_mode_t mode)
{
    switch (mode) {
    case MUSIC_SEQUENTIAL_PLAY:
        return "sequential_play";
    case MUSIC_SINGLE_CYCLE:
        return "single_cycle";
    case MUSIC_RANDOM_PLAY:
        return "random_play";
    default:
        return "sequential_play";
    }
}

static music_player_t *s_music_player = NULL;

music_player_t *music_player(void)
{
    return s_music_player;
}

ret_t music_player_set(music_player_t *music_player)
{
    s_music_player = music_player;

    return RET_OK;
}

music_player_t *music_player_create(void)
{
    music_player_t *player = TKMEM_ZALLOC(music_player_t);
    return_value_if_fail(player != NULL, NULL);

    player->volume = 40;
    player->is_playing = FALSE;
    player->timer_id = TK_INVALID_ID;
    player->play_mode = MUSIC_SEQUENTIAL_PLAY;
    player->current_index = 0;
    player->songs = s_songs[player->current_index];

    return player;
}

static ret_t music_player_on_update_progress_timer(const timer_info_t *timer)
{
    music_player_t *music_player = (music_player_t *)(timer->ctx);
    return_value_if_fail(music_player != NULL, RET_REPEAT);

    music_player->current_progress += 1;

    if (music_player->current_progress >= 100) {
        music_player_to_next(music_player);
    }

    return RET_REPEAT;
}

static ret_t music_player_start_progress_timer(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    if (music_player->timer_id == TK_INVALID_ID) {
        music_player->timer_id = timer_add(music_player_on_update_progress_timer, music_player, 1000);
    }

    return RET_OK;
}

static ret_t music_player_stop_progress_timer(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    if (music_player->timer_id != TK_INVALID_ID) {
        timer_remove(music_player->timer_id);
        music_player->timer_id = TK_INVALID_ID;
    }

    return RET_OK;
}

ret_t music_player_to_prev(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    if (music_player->play_mode == MUSIC_RANDOM_PLAY) {
        music_player->current_index = random() % 6;
    } else {
        music_player->current_index = (music_player->current_index + 1) % 6;
    }

    music_player->is_playing = TRUE;
    music_player->songs = s_songs[music_player->current_index];
    music_player->current_progress = 0;
    music_player_start_progress_timer(music_player);

    return RET_OK;
}

ret_t music_player_to_next(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    if (music_player->play_mode == MUSIC_RANDOM_PLAY) {
        music_player->current_index = random() % 6;
    } else {
        music_player->current_index = (music_player->current_index - 1) % 6;
    }

    music_player->is_playing = TRUE;
    music_player->songs = s_songs[music_player->current_index];
    music_player->current_progress = 0;
    music_player_start_progress_timer(music_player);

    return RET_OK;
}

ret_t music_player_switch_play_mode(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    music_player->play_mode = (music_player->play_mode + 1) % MUSIC_PLAY_MODE_MAX;

    return RET_OK;
}

ret_t music_player_set_volumn(music_player_t *music_player, uint32_t volumn)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    music_player->volume = volumn;

    return RET_OK;
}

ret_t music_player_pause(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    music_player->is_playing = FALSE;
    music_player_stop_progress_timer(music_player);

    return RET_OK;
}

ret_t music_player_play(music_player_t *music_player)
{
    return_value_if_fail(music_player != NULL, RET_BAD_PARAMS);

    music_player->is_playing = TRUE;
    music_player_start_progress_timer(music_player);

    return RET_OK;
}
