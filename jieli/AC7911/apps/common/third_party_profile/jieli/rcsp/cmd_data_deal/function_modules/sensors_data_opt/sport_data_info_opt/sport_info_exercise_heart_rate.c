#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_exercise_heart_rate.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "watch_common.h"
#include "sport_data/detection_response.h"

void sport_info_exercise_heart_rate_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    u8 heart_rate_switch = data[0];
    e_heart_rate heart_rate = {0};
    memcpy(&heart_rate, data + 1, sizeof(heart_rate));
    struct watch_algo __watch_algo;
    watch_algo_handle_get(&__watch_algo);
    int arg[5];
    arg[0] = EXERCISE_HEART_RATE;
    arg[1] = heart_rate_switch;
    arg[2] = SCREEN_LIGHT;
    arg[3] = heart_rate.heart_rate_type;
    arg[4] = heart_rate.max_heart_rate;
    __watch_algo.detection_ioctrl(5, arg);
    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_EXERCISE_HEART_RATE, heart_rate_switch, 1);
    sport_info_write_vm(VM_SPORT_INFO_EXERCISE_HEART_RATE, (u8 *)&heart_rate, sizeof(e_heart_rate));
}

u32 sport_info_exercise_heart_rate_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;

    u8 heart_rate_data[3] = {0};
    heart_rate_data[0] = !!sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_EXERCISE_HEART_RATE);

    sport_exercise_heart_rate_get((e_heart_rate *)(heart_rate_data + 1));
    rlen = add_one_attr(buf, buf_size, offset, attr, heart_rate_data, sizeof(heart_rate_data));

    return rlen;
}

#endif

