#include "rcsp_config.h"
#include "rcsp_function.h"
#include "rcsp.h"
#include "rcsp_event.h"
#include "rcsp_func_cmd_common.h"
#include "event/device_event.h"

#include "ble_rcsp_module.h"
#include "rcsp_setting_opt.h"
/* #include "dev_manager.h" *////RCSP TODO
#include "btstack/avctp_user.h"

/* #include "app_task.h" *////RCSP TODO

#if TCFG_APP_FM_EMITTER_EN
#include "fm_emitter/fm_emitter_manage.h"
#endif

#if RCSP_ADV_ANC_VOICE
#include "adv_anc_voice.h"
#endif // RCSP_ADV_ANC_VOICE

#if RCSP_ADV_ASSISTED_HEARING
#include "adv_hearing_aid_setting.h"
#endif // RCSP_ADV_ASSISTED_HEARING

#if (RCSP_MODE)

#define ASSERT_SET_LEN(len, limit) 	\
									do{	\
										if(len >= limit){	\
										}else{				\
											return 0;   \
										}\
									}while(0);


#define ASSERT_SET_LEN_RETURN_NULL(len, limit) 	\
									do{	\
										if(len >= limit){	\
										}else{				\
											return ;   \
										}\
									}while(0);

#define FUNCTION_UPDATE_MAX_LEN			(256)

#pragma pack(1)
struct _DEV_info {
    u8 status;
    u32 usb_handle;
    u32 sd0_handle;
    u32 sd1_handle;
    u32 flash_handle;
    u32 flash_handle_2;
};
#pragma pack()

typedef bool (*func_set)(void *priv, u8 *data, u16 len);
typedef u32(*func_get)(void *priv, u8 *buf, u16 buf_size, u32 mask);
static const func_set set_tab[FUNCTION_MASK_MAX] = {
    [BT_FUNCTION_MASK] = bt_func_set,
#if (RCSP_MODE && TCFG_APP_MUSIC_EN)
    [MUSIC_FUNCTION_MASK] = music_func_set,
#endif /* #if (RCSP_MODE && TCFG_APP_MUSIC_EN) */
    [RTC_FUNCTION_MASK] = rtc_func_set,
    [LINEIN_FUNCTION_MASK] = linein_func_set,
    [FM_FUNCTION_MASK] = fm_func_set,
    [FMTX_FUNCTION_MASK] = NULL,
};

static const func_get get_tab[FUNCTION_MASK_MAX] = {
    [BT_FUNCTION_MASK] = bt_func_get,
#if (RCSP_MODE && TCFG_APP_MUSIC_EN)
    [MUSIC_FUNCTION_MASK] = music_func_get,
#endif /* #if (RCSP_MODE && TCFG_APP_MUSIC_EN) */
    [RTC_FUNCTION_MASK] = rtc_func_get,
    [LINEIN_FUNCTION_MASK] = linein_func_get,
    [FM_FUNCTION_MASK] = fm_func_get,
    [FMTX_FUNCTION_MASK] = NULL,
};

static u8 _OpCode = 0;
static u8 _OpCode_SN = 0;

