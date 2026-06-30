#ifndef __BK_SMART_CONFIG_NERTC_ADAPTER_H__
#define __BK_SMART_CONFIG_NERTC_ADAPTER_H__

typedef struct
{
    uint8_t valid;
    char appid[33];
    char channel_name[128];
} bk_sconf_agent_info_t;

typedef enum
{
	HTTP_STATUS_SUCCESS = 200,
	HTTP_STATUS_PARAM_ERROR = 400,
	HTTP_STATUS_MAX_AGENT_UPTIME_EXCEEDED = 403,
	HTTP_STATUS_TRIAL_LIMIT_EXCEEDED = 404,
	HTTP_STATUS_DEVICE_REMOVED = 405,
	HTTP_STATUS_AGENT_START_FAILED = 406,
}agent_status_code;


void bk_app_notify_wakeup_event(void);
uint16_t bk_sconf_send_agent_info(char *payload, uint16_t max_len);
void  bk_sconf_prase_agent_info(char *payload, uint8_t reset);
void bk_sconf_erase_agent_info(void);
int bk_sconf_save_agent_info(char *appid, char *channel_name);
int bk_sconf_get_agent_info(bk_sconf_agent_info_t *info);
int bk_sconf_wakeup_agent(uint8_t reset);
int bk_sconf_upate_agent_info(char *update_info);
void bk_sconf_begin_to_switch_ir_mode(void);
int bk_sconf_post_nfc_id(uint8_t *nfc_id);
#endif
