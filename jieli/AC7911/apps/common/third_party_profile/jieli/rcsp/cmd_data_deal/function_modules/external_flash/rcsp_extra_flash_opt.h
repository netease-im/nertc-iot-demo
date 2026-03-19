#ifndef _RCSP_EXTRA_FLASH_OPT_H_
#define _RCSP_EXTRA_FLASH_OPT_H_
#include "typedef.h"
#include "event/event.h"///RCSP TODO

int rcsp_extra_flash_opt(u8 *data, u16 len, u8 OpCode, u8 OpCode_SN);
int get_extra_flash_info(void *priv, u8 *resp_data);
int rcsp_extra_flash_opt_start(void);
void rcsp_extra_flash_opt_stop(void);

// 其他地方会调用到
void rcsp_extra_flash_opt_dial_nodify(void);
void rcsp_eflash_flag_set(u8 eflash_state_type);
u8 rcsp_eflash_flag_get(void);
void rcsp_eflash_update_flag_set(u8 eflash_state_type);
u8 rcsp_eflash_update_flag_get(void);

void rcsp_extra_flash_init(void);
int rcsp_extra_flash_event_deal(struct sys_event *event);
void rcsp_extra_flash_close(void);

void rcsp_extra_flash_disconnect_tips(u32 sec);
#endif
