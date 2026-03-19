#include "rcsp_bt_func.h"
#include "rcsp_func_cmd_common.h"
#include "rcsp_function.h"
#include "rcsp_config.h"
#include "rcsp_event.h"
/* #include "app_action.h" *////RCSP TODO

#include "ble_rcsp_module.h"
#include "rcsp_music_info_setting.h"
#include "JL_rcsp_protocol.h"

#if (RCSP_MODE)

bool bt_func_set(void *priv, u8 *data, u16 len)
{
#if RCSP_ADV_MUSIC_INFO_ENABLE
    music_info_cmd_handle(data, len);
#endif
    return true;
}

u32 bt_func_get(void *priv, u8 *buf, u16 buf_size, u32 mask)
{
    u16 offset = 0;
    u16 music_sec = 0;
    u32 curr_music_sec = 0;
    u8 music_state = 0;
    u8 player_time[4];

#if RCSP_ADV_MUSIC_INFO_ENABLE
    if (mask & BIT(BT_INFO_ATTR_MUSIC_TITLE)) {
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_TITLE, (u8 *)get_music_title(), get_music_title_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_ARTIST)) {
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_ARTIST, (u8 *)get_music_artist(), get_music_artist_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_ALBUM)) {
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_ALBUM, (u8 *)get_music_album(), get_music_album_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_NUMBER)) {
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_NUMBER, (u8 *)get_music_number(), get_music_number_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_TOTAL)) {
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_TOTAL, (u8 *)get_music_total(),  get_music_total_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_GENRE)) {
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_GENRE, (u8 *)get_music_genre(), get_music_genre_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_TIME)) {
        music_sec = get_music_total_time();
        player_time[0] = music_sec >> 8;
        player_time[1] = music_sec;
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_TIME, player_time, 2);
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_STATE)) {
        //printf("\n music state\n");
        music_state = get_music_player_state();
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_STATE, &music_state, 1);
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_CURR_TIME)) {
        //printf("\nmusic curr time\n");
        curr_music_sec = get_music_curr_time();
        player_time[0] = curr_music_sec >> 24;
        player_time[1] = curr_music_sec >> 16;
        player_time[2] = curr_music_sec >> 8;
        player_time[3] = curr_music_sec;
        offset += add_one_attr(buf, buf_size, offset, BT_INFO_ATTR_MUSIC_CURR_TIME, player_time, 4);
    }
#endif

    return offset;
}

#endif