static void common_function_attr_vol_set(void *priv, u8 attr, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    if (BT_CALL_HANGUP != get_call_status()) {
        rcspModel->err_code = -1;
        return;
    }
    ASSERT_SET_LEN_RETURN_NULL(len, 1);
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_VOL_SETTING);
    if (setting_opt_hdl) {
        set_rcsp_opt_setting(setting_opt_hdl, data);
    }
}
static void common_function_attr_eq_set(void *priv, u8 attr, u8 *data, u16 len)
{
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_EQ_SETTING);
    if (setting_opt_hdl) {
        set_rcsp_opt_setting(setting_opt_hdl, data);
        u32 mask = BIT(attr);
        rcsp_msg_post(USER_MSG_RCSP_SET_EQ_PARAM, 2, (int)priv, mask);
    }
}
static void common_function_attr_fmtx_freq_set(void *priv, u8 attr, u8 *data, u16 len)
{
#if (TCFG_APP_FM_EMITTER_EN && TCFG_FM_EMITTER_INSIDE_ENABLE)
    ASSERT_SET_LEN_RETURN_NULL(len, 2);
    u16 freq = READ_BIG_U16(data);
    rcsp_msg_post(USER_MSG_RCSP_SET_FMTX_FREQ, 2, (int)priv, (int)freq);
#endif
}
static void common_function_attr_bt_emitter_sw_set(void *priv, u8 attr, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    ASSERT_SET_LEN_RETURN_NULL(len, 1);
    rcspModel->emitter_sw = data[0];
    rcsp_msg_post(USER_MSG_RCSP_SET_BT_EMITTER_SW, 1, (int)priv);
}
static void common_function_attr_bt_emitter_connect_state_set(void *priv, u8 attr, u8 *data, u16 len)
{
    ASSERT_SET_LEN_RETURN_NULL(len, 7);
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    memcpy(rcspModel->emitter_con_addr, data + 1, 6);
    rcsp_msg_post(USER_MSG_RCSP_SET_BT_EMITTER_CONNECT_STATES, 2, (int)priv, (int)data[0]);
}

static void common_function_attr_high_low_set(void *priv, u8 attr, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    if (BT_CALL_HANGUP != get_call_status()) {
        rcspModel->err_code = -1;
        return;
    }

    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_HIGH_LOW_VOL);
    if (!set_setting_extra_handle(setting_opt_hdl, data, NULL)) {
        set_rcsp_opt_setting(setting_opt_hdl, NULL);
    }
}

static void common_function_attr_anc_voice_info_set(void *priv, u8 attr, u8 *data, u16 len)
{
#if RCSP_ADV_ANC_VOICE
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    anc_voice_info_set(data, len);
#endif // RCSP_ADV_ANC_VOICE
}

static void common_function_attr_hearing_aid_setting(void *priv, u8 attr, u8 *data, u16 len)
{
#if RCSP_ADV_ASSISTED_HEARING
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    deal_hearing_aid_setting(_OpCode, _OpCode_SN, data, 0, 1);
#endif // RCSP_ADV_ASSISTED_HEARING
}

static void common_function_attr_misc_setting_set(void *priv, u8 attr, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    if (BT_CALL_HANGUP != get_call_status()) {
        rcspModel->err_code = -1;
        return;
    }
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_MISC_SETTING);
    if (setting_opt_hdl) {
        if (!set_setting_extra_handle(setting_opt_hdl, data, NULL)) {
            set_rcsp_opt_setting(setting_opt_hdl, data);
        } else {
            rcspModel->err_code = -1;
        }
    }
}

static void common_function_attr_color_led_setting_set(void *priv, u8 attr, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_COLOR_LED_SETTING);
    if (setting_opt_hdl) {
        set_rcsp_opt_setting(setting_opt_hdl, data);
    }
}

static void common_function_attr_karaoke_eq_setting_set(void *priv, u8 attr, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_KARAOKE_EQ_SETTING);
    if (setting_opt_hdl) {
        set_rcsp_opt_setting(setting_opt_hdl, data);
        u32 mask = BIT(attr);
        rcsp_msg_post(USER_MSG_RCSP_SET_EQ_PARAM, 2, (int)priv, mask);
    }
}

