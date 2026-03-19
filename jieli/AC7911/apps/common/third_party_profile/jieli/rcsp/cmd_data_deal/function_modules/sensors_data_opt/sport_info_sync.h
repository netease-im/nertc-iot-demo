#ifndef __SPORT_INFO_SYNC_H__
#define __SPORT_INFO_SYNC_H__

#include "typedef.h"
#include "app_config.h"

int JL_rcsp_sports_info_sync_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
int sport_info_sync_keep_exercise_resp(void);
int sport_info_sync_pause_exercise_resp(void);
int sport_info_sync_end_exercise_by_fw(void);

// 当data是NULL时，就表示固件主动推送数据给app
int sport_info_sync_start_exercise_resp(u8 *data, u16 *data_len);

#endif
