#ifndef _RCSP_EXTRA_FLASH_CMD_H_
#define _RCSP_EXTRA_FLASH_CMD_H_
#include "typedef.h"
#include "event/event.h"///RCSP TODO

int JL_rcsp_extra_flash_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
int JL_rcsp_extra_flash_cmd_no_resp(void *priv, u8 OpCode, u8 *data, u16 len);
u8 JL_rcsp_extra_flash_info_notify(u8 *buf, u16 len);

int rcsp_extra_flash_opt_resp(u8 dire, u8 OpCode, u8 OpCode_SN, u8 *resp_data, u16 data_len);

#endif
