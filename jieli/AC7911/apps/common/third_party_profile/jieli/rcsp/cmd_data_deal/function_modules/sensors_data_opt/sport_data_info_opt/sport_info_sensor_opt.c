#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_info_sensor_opt.h"


#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/watch_common.h"

void sport_info_sensor_opt_attr_set(void *priv, u8 attr, u8 *data, u16 len)
{
    printf("%s", __func__);
    u8 func_switch = data[0];
    u8 sensors_switch = data[1];
    printf("sensor-switch %x", sensors_switch);
    struct watch_algo __algo_hd;
    watch_algo_handle_get(&__algo_hd);
    for (u8 i = SPORT_INFO_SWTICH_TYPE_SENSOR_PEDOMETER; i <= (SPORT_INFO_SWTICH_TYPE_SENSOR_ALTITUDE_RECORD - SPORT_INFO_SWTICH_TYPE_SENSOR_PEDOMETER + 1); i++) {
        __algo_hd.sensor_switch_set((i), sensors_switch & BIT(i - 1));
        sport_info_switch_record_update(i, sensors_switch & BIT(i - 1), 1);
    }

    sport_info_switch_record_update(SPORT_INFO_SWTICH_TYPE_SENSOR, func_switch, 1);
}

u32 sport_info_sensor_opt_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u32 switch_record = sport_info_swtich_record_get(-1);
    u8 switch_data[2] = {0};
    switch_data[0] = !!(switch_record & BIT(SPORT_INFO_SWTICH_TYPE_SENSOR));
    for (u8 i = 0; i <= (SPORT_INFO_SWTICH_TYPE_SENSOR_ALTITUDE_RECORD - SPORT_INFO_SWTICH_TYPE_SENSOR_PEDOMETER); i++) {
        if (switch_record & BIT(SPORT_INFO_SWTICH_TYPE_SENSOR_PEDOMETER + i)) {
            switch_data[1] |= BIT(i);
        }
    }
    rlen = add_one_attr(buf, buf_size, offset, attr, switch_data, sizeof(switch_data));
    return rlen;
}

#endif
