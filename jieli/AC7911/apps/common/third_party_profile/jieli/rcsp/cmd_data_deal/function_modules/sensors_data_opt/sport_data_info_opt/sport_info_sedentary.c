#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_sedentary.h"


#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "watch_common.h"
#include "sport_data/detection_response.h"

void sport_info_sedentary_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    u8 sedentary_switch = data[0];
    u8 sedentary_mode = data[1];

    sedentary_t sedentary = {0};
    memcpy(&sedentary, data + 2, sizeof(sedentary_t));
    sedentary.nop_mode = data [2];
    u8 time[4];
    time[0] = sedentary.begin_time_hour;
    time[1] = sedentary.begin_time_min;
    time[2] = sedentary.end_time_hour;
    time[3] = sedentary.end_time_min;
    struct watch_algo algo_hd;
    watch_algo_handle_get(&algo_hd);
    int arg[6];
    arg[0] = SEDENTARY;
    arg[1] = sedentary_switch;
    arg[2] = sedentary_mode;
    arg[3] = (time[0] << 24 | time[1] << 16 | time[2] << 8 | time[3]);
    arg[4] = sedentary.nop_mode;
    algo_hd.detection_ioctrl(5, arg);

    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_SEDENTARY, sedentary_switch, 1);
    sport_info_mode_record_update(SPORT_INFO_MODE_TYPE_SEDENTARY, sedentary_mode);
    sport_info_write_vm(VM_SPORT_INFO_SEDENTARY, (u8 *)&sedentary, sizeof(sedentary_t));
}

u32 sport_info_sedentary_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 sedentary_data[2 + 5] = {0};
    sedentary_data[0] = !!sport_info_swtich_record_get(SPORT_INFO_SWTICH_TYPE_SEDENTARY);

    u8 *mode_data = NULL;
    u16 mode_len = sport_info_record_get(SPORT_INFO_MODE_TYPE_SEDENTARY, &mode_data);

    if (mode_data && mode_len) {
        sedentary_data[1] = mode_data[0];
    }
    sport_sedentary_get((sedentary_t *)(sedentary_data + 2));
    rlen = add_one_attr(buf, buf_size, offset, attr, sedentary_data, sizeof(sedentary_data));

    return rlen;
}

#endif
