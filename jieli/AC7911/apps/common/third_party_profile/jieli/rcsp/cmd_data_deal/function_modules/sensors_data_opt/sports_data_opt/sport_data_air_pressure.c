#include "rcsp_config.h"
#include "sport_data_air_pressure.h"
#include "sport_data_func.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/altitude_presure.h"
enum {
    SPORT_DATA_REAL_AIR_PRESURE,
    SPORT_DATA_MIN_AIR_PRESURE,
    SPORT_DATA_MAX_AIR_PRESURE,

};


#define AIR_PRESSURE_DATA_MAX_LEN		600
static u8 *air_pressure_data = NULL;

u32 sport_data_func_attr_air_pressure_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    printf("%s", __func__);
    u32 rlen = 0;
    u16 data_len = 0;
    if (NULL == air_pressure_data) {
        air_pressure_data = zalloc(AIR_PRESSURE_DATA_MAX_LEN);

    }
    if (sub_mask & BIT(SPORT_DATA_REAL_AIR_PRESURE)) {
        u16 air_presure = get_data_air_presure();
        air_pressure_data[data_len++] = air_presure & 0xff;
        air_pressure_data[data_len++] = air_presure >> 8;

        printf("%x %x %x", air_presure, air_presure >> 8, air_presure & 0xff);
    }

    if (sub_mask & BIT(SPORT_DATA_MIN_AIR_PRESURE)) {
        u16 air_presure = get_min_air_presure();
        air_pressure_data[data_len++] = air_presure & 0xff;
        air_pressure_data[data_len++] = air_presure >> 8;
    }

    if (sub_mask & BIT(SPORT_DATA_MAX_AIR_PRESURE)) {
        u16 air_presure = get_max_air_presure();
        air_pressure_data[data_len++] = air_presure & 0xff;
        air_pressure_data[data_len++] = air_presure >> 8;
    }

    if (0xff == sub_mask) {
        data_len = AIR_PRESSURE_DATA_MAX_LEN;

        memset(air_pressure_data, 0, data_len);

        data_len = get_day_air_presure_data(air_pressure_data);

        for (int i = 0; i < data_len; i++) {
            printf("data=%d", air_pressure_data[i]);
        }
    }
    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, air_pressure_data, data_len);
    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, air_pressure_data, data_len, rlen, (void *)&air_pressure_data);
    return rlen;
}

#endif