static const attr_set_func common_function_set_tab[COMMON_FUNCTION_ATTR_TYPE_MAX] = {
    [COMMON_FUNCTION_ATTR_TYPE_BATTERY 				          ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_VOL 					          ] = common_function_attr_vol_set,
    [COMMON_FUNCTION_ATTR_TYPE_DEV_INFO 			          ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_ERROR_STATS			          ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_EQ_INFO				          ] = common_function_attr_eq_set,//NULL,
    [COMMON_FUNCTION_ATTR_TYPE_BS_FILE_TYPE			          ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_FUNCTION_MODE		          ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_FMTX_FREQ			          ] = common_function_attr_fmtx_freq_set,
    [COMMON_FUNCTION_ATTR_TYPE_BT_EMITTER_SW		          ] = common_function_attr_bt_emitter_sw_set,
    [COMMON_FUNCTION_ATTR_TYPE_BT_EMITTER_CONNECT_STATES      ] = common_function_attr_bt_emitter_connect_state_set,
    [COMMON_FUNCTION_ATTR_TYPE_HIGH_LOW_SET			          ] = common_function_attr_high_low_set,
    [COMMON_FUNCTION_ATTR_TYPE_PRE_FETCH_ALL_EQ_INFO 	      ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_ANC_VOICE	                  ] = common_function_attr_anc_voice_info_set,
    [COMMON_FUNCTION_ATTR_TYPE_FETCH_ALL_ANC_VOICE            ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_PHONE_SCO_STATE_INFO 	      ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_MISC_SETTING_INFO 	      	  ] = common_function_attr_misc_setting_set,
    [COMMON_FUNCTION_ATTR_TYPE_COLOR_LED_SETTING_INFO 		  ] = common_function_attr_color_led_setting_set,
    [COMMON_FUNCTION_ATTR_TYPE_PRE_FETCH_KARAOKE_EQ_INFO	  ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_KARAOKE_EQ_SETTING_INFO 		  ] = common_function_attr_karaoke_eq_setting_set,
    [COMMON_FUNCTION_ATTR_TYPE_ASSISTED_HEARING               ] = common_function_attr_hearing_aid_setting,
};


static u32 common_function_attr_battery_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    extern u8 get_vbat_percent(void);
    u8 vbat = get_vbat_percent();
    rlen = add_one_attr(buf, buf_size, offset, attr, &vbat, sizeof(vbat));
    return rlen;
}
static u32 common_function_attr_vol_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    //extern s8 app_audio_get_volume(u8 state);
    //u8 cur_vol = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
    if (BT_CALL_HANGUP != get_call_status()) {
        return 0;
    }
    u8 cur_vol;
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_VOL_SETTING);
    if (setting_opt_hdl) {
        get_rcsp_opt_setting(setting_opt_hdl, (u8 *)&cur_vol);
        rlen = add_one_attr(buf, buf_size, offset, attr, &cur_vol, sizeof(cur_vol));
    }
    return rlen;
}
static u32 common_function_attr_dev_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    struct RcspModel *rcspModel = (struct RcspModel *) priv;
    struct _DEV_info dev_info = {0};
    u32 dev_status_tmp = 0;
#if 1
    if (dev_manager_get_root_path_by_logo("udisk0")) {
        printf("dev [udisk0] online\n");
        dev_info.status |= BIT(BS_UDISK);
        dev_info.usb_handle |= app_htonl((u32)BS_UDISK);
    }

    if (dev_manager_get_root_path_by_logo("sd0")) {
        printf("dev [sd0] online\n");
        dev_info.status |= BIT(BS_SD0);
        dev_info.sd0_handle |= app_htonl((u32)BS_SD0);
    }
    if (dev_manager_get_root_path_by_logo("sd1")) {
        printf("dev [sd1] online\n");
        dev_info.status |= BIT(BS_SD1);
        dev_info.sd1_handle = app_htonl((u32)BS_SD1);
    }
    if (rcspModel->file_transfer_mode && dev_manager_get_root_path_by_logo("virfat_flash")) {
        printf("dev [virfat_flash] online\n");
        dev_info.status |= BIT(BS_FLASH);
        dev_info.flash_handle = app_htonl((u32)BS_FLASH);
    }
    if (rcspModel->file_trans_nor_fat && dev_manager_get_root_path_by_logo("fat_nor")) {
        printf("dev [fat_nor] online\n");
        dev_info.status |= BIT(BS_FLASH_2);
        dev_info.flash_handle_2 = app_htonl((u32)BS_FLASH_2);
    }
#endif
#if TCFG_APP_LINEIN_EN
    extern int linein_app_check(void);
    if (linein_app_check()) {
        dev_info.status |= BIT(BS_AUX);
    }
