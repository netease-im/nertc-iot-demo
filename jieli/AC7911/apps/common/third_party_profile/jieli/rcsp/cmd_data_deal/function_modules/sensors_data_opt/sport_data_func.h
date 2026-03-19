#ifndef __RCSP_SPORT_DATA_FUNC_H__
#define __RCSP_SPORT_DATA_FUNC_H__

#include "typedef.h"
#include "app_config.h"

enum {
    SPORTS_DATA_FUNC_ATTR_TYPE_HEART_RATE,
    SPORTS_DATA_FUNC_ATTR_TYPE_AIR_PRESSURE,
    SPORTS_DATA_FUNC_ATTR_TYPE_ALTITUDE,
    SPORTS_DATA_FUNC_ATTR_TYPE_EXERCISE_STEPS,
    SPORTS_DATA_FUNC_ATTR_TYPE_PRESSURE_DETECTION,
    SPORTS_DATA_FUNC_ATTR_TYPE_BLOOD_OXYGEN,
    SPORTS_DATA_FUNC_ATTR_TYPE_TRAINING_LOAD,
    SPORTS_DATA_FUNC_ATTR_TYPE_MAX_OXYGEN_UPTAKE,
    SPORTS_DATA_FUNC_ATTR_TYPE_EXERCISE_RECOVERY_TIME,
    SPORTS_DATA_FUNC_ATTR_TYPE_SPORTS_INFORMATION,
    SPORTS_DATA_FUNC_ATTR_TYPE_GET_MAX,
};

enum {
    DATA_FUNC_ATTR_TYPE_LOCATION,
    DATA_FUNC_ATTR_TYPE_WEATHER,
    DATA_FUNC_ATTR_TYPE_NOTICE,
    DATA_FUNC_ATTR_TYPE_NOTICE_REMOVE,
    DATA_FUNC_ATTR_TYPE_SET_MAX,
};

typedef struct sport_resp_data_t {
    u16 resp_data_len;
    u8  resp_mask;
    u8  resp_sub_mask;
    u8  *resp_data;
    int (*call_back)(void *priv);
    void *priv;
} sport_resp_data;

typedef u32(*sport_attr_get_func)(void *priv, u8 attr, u8 type, u8 *buf, u16 buf_size, u32 offset);

int JL_rcsp_sports_data_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
void sport_data_func_update(u32 mask, u8 *sub_mask);
void sport_data_global_var_deal(void *priv, u8 type, u8 sub_mask, u8 *data, u16 data_len, u8 result_len, void *callback_param);
u16 add_one_attr_with_submask_huge(u8 *buf, u16 max_len, u8 offset, u8 type, u8 sub_mask, u8 *data, u16 size);
void sport_data_func_get_prepare_deal(void);
void sport_data_func_get_finish_deal(void);
// #if (RCSP_MODE)
// #define RCSP_UPDATE					sport_data_func_update
// #else
// #define RCSP_UPDATE(...)
// #endif

#endif
