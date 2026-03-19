#ifndef __RCSP_SWITCH_DEVICE_H__
#define __RCSP_SWITCH_DEVICE_H__

#include "typedef.h"
#include "app_config.h"

void switch_device(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
u8 get_rcsp_support_new_reconn_flag(void);

#endif // __RCSP_SWITCH_DEVICE_H__