#endif
    rlen = add_one_attr(buf, buf_size, offset, attr, (u8 *)&dev_info, sizeof(dev_info));
    return rlen;
}
static u32 common_function_attr_error_states_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    return rlen;
}

static u32 common_function_attr_pre_fetch_all_eq_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 eq_pre_fetch_info[1  +  20  + (1 + 10) * 10] = {0}; // num + freq + all_gain_of_eq [max]
    u16 eq_per_fetch_size = sizeof(eq_pre_fetch_info);
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_EQ_SETTING);
    if (setting_opt_hdl) {
        eq_per_fetch_size = get_setting_extra_handle(setting_opt_hdl, eq_pre_fetch_info, &eq_per_fetch_size);
        if (eq_per_fetch_size) {
            rlen = add_one_attr(buf, buf_size, offset, attr, eq_pre_fetch_info, eq_per_fetch_size);
        }
    }
    return rlen;
}

static u32 common_function_attr_anc_voice_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
#if RCSP_ADV_ANC_VOICE
    u8 anc_info[9] = {0};
    anc_voice_info_get(anc_info, sizeof(anc_info));
    rlen = add_one_attr(buf, buf_size, offset, attr, anc_info, sizeof(anc_info));
#endif // RCSP_ADV_ANC_VOICE
    return rlen;
}

static u32 common_function_attr_anc_voice_info_fetch_all_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
#if RCSP_ADV_ANC_VOICE
    u8 anc_fetch_all_info[28] = {0};
    anc_voice_info_fetch_all_get(anc_fetch_all_info, sizeof(anc_fetch_all_info));
    rlen = add_one_attr(buf, buf_size, offset, attr, anc_fetch_all_info,  sizeof(anc_fetch_all_info));
#endif // RCSP_ADV_ANC_VOICE
    return rlen;
}

static u32 common_function_attr_phone_sco_state_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 phone_state = 0;
    if (BT_CALL_HANGUP != get_call_status()) {
        phone_state = 1;
    }
    rlen = add_one_attr(buf, buf_size, offset, attr, &phone_state, sizeof(phone_state));
    return rlen;
}

static u32 common_function_attr_eq_param_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 eq_info[11] = {0};
    u16 eq_info_size = sizeof(eq_info);
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_EQ_SETTING);
    if (setting_opt_hdl) {
        eq_info_size = get_setting_extra_handle(setting_opt_hdl, eq_info, &eq_info_size);
        if (eq_info_size) {
            rlen = add_one_attr(buf, buf_size, offset, attr, eq_info, eq_info_size);
        }
    }
    return rlen;
}

static u32 common_function_attr_bs_file_type(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
#if RCSP_FILE_OPT
    /* printf("%s\n", rcsp_browser_file_ext()); */
    rlen = add_one_attr(buf, buf_size, offset, attr, (u8 *)rcsp_browser_file_ext(), rcsp_browser_file_ext_size());
#endif
    return rlen;
}
static u32 common_function_attr_function_mode_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    struct RcspModel *rcspModel = (struct RcspModel *) priv;
    rlen = add_one_attr(buf, buf_size, offset, attr, &rcspModel->cur_app_mode, sizeof(rcspModel->cur_app_mode));
    return rlen;
}
static u32 common_function_attr_fmtx_freq_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
#if (TCFG_APP_FM_EMITTER_EN && TCFG_FM_EMITTER_INSIDE_ENABLE)
    u16 freq = fm_emitter_manage_get_fre();
    printf("freq %d\n", freq);
    freq = app_htons(freq);
    rlen = add_one_attr(buf, buf_size, offset, attr, (u8 *)&freq, sizeof(freq));
