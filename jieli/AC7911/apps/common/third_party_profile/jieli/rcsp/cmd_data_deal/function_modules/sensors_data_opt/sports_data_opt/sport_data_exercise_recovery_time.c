#include "rcsp_config.h"
#include "sport_data_exercise_recovery_time.h"
#include "sport_data_func.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)

#define EXERCISE_RECOVERY_TIME_DATA_MAX_LEN		0
static u8 *exercise_recovery_time_data = NULL;

u32 sport_data_func_attr_exercise_recovery_time_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u16 data_len = 0;

    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, exercise_recovery_time_data, data_len);
    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, exercise_recovery_time_data, data_len, rlen, (void *)&exercise_recovery_time_data);
    return rlen;
}

#endif
