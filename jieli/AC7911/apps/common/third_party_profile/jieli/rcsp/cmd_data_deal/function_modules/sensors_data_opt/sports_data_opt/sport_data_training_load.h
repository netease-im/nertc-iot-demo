#ifndef __RCSP_SPORT_DATA_TRAINING_LOAD_H__
#define __RCSP_SPORT_DATA_TRAINING_LOAD_H__

#include "typedef.h"
#include "app_config.h"

u32 sport_data_func_attr_training_load_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset);
#endif