#endif
    return rlen;
}
static u32 common_function_attr_bt_emitter_sw_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return 0;
    }
    u32 rlen = 0;
    u8 sw = rcspModel->emitter_sw;
    rlen = add_one_attr(buf, buf_size, offset,  attr, &sw, 1);
    return rlen;
}
static u32 common_function_attr_bt_emitter_connect_state_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return 0;
    }

    u32 rlen = 0;
    u8 send_buf[7] = {0};
    /* if(rcspModel->emitter_sw) */
    {
        send_buf[0] = rcspModel->emitter_bt_state;
        memcpy(send_buf + 1, rcspModel->emitter_con_addr, 6);
    }
    printf("stata = %d\n", send_buf[0]);
    put_buf(send_buf + 1, 6);
    rlen = add_one_attr(buf, buf_size, offset,  attr, send_buf, sizeof(send_buf));
    return rlen;
}
static u32 common_function_attr_high_low_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 data[8] = {0};
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_HIGH_LOW_VOL);
    if (setting_opt_hdl) {
        u8 size = get_setting_extra_handle(setting_opt_hdl, data, NULL);
        if (size) {
            rlen = add_one_attr(buf, buf_size, offset, attr, data, size);
        }
    }

    return rlen;
}

static u32 common_function_attr_misc_setting_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    extern u32 get_misc_setting_data_len(void);
    u32 data_len = get_misc_setting_data_len();
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_MISC_SETTING);
    if (data_len && setting_opt_hdl) {
        u8 *misc_data = zalloc(data_len);
        data_len = get_rcsp_opt_setting(setting_opt_hdl, misc_data);
        rlen = add_one_attr(buf, buf_size, offset, attr, misc_data, data_len);
        if (misc_data) {
            free(misc_data);
        }
    }
    return rlen;
}

static u32 common_function_attr_color_led_setting_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    extern u32 get_color_led_setting_data_len(void);
    u32 data_len = get_color_led_setting_data_len();
    u8 color_led_data[data_len];

    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_COLOR_LED_SETTING);
    if (setting_opt_hdl) {
        get_rcsp_opt_setting(setting_opt_hdl, color_led_data);
        rlen = add_one_attr(buf, buf_size, offset, attr, color_led_data, data_len);
    }
    return rlen;
}

static u32 common_function_attr_karaoke_eq_setting_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 karaoke_eq_info[9] = {0};
    u16 karaoke_eq_size = sizeof(karaoke_eq_info);
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_KARAOKE_EQ_SETTING);
    if (setting_opt_hdl) {
        karaoke_eq_size = get_setting_extra_handle(setting_opt_hdl, karaoke_eq_info, &karaoke_eq_size);
        if (karaoke_eq_size) {
            rlen = add_one_attr(buf, buf_size, offset, attr, karaoke_eq_info, karaoke_eq_size);
        }
    }
    return rlen;
}

static u32 common_function_attr_dha_fitting_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
#if (RCSP_ADV_ASSISTED_HEARING)
    if (mask & BIT(COMMON_INFO_ATTR_ASSISTED_HEARING)) {
        u8 dha_fitting_data[3 + 2 + 2 * DHA_FITTING_CHANNEL_MAX] = {0};
        get_dha_fitting_info(dha_fitting_data);
        offset += add_one_attr(buf, buf_size, offset, attr, dha_fitting_data, sizeof(dha_fitting_data));
    }
#endif
    return rlen;
}

static u32 common_function_attr_karaoke_pre_fetch_eq_setting_info_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u8 karaoke_eq_pre_fetch_info[1 + 8 * 2] = {0}; // num + freq * 2
    u16 karaoke_eq_pre_fetch_size = sizeof(karaoke_eq_pre_fetch_info);
    RCSP_SETTING_OPT *setting_opt_hdl = get_rcsp_setting_opt_hdl(ATTR_TYPE_KARAOKE_EQ_SETTING);
    if (setting_opt_hdl) {
        karaoke_eq_pre_fetch_size = get_setting_extra_handle(setting_opt_hdl, karaoke_eq_pre_fetch_info, &karaoke_eq_pre_fetch_size);
        if (karaoke_eq_pre_fetch_size) {
            rlen = add_one_attr(buf, buf_size, offset, attr, karaoke_eq_pre_fetch_info, karaoke_eq_pre_fetch_size);
        }
    }
    return rlen;
}

