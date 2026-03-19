#include "rcsp_config.h"
#include "sport_data_altitude.h"
#include "sport_data_func.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/altitude_presure.h"
enum {
    SPORT_DATA_REAL_ALTITUDE,
    SPORT_DATA_MIN_ALTITUDE,
    SPORT_DATA_MAX_ALTITUDE,

};
#define ALTITUDE_DATA_MAX_LEN		1200
static u8 *altitude_data = NULL;

u32 sport_data_func_attr_altitude_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    printf("%s", __func__);
    u32 rlen = 0;
    u16 data_len = 0;
    if (NULL == altitude_data) {
        altitude_data = zalloc(ALTITUDE_DATA_MAX_LEN);
    }
    if (sub_mask & BIT(SPORT_DATA_REAL_ALTITUDE)) {
        u32 altitude = get_data_altitude();
        altitude_data[data_len++] = (altitude) & 0xff;
        altitude_data[data_len++] = (altitude >> 8) & 0xff;
        altitude_data[data_len++] = (altitude >> 16) & 0xff;
        altitude_data[data_len++] = (altitude >> 24) & 0xff;

    }

    if (sub_mask & BIT(SPORT_DATA_MIN_ALTITUDE)) {
        u32 altitude = get_min_altitude();
        altitude_data[data_len++] = (altitude) & 0xff;
        altitude_data[data_len++] = (altitude >> 8) & 0xff;
        altitude_data[data_len++] = (altitude >> 16) & 0xff;
        altitude_data[data_len++] = (altitude >> 24) & 0xff;
    }

    if (sub_mask & BIT(SPORT_DATA_MAX_ALTITUDE)) {
        u32 altitude = get_max_altitude();
        altitude_data[data_len++] = (altitude) & 0xff;
        altitude_data[data_len++] = (altitude >> 8) & 0xff;
        altitude_data[data_len++] = (altitude >> 16) & 0xff;
        altitude_data[data_len++] = (altitude >> 24) & 0xff;
    }
    if (0xff == sub_mask) {
        data_len = ALTITUDE_DATA_MAX_LEN;

        memset(altitude_data, 0, data_len);
        data_len = get_day_altitude_data(altitude_data);
        printf("altitude_data_len=%d", data_len);
        for (int i = 0; i < data_len; i++) {
            printf("data=%d", altitude_data[i]);
        }
    }


    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, altitude_data, data_len);
    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, altitude_data, data_len, rlen, (void *)&altitude_data);
    return rlen;
}

#endif
