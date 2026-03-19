#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_sleep_detection.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "watch_common.h"
#include "sport_data/detection_response.h"

enum {
    SPORT_INFO_SLEEP_DETECTION_TURN_OFF,
    SPORT_INFO_SLEEP_DETECTION_TURN_ON_ALL_DATA,
    SPORT_INFO_SLEEP_DETECTION_CUSTOM_TIME_PERIOD,
};

void sport_info_sleep_detection_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    u8 sleep_switch = data[0];
    sleep_detection_t sleep_detection = {0};
    memcpy(&sleep_detection, data + 1, sizeof(sleep_detection_t));
#if 0
    extern void sleep_file_test(void);
    extern void step_test(void);
    if (sleep_switch == 1) {
        sleep_file_test();
        step_test();
    }
#endif
    u8 time[4];
    time[0] = sleep_detection.begin_time_hour;
    time[1] = sleep_detection.begin_time_min;
    time[2] = sleep_detection.end_time_hour;
    time[3] = sleep_detection.end_time_min;
    struct watch_algo algo_hd;
    watch_algo_handle_get(&algo_hd);
    int arg[6];
    arg[0] = SLEEP;
    arg[1] = sleep_switch;
    arg[2] = -1;
    arg[3] = (time[0] << 24 | time[1] << 16 | time[2] << 8 | time[3]);
    algo_hd.detection_ioctrl(4, arg);

    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_SLEEP_DETECTION, SPORT_INFO_SLEEP_DETECTION_TURN_ON_ALL_DATA == sleep_switch, 0);
    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_SLEEP_DETECTION_CUSTOMIZE, SPORT_INFO_SLEEP_DETECTION_CUSTOM_TIME_PERIOD == sleep_switch, 1);
    sport_info_write_vm(VM_SPORT_INFO_SLEEP_DETECTION, (u8 *)&sleep_detection, sizeof(sleep_detection_t));
}

u32 sport_info_sleep_detection_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;

    u8 press_data[1 + 4] = {0};
    if (sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_SLEEP_DETECTION)) {
        press_data[0] = SPORT_INFO_SLEEP_DETECTION_TURN_ON_ALL_DATA;
    } else if (sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_SLEEP_DETECTION_CUSTOMIZE)) {
        press_data[0] = SPORT_INFO_SLEEP_DETECTION_CUSTOM_TIME_PERIOD;
    }

    sport_sleep_detection_get((sleep_detection_t *)(press_data + 1));
    rlen = add_one_attr(buf, buf_size, offset, attr, press_data, sizeof(press_data));

    return rlen;
}

#endif

