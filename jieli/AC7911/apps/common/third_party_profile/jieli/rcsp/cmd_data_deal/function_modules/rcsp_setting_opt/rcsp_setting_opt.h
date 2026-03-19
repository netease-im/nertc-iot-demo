#ifndef __RCSP_SETTING_OPT_H__
#define __RCSP_SETTING_OPT_H__

#include "system/includes.h"

#define REGISTER_APP_SETTING_OPT(rcsp_opt) \
int rcsp_opt##_setting_init(void) {\
   return register_rcsp_setting_opt_setting(&rcsp_opt);\
}\
late_initcall(rcsp_opt##_setting_init);

#pragma pack(1)
typedef struct _RCSP_SETTING_OPT {
    struct _RCSP_SETTING_OPT *next;
    bool need_opt;
    u32 data_len;
    int setting_type;
    int syscfg_id;
    int (*get_setting)(u8 *setting_data);
    void (*set_setting)(u8 *setting_data);
    void (*deal_opt_setting)(u8 *setting_data, u8 write_vm, u8 tws_sync);

    // 上面是必填，下面是选填
    int (*custom_setting_init)(void);
    int (*custom_vm_info_update)(void);
    int (*custom_setting_update)(u8 *setting_data);
    int (*custom_sibling_setting_deal)(u8 *setting_data);
    int (*custom_setting_release)(void);
    int (*set_setting_extra_handle)(void *setting_data, void *param);
    int (*get_setting_extra_handle)(void *setting_data, void *param);
} RCSP_SETTING_OPT;
#pragma pack()

void deal_sibling_setting(u8 *buf);
u8 rcsp_read_data_from_vm(u8 syscfg_id, u8 *buf, u8 buf_len);
int register_rcsp_setting_opt_setting(void *opt_param);

RCSP_SETTING_OPT *get_rcsp_setting_opt_hdl(u16 setting_type);
void set_rcsp_opt_setting(RCSP_SETTING_OPT *setting_opt, u8 *data);
int get_rcsp_opt_setting(RCSP_SETTING_OPT *setting_opt, u8 *data);
int set_setting_extra_handle(RCSP_SETTING_OPT *setting_opt_hdl, void *setting_data, void *param);
int get_setting_extra_handle(RCSP_SETTING_OPT *setting_opt_hdl, void *setting_data, void *param);

void rcsp_setting_init(void);
void update_rcsp_setting(u8 type);
void update_info_from_vm_info(void);
void rcsp_opt_release(void);

//属于弹窗
void modify_bt_name_and_reset(u32 msec);
#endif
