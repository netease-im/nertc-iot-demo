#ifndef __RCSP_ADV_BLUETOOTH__
#define __RCSP_ADV_BLUETOOTH__
#include "typedef.h"
#include "event/event.h"///RCSP TODO


// enum RCSP_ADV_MSG_T {
//     MSG_JL_ADV_SETTING_SYNC,
//     MSG_JL_ADV_SETTING_UPDATE,
// };
//
// bool rcsp_adv_msg_post(int msg, int argc, ...);
int JL_rcsp_adv_event_handler(struct rcsp_event *rcsp);
int JL_rcsp_adv_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
u8 adv_info_device_request(u8 *buf, u16 len);

#endif