static const attr_get_func target_common_function_get_tab[COMMON_FUNCTION_ATTR_TYPE_MAX] = {
    [COMMON_FUNCTION_ATTR_TYPE_BATTERY 				          ] = common_function_attr_battery_get,
    [COMMON_FUNCTION_ATTR_TYPE_VOL 					          ] = common_function_attr_vol_get,
    [COMMON_FUNCTION_ATTR_TYPE_DEV_INFO 			          ] = common_function_attr_dev_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_ERROR_STATS			          ] = NULL,
    [COMMON_FUNCTION_ATTR_TYPE_EQ_INFO				          ] = common_function_attr_eq_param_get,
    [COMMON_FUNCTION_ATTR_TYPE_BS_FILE_TYPE			          ] = common_function_attr_bs_file_type,
    [COMMON_FUNCTION_ATTR_TYPE_FUNCTION_MODE		          ] = common_function_attr_function_mode_get,
    [COMMON_FUNCTION_ATTR_TYPE_FMTX_FREQ			          ] = common_function_attr_fmtx_freq_get,
    [COMMON_FUNCTION_ATTR_TYPE_BT_EMITTER_SW		          ] = common_function_attr_bt_emitter_sw_get,
    [COMMON_FUNCTION_ATTR_TYPE_BT_EMITTER_CONNECT_STATES      ] = common_function_attr_bt_emitter_connect_state_get,
    [COMMON_FUNCTION_ATTR_TYPE_HIGH_LOW_SET			          ] = common_function_attr_high_low_get,
    [COMMON_FUNCTION_ATTR_TYPE_PRE_FETCH_ALL_EQ_INFO 	      ] = common_function_attr_pre_fetch_all_eq_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_ANC_VOICE                      ] = common_function_attr_anc_voice_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_FETCH_ALL_ANC_VOICE            ] = common_function_attr_anc_voice_info_fetch_all_get,
    [COMMON_FUNCTION_ATTR_TYPE_PHONE_SCO_STATE_INFO 	      ] = common_function_attr_phone_sco_state_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_MISC_SETTING_INFO 	      	  ] = common_function_attr_misc_setting_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_COLOR_LED_SETTING_INFO 		  ] = common_function_attr_color_led_setting_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_PRE_FETCH_KARAOKE_EQ_INFO	  ] = common_function_attr_karaoke_pre_fetch_eq_setting_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_KARAOKE_EQ_SETTING_INFO 		  ] = common_function_attr_karaoke_eq_setting_info_get,
    [COMMON_FUNCTION_ATTR_TYPE_ASSISTED_HEARING               ] = common_function_attr_dha_fitting_info_get,
};



static bool rcsp_common_function_set(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    printf("rcsp_common_function_set\n");
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return false;
    }
    _OpCode = OpCode;
    _OpCode_SN = OpCode_SN;
    put_buf(data, len);
    attr_set(priv, data, len, common_function_set_tab, COMMON_FUNCTION_ATTR_TYPE_MAX);
    if (rcspModel->err_code) {
        rcspModel->err_code = 0;
        return false;
    }
    return true;
}

static u32 rcsp_common_function_get(void *priv, u8 *buf, u16 buf_size, u32 mask)
{
    printf("rcsp_common_function_get, mask = %x\n", mask);
    return attr_get(priv, buf, buf_size, target_common_function_get_tab, COMMON_FUNCTION_ATTR_TYPE_MAX, mask);
}

bool rcsp_function_set(void *priv, u8 OpCode, u8 OpCode_SN, u8 function, u8 *data, u16 len)
{
    if (function >= FUNCTION_MASK_MAX) {
        if (function == COMMON_FUNCTION) {
            return rcsp_common_function_set(priv, OpCode, OpCode_SN, data, len);
        } else {
            return false;
        }
    }

    func_set func = set_tab[function];
    if (func) {
        return func(priv, data, len);
    }

    return false;
}

