#include "rcsp_config.h"
#include "sport_data_exercise_steps.h"
#include "sport_data_func.h"
#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#include "sport_data/watch_common.h"
enum {
    SPORT_DATA_REAL_TIME_STEP,
    SPORT_DATA_REAL_TIME_DISTANCE,
    SPORT_DATA_REAL_TIME_KCAL
};
#define EXERCISE_STEPS_DATA_MAX_LEN	40
static u8 *exercise_steps_data = NULL;

u32 sport_data_func_attr_exercise_steps_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    struct watch_execise __execise_hd;
    watch_execise_handle_get(&__execise_hd);
    struct motion_info __motion_info;
    __execise_hd.daily_motion_info_get(&__motion_info);
    u32 rlen = 0;
    u16 data_len = 0;
    if (NULL == exercise_steps_data) {
        exercise_steps_data = zalloc(EXERCISE_STEPS_DATA_MAX_LEN);
    }

    if (sub_mask & BIT(SPORT_DATA_REAL_TIME_STEP)) {
        int step = __motion_info.step;
        exercise_steps_data[data_len++] = (step >> 24) & 0xff ;
        exercise_steps_data[data_len++] = (step >> 16) & 0xff ;
        exercise_steps_data[data_len++] = (step >> 8) & 0xff ;
        exercise_steps_data[data_len++] =  step & 0xff;
    }

    if (sub_mask & BIT(SPORT_DATA_REAL_TIME_DISTANCE)) {
        u16 distance = __motion_info.step_distance;
        printf("%s %d", __func__, distance);
        exercise_steps_data[data_len++] = (distance >> 8) & 0xff ;
        exercise_steps_data[data_len++] =  distance & 0xff;
    }
    if (sub_mask & BIT(SPORT_DATA_REAL_TIME_KCAL)) {
        u16 kcal = __motion_info.caloric;
        exercise_steps_data[data_len++] = (kcal >> 8) & 0xff ;
        exercise_steps_data[data_len++] =  kcal & 0xff;
    }

    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, exercise_steps_data, data_len);
    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, exercise_steps_data, data_len, rlen, (void *)&exercise_steps_data);
    return rlen;
}

#endif
