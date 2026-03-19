#ifndef __RCSP_FEATURE_H__
#define __RCSP_FEATURE_H__

#include "typedef.h"
#include "app_config.h"

u32 target_feature_parse_packet(void *priv, u8 *buf, u16 buf_size, u32 mask);

#endif//__RCSP_FEATURE_H__

