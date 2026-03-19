#ifndef TK_MUSIC_PLAYER_H
#define TK_MUSIC_PLAYER_H

#include "awtk.h"

BEGIN_C_DECLS

/**
 * @enum music_play_mode_t
 * 音乐播放模式。
 */
typedef enum _music_play_mode_t {
    /**
     * @const MUSIC_SEQUENTIAL_PLAY
     * 顺序播放。
     */
    MUSIC_SEQUENTIAL_PLAY = 0,
    /**
     * @const MUSIC_SINGLE_CYCLE
     * 单曲循环。
     */
    MUSIC_SINGLE_CYCLE,
    /**
     * @const MUSIC_RANDOM_PLAY
     * 随机播放。
     */
    MUSIC_RANDOM_PLAY,

    MUSIC_PLAY_MODE_MAX
} music_play_mode_t;

/**
 * @method music_play_mode_to_string
 * 将播放模式的枚举转化为字符串。
 * @return {const char*} 返回字符串象。
 */
const char *music_play_mode_to_string(music_play_mode_t mode);

/**
 * @class music_player_t
 * @annotation ["model", "single"]
 * 全局的音乐播放器对象。
 */
typedef struct _music_player_t {
    /**
     * @property {const char*} songs
     * @annotation ["readable", "writable"]
     */
    const char *songs;

    /**
     * @property {uint32_t} current_index
     * @annotation ["readable", "writable"]
     */
    uint32_t current_index;

    /**
     * @property {uint32_t} current_progress
     * @annotation ["readable", "writable"]
     */
    uint32_t current_progress;

    /**
     * @property {uint32_t} play_mode
     * @annotation ["readable", "writable"]
     */
    music_play_mode_t play_mode;

    /**
     * @property {uint32_t} volume
     * @annotation ["readable", "writable"]
     */
    uint32_t volume;

    /**
     * @property {bool_t} is_playing
     * @annotation ["readable", "writable"]
     */
    bool_t is_playing;

    uint32_t timer_id;

    /*private*/
} music_player_t;

/**
 * @method music_player
 * 获取缺省的music_player对象。
 * @return {tk_object_t*} 返回music_player对象。
 */
music_player_t *music_player(void);

/**
 * @method music_player_set
 * 设置缺省的music_player对象。
 * @param {music_player_t*} music_player 对象。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_set(music_player_t *music_player);

/**
 * @method music_player_create
 * @annotation ["constructor"]
 * 创建music_player对象。
 * @return {tk_object_t*} 返回music_player对象。
 */
music_player_t *music_player_create(void);

/**
 * @method music_player_to_prev
 * 切换到上一首。
 * @param {music_player_t*} music_player 对象。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_to_prev(music_player_t *music_player);

/**
 * @method music_player_to_next
 * 切换到上一首。
 * @param {music_player_t*} music_player 对象。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_to_next(music_player_t *music_player);

/**
 * @method music_player_switch_play_mode
 * 切换播放模式。
 * @param {music_player_t*} music_player 对象。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_switch_play_mode(music_player_t *music_player);

/**
 * @method music_player_set_volumn
 * 设置音乐播放器音量。
 * @param {music_player_t*} music_player 对象。
 * @param {uint32_t} volumn 音量。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_set_volumn(music_player_t *music_player, uint32_t volumn);

/**
 * @method music_player_pause
 * 设置音乐播放器暂停。
 * @param {music_player_t*} music_player 对象。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_pause(music_player_t *music_player);

/**
 * @method music_player_play
 * 设置音乐播放器播放。
 * @param {music_player_t*} music_player 对象。
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t music_player_play(music_player_t *music_player);

END_C_DECLS

#endif /*TK_MUSIC_PLAYER_H*/