u32 rcsp_function_get(void *priv, u8 function, u8 *data, u16 len, u8 *buf, u16 buf_size)
{
    ASSERT_SET_LEN(len, 4);
    u32 mask = READ_BIG_U32(data);
    if (function >= FUNCTION_MASK_MAX) {
        if (function == COMMON_FUNCTION) {
            return rcsp_common_function_get(priv, buf, buf_size, mask);
        } else {
            return 0;
        }
    }

    func_get func = get_tab[function];
    if (func) {
        return func(priv, buf, buf_size, mask);
    }
    return 0;
}

bool rcsp_function_cmd_set(void *priv, u8 OpCode, u8 OpCode_SN, u8 function, u8 *data, u16 len)
{
    if (COMMON_FUNCTION == function) {
        // 模式切换
        rcsp_msg_post(USER_MSG_RCSP_MODE_SWITCH, 2, (int)priv, (int)data[0]);
        return true;
    }
    return rcsp_function_set(priv, OpCode, OpCode_SN, function, data, len);
}

void rcsp_function_update(u8 function, u32 mask)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL || 0 == JL_rcsp_get_auth_flag()) {
        return ;
    }
    u32 rlen = 0;
    u8 *buf = zalloc(FUNCTION_UPDATE_MAX_LEN);
    if (buf == NULL) {
        printf("no ram err!!\n");
        return ;
    }
    buf[0] = function;
    if (function >= FUNCTION_MASK_MAX) {
        if (function == COMMON_FUNCTION) {
            rlen = attr_get((void *)rcspModel, buf + 1, FUNCTION_UPDATE_MAX_LEN - 1, target_common_function_get_tab, COMMON_FUNCTION_ATTR_TYPE_MAX, mask);
        }
    } else {
        func_get func = get_tab[function];
        if (func) {
            rlen = func(rcspModel, buf + 1, FUNCTION_UPDATE_MAX_LEN - 1, mask);
        }
    }
    if (rlen) {
        JL_CMD_send(JL_OPCODE_SYS_INFO_AUTO_UPDATE, buf, rlen + 1, JL_NOT_NEED_RESPOND);
    }
    free(buf);
}

void rcsp_update_bt_emitter_connect_state(u8 state, u8 *addr)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return ;
    }
    rcspModel->emitter_bt_state = state;
    memcpy(rcspModel->emitter_con_addr, addr, 6);
    rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_BT_EMITTER_CONNECT_STATES));
}

void rcsp_update_dev_state(u32 event, void *param)
{
#if JL_RCSP_EXTRA_FLASH_OPT
    extern bool rcsp_exflash_flash_opt_check(void *param);
    if (rcsp_exflash_flash_opt_check(param)) {
        return;
    }
#endif
    switch (event) {
    case DEVICE_EVENT_FROM_ALM:
        printf("DEVICE_EVENT_FROM_ALM\n");
        extern void rcsp_rtc_msg_deal(int msg);
        rcsp_rtc_msg_deal(-1);
        break;
    case DEVICE_EVENT_FROM_LINEIN:
        rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_DEV_INFO));
        break;
    }
}

u8 get_cur_mode(u8 app_mode)
{
    u8 current_mode;
#if 0 ///RCSP TODO
    switch (app_mode) {
    case APP_MUSIC_TASK:
        current_mode = MUSIC_FUNCTION;
        break;
    case APP_RTC_TASK:
        current_mode = RTC_FUNCTION;
        break;
    case APP_LINEIN_TASK:
        current_mode = LINEIN_FUNCTION;
        break;
    case APP_FM_TASK:
        current_mode = FM_FUNCTION;
        break;
    case APP_BT_TASK:
    default:
        current_mode = BT_FUNCTION;
        break;
    }
#endif
    return current_mode;
}

void function_change_inform(u8 app_mode, u8 ret)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return ;
    }
    if (LINEIN_FUNCTION == rcspModel->cur_app_mode) {
        rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_DEV_INFO));
    }
    rcspModel->cur_app_mode = get_cur_mode(app_mode);
    rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_FUNCTION_MODE));
}

#endif
