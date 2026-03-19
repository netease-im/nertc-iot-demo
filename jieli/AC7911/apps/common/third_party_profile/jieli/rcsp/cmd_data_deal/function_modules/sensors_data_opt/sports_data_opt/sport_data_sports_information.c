#include "rcsp_config.h"
#include "sport_data_sports_information.h"
#include "sport_data_func.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/watch_common.h"

#define SPORTS_INFORMATION_DATA_MAX_LEN		50
static u8 *sports_information_data = NULL;
enum {
    SPORT_DATA_SPORT_STATUS,
    SPORT_DATA_SPORT_DISTANCE,
    SPORT_DATA_SPORT_START_TIME,
    SPORT_DATA_SPORT_END_TIME,
    SPORT_DATA_SPORT_KCAL,
    SPORT_DATA_SPORT_SPEED,

};
u32 sport_data_func_attr_sports_information_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    struct watch_execise __execise_hd;
    watch_execise_handle_get(&__execise_hd);
    struct motion_info __execise_info;
    __execise_hd.execise_info_get(&__execise_info);
    u32 rlen = 0;
    u16 data_len = 0;
    if (NULL == sports_information_data) {
        sports_information_data = zalloc(SPORTS_INFORMATION_DATA_MAX_LEN);
    }
    if (sub_mask & BIT(SPORT_DATA_SPORT_STATUS)) {
        sports_information_data[data_len++] = __execise_hd.execise_ctrl_status_get();
    }
    if (sub_mask & BIT(SPORT_DATA_SPORT_DISTANCE)) {
        u16 distance = __execise_info.step_distance;
        sports_information_data[data_len++] = (distance >> 8) & 0xff;
        sports_information_data[data_len++] = distance    & 0xff;
    }
    if (sub_mask & BIT(SPORT_DATA_SPORT_START_TIME)) {
        u32 start_time = get_sport_start_time(NULL);
        sports_information_data[data_len++] = (start_time >> 24) & 0xff;
        sports_information_data[data_len++] = (start_time >> 16) & 0xff;
        sports_information_data[data_len++] = (start_time >> 8)  & 0xff;
        sports_information_data[data_len++] = start_time      & 0xff;
    }
    if (sub_mask & BIT(SPORT_DATA_SPORT_END_TIME)) {
        u32 end_time = get_sport_end_time(NULL);
        sports_information_data[data_len++] = (end_time >> 24) & 0xff;
        sports_information_data[data_len++] = (end_time >> 16) & 0xff;
        sports_information_data[data_len++] = (end_time >> 8)  & 0xff;
        sports_information_data[data_len++] = end_time      & 0xff;
    }
    if (sub_mask & BIT(SPORT_DATA_SPORT_KCAL)) {
        u16 kcal = __execise_info.caloric;
        sports_information_data[data_len++] = (kcal >> 8) & 0xff;
        sports_information_data[data_len++] = (kcal)    & 0xff;
    }
    if (sub_mask & BIT(SPORT_DATA_SPORT_SPEED)) {
        u16 speed = __execise_info.motion_speed;
        sports_information_data[data_len++] = speed >> 8 & 0xff;
        sports_information_data[data_len++] = speed & 0xff;
    }



    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, sports_information_data, data_len);
    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, sports_information_data, data_len, rlen, (void *)&sports_information_data);
    return rlen;
}

#endif
