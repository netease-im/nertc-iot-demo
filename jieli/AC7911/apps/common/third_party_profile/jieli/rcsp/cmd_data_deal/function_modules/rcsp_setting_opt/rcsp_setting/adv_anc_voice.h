#ifndef __ADV_ANS_VOICE_H__
#define __ADV_ANS_VOICE_H__

#include "rcsp_cfg.h"
#include "app_config.h"

#if RCSP_ADV_EN


#define ANC_VOICE_TYPE_MAX		3

int anc_voice_setting_init(void);
int anc_voice_setting_sync(void);

// void deal_anc_voice(u8 *anc_setting, u8 write_vm, u8 tws_sync);
void set_anc_voice_info(u8 *anc_info);
void get_anc_voice_info(u8 *anc_info);

u8 anc_voice_info_get(u8 *data, u16 len);
u8 anc_voice_info_fetch_all_get(u8 *data, u16 len);
int anc_voice_info_set(u8 *data, u16 len);
int update_anc_voice_key_opt(void);
void rcsp_adv_voice_mode_update(u8 mode);

#endif // RCSP_ADV_EN

#endif
