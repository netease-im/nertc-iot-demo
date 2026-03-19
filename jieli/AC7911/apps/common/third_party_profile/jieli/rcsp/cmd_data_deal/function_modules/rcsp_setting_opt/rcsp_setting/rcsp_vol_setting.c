#include "rcsp_config.h"
#include "cpu.h"
#include "syscfg_id.h"

#include "rcsp_setting_sync.h"
#include "rcsp_setting_opt.h"
#include "ble_rcsp_module.h"
#include "rcsp_function.h"

#include "key_event_deal.h"
/* #include "audio_config.h" *////RCSP TODO

#if JL_EARPHONE_APP_EN
/* #include "vol_sync.h" *////RCSP TODO
#else
/* #include "bt/vol_sync.h" *////RCSP TODO
#endif

#if (RCSP_MODE && RCSP_ADV_EQ_SET_ENABLE)

static u8 vol_setting[1] = {0};
extern int get_bt_tws_connect_status();

bool rcsp_set_device_volume(int volume)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return false;
    }
    if (0 == rcspModel->dev_vol_sync) {
        return false;
    }
    if (0 == rcspModel->A_platform) {
        set_music_device_volume(volume);
    }

    return true;
}

bool rcsp_set_volume(s8 volume)
{
    static u8 cur_flag = 0;
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (cur_flag || rcspModel == NULL) {
        cur_flag = 0;
        return false;
    }
    if (0 == rcspModel->dev_vol_sync) {
        return false;
    }
#if TCFG_APP_LINEIN_EN
    if (LINEIN_FUNCTION == rcspModel->cur_app_mode) {
        cur_flag = 1;
        extern int linein_volume_set(u8 vol);
        linein_volume_set(volume);
        cur_flag = 0;
        return true;
    }
#endif
    return false;
}

bool rcsp_key_volume_down(u8 value)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return false;
    }
    if (0 == rcspModel->dev_vol_sync) {
        return false;
    }
#if TCFG_APP_LINEIN_EN
    if (LINEIN_FUNCTION == rcspModel->cur_app_mode) {
        extern void linein_key_vol_down();
        linein_key_vol_down();
        return true;
    }
#endif
    return false;
}

bool rcsp_key_volume_up(u8 value)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return false;
    }
    if (0 == rcspModel->dev_vol_sync) {
        return false;
    }
#if TCFG_APP_LINEIN_EN
    if (LINEIN_FUNCTION == rcspModel->cur_app_mode) {
        extern void linein_key_vol_up();
        linein_key_vol_up();
        true;
    }
#endif
    return false;
}

void rcsp_get_max_vol_info(u8 *vol_info)
{
    *vol_info = get_max_sys_vol();
}

void rcsp_get_cur_dev_vol_info(u8 *vol_info)
{
    *vol_info = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
}

void rcsp_set_vol_for_find_device(u8 vol_flag)
{
    static u8 vol = 0;
    static u8 state = 0;
    if (state == vol_flag) {
        return ;
    }
    if (vol_flag) {
        // 停止媒体
        vol = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
        // 最大声
        app_audio_set_volume(app_audio_get_state(), get_max_sys_vol(), 1);

    } else {
        // 恢复
        app_audio_set_volume(app_audio_get_state(), vol, 1);
    }

    rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_VOL));
    state = vol_flag;
}

static int rcsp_get_vol_info(u8 *vol_info)
{
    *vol_info = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
    return 0;
}

static void set_vol_info(u8 *vol_info)
{
    memcpy(vol_setting, vol_info, sizeof(vol_setting));
}

static void update_vol_vm_value(u8 *vol_info)
{
    // 不需要写vm操作，5s后会自动保持音量值
    //syscfg_write(CFG_MUSIC_VOL, vol_setting, sizeof(vol_setting));
}

static void vol_setting_sync(u8 *vol_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_rcsp_setting(ATTR_TYPE_VOL_SETTING);
    }
#endif
}

static void vol_state_update(void)
{
    // 更新操作，包含写vm操作
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return ;
    }
    u8 vol = vol_setting[0];
    if (LINEIN_FUNCTION == rcspModel->cur_app_mode) {
#if TCFG_APP_LINEIN_EN
        extern int linein_volume_set(u8 vol);
        linein_volume_set(vol);
#endif
    } else {
        app_audio_set_volume(app_audio_get_state(), vol, 1);
    }
}

static void deal_vol_setting(u8 *vol_info, u8 write_vm, u8 tws_sync)
{
    if (vol_info) {
        set_vol_info(vol_info);
    }
    if (write_vm) {
        update_vol_vm_value(vol_setting);
    }
    if (tws_sync) {
        vol_setting_sync(vol_setting);
    }
    vol_state_update();
}

static int vol_setting_init(void)
{
    int ret = 0;
    u8 cur_vol = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
    set_vol_info(&cur_vol);

    return 0;
}

static RCSP_SETTING_OPT vol_setting_opt = {
    .data_len = 1,
    .setting_type = ATTR_TYPE_VOL_SETTING,
    .syscfg_id = CFG_MUSIC_VOL,
    .deal_opt_setting = deal_vol_setting,
    .set_setting = set_vol_info,
    .get_setting = rcsp_get_vol_info,
    .custom_setting_init = vol_setting_init, // 初始化不需要vm操作，所以重新实现
    .custom_setting_release = NULL,
    .custom_vm_info_update = NULL,
    .custom_setting_update = NULL,
    .custom_sibling_setting_deal = NULL,
};
REGISTER_APP_SETTING_OPT(vol_setting_opt);

#else

void rcsp_set_vol_for_find_device(u8 vol_flag)
{
#if 0
    static u8 vol = 0;
    static u8 state = 0;
    if (state == vol_flag) {
        return ;
    }
    if (vol_flag) {
        // 停止媒体
        vol = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
        // 最大声
        app_audio_set_volume(app_audio_get_state(), get_max_sys_vol(), 1);

    } else {
        // 恢复
        app_audio_set_volume(app_audio_get_state(), vol, 1);
    }

    rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_VOL));
    state = vol_flag;
#endif //end of  0
}

bool rcsp_set_device_volume(int volume)
{
    return false;
}

void rcsp_get_max_vol_info(u8 *vol_info)
{

}

void rcsp_get_cur_dev_vol_info(u8 *vol_info)
{

}

#endif
