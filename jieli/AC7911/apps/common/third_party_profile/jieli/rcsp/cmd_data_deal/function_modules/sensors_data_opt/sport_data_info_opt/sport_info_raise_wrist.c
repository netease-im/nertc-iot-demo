#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_raise_wrist.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "watch_common.h"
#include "sport_data/detection_response.h"

enum {
    SPORT_INFO_RAISE_WRIST_TURN_OFF,
    SPORT_INFO_RAISE_WRIST_TURN_ON_ALL_DATA,
    SPORT_INFO_RAISE_WRIST_TURN_CUSTOM_TIME_PERIOD,
};

void sport_info_raise_wrist_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    printf("%s", __func__);
    u8 raise_wrist_switch = data[0];
    u8 raise_wrist_mode = data[1];

    raise_wrist_t raise_wrist = {0};
    memcpy(&raise_wrist, data + 2, sizeof(raise_wrist_t));
    u8 time[4];
    time[0] = raise_wrist.begin_time_hour;
    time[1] = raise_wrist.begin_time_min;
    time[2] = raise_wrist.end_time_hour;
    time[3] = raise_wrist.end_time_min;

    struct watch_algo algo_hd;
    watch_algo_handle_get(&algo_hd);
    int arg[6];
    arg[0] = WRIST;
    arg[1] = raise_wrist_switch;
    arg[2] = raise_wrist_mode;
    arg[3] = (time[0] << 24 | time[1] << 16 | time[2] << 8 | time[3]);
    algo_hd.detection_ioctrl(4, arg);

    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_RAISE_WRIST, SPORT_INFO_RAISE_WRIST_TURN_ON_ALL_DATA == raise_wrist_switch, 0);
    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_RAISE_WRIST_CUSTOMIZE, SPORT_INFO_RAISE_WRIST_TURN_CUSTOM_TIME_PERIOD == raise_wrist_switch, 1);

    sport_info_mode_record_update(SPORT_INFO_MODE_TYPE_RAISE_WRIST, raise_wrist_mode);
    sport_info_write_vm(VM_SPORT_INFO_RAISE_WRIST, (u8 *)&raise_wrist, sizeof(raise_wrist_t));
}

u32 sport_info_raise_wrist_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;

    u8 raise_wrist_data[2 + 4] = {0};
    if (sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_RAISE_WRIST)) {
        raise_wrist_data[0] = SPORT_INFO_RAISE_WRIST_TURN_ON_ALL_DATA;
    } else if (sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_RAISE_WRIST_CUSTOMIZE)) {
        raise_wrist_data[0] = SPORT_INFO_RAISE_WRIST_TURN_CUSTOM_TIME_PERIOD;
    }

    u8 *mode_data = NULL;
    u16 mode_len = sport_info_record_get(SPORT_INFO_MODE_TYPE_RAISE_WRIST, &mode_data);
    if (mode_data && mode_len) {
        raise_wrist_data[1] = mode_data[0];
    }
    sport_raise_wrist_get((raise_wrist_t *)(raise_wrist_data + 2));
    rlen = add_one_attr(buf, buf_size, offset, attr, raise_wrist_data, sizeof(raise_wrist_data));

    return rlen;
}

#endif
