#ifndef __RCSP_MISC_SETTING__
#define __RCSP_MISC_SETTING__

#include "system/includes.h"

#define REGISTER_APP_MISC_SETTING_OPT(rcsp_misc_opt) \
int rcsp_misc_opt##_setting_init(void) {\
   return register_rcsp_setting_misc_setting(&rcsp_misc_opt);\
}\
late_initcall(rcsp_misc_opt##_setting_init);

#define RCSP_REVERBERATION_SETTING	1
#define RCSP_DRC_VAL_SETTING		1
#define RCSP_KARAOKE_SOUND_EFFECT	1
#define RCSP_KARAOKE_ATMOSPHERE		1
#define RCSP_KARAOKE_SOUND_PARAM	1

enum {
    MISC_SETTING_REVERBERATION = 0,
    MISC_SETTING_DRC_VAL = 1,
    MISC_SETTING_KARAOKE_SOUND_EFFECT = 2,
    MISC_SETTING_KARAOKE_ATMOSPHERE = 3,
    MISC_SETTING_KARAOKE_SOUND_PARAM = 4,
    MISC_SETTING_MASK_MAX = 32, // 最多只支持32个
};

#pragma pack(1)
typedef struct _RCSP_MISC_SETTING_OPT {
    struct _RCSP_MISC_SETTING_OPT *next;
    bool be_notify_app;	// 接收到数据并更新效果完后是否需要通知app, 如果需要通知app则置1
    u32 misc_data_len;
    int misc_setting_type;
    int misc_syscfg_id;

    // misc_set_setting函数参数：
    // misc_setting : 传入数据
    // is_conversion : 1 - 需要转换，从大端转为小端
    // 				   0 - 不需要转换
    int (*misc_set_setting)(u8 *misc_setting, u8 is_conversion);

    // misc_get_setting函数参数：
    // misc_setting : 传出数据
    // is_conversion : 1 - 需要转换，从小端转为大端
    // 				   0 - 不需要转换
    int (*misc_get_setting)(u8 *misc_setting, u8 is_conversion);
    int (*misc_state_update)(u8 *misc_setting);	// 参数用于判断当前更新是设置状态(非空)还是初始化/同步(未空)

    // 上面是必填，下面是选填
    int (*misc_custom_setting_init)(void);
    int (*misc_custom_write_vm)(u8 *misc_setting);
    int (*misc_custom_key_event_callback_deal)(u32 event, void *param);
} RCSP_MISC_SETTING_OPT;
#pragma pack()

int register_rcsp_setting_misc_setting(void *misc_setting);
int rcsp_misc_event_deal(u32 event, void *param);
#endif
