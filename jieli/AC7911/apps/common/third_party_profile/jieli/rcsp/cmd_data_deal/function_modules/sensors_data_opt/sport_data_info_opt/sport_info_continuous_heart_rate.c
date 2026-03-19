#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_continuous_heart_rate.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)

#include "sport_data/detection_response.h"
#include "watch_common.h"
void sport_info_continuous_heart_rate_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    u8 heart_rate_switch = data[0];
    u8 heart_rate_mode = data[1];
#if C_HEART_TEST
    u8 test_switch = data[2];
    printf("%s test_switch=%d", __func__, test_switch);
    extern void c_heart_rate(u8 enable);
    c_heart_rate(test_switch);
#endif
    struct watch_algo __watch_algo;
    watch_algo_handle_get(&__watch_algo);
    int arg[5];
    arg[0] = CONTINUE_HEART_RATE;
    arg[1] = heart_rate_switch;
    arg[2] = SCREEN_LIGHT;
    arg[3] = heart_rate_mode;
    __watch_algo.detection_ioctrl(4, arg);
    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_CONTINUOUS_HEART_RATE, heart_rate_switch, 1);
    sport_info_mode_record_update(SPORT_INFO_MODE_TYPE_CONTINUOUS_HEART_RATE, heart_rate_mode);
}

u32 sport_info_continuous_heart_rate_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 heart_rate_data[2] = {0};
    heart_rate_data[0] = !!sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_CONTINUOUS_HEART_RATE);

    u8 *mode_data = NULL;
    u16 mode_len = sport_info_record_get(SPORT_INFO_MODE_TYPE_CONTINUOUS_HEART_RATE, &mode_data);
    if (mode_data && mode_len) {
        heart_rate_data[1] = mode_data[0];
    }
    rlen = add_one_attr(buf, buf_size, offset, attr, heart_rate_data, sizeof(heart_rate_data));

    return rlen;
}

#endif
