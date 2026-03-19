#include "app_config.h"

#include "rcsp_cfg.h"
#include "rcsp_setting_sync.h"
#include "rcsp_setting_opt.h"
#include "adv_time_stamp_setting.h"
#include "rcsp_bt_manage.h"
#include "rcsp_manage.h"

#if (RCSP_MODE && TCFG_USER_TWS_ENABLE)
/* #include "bt_tws.h" *////RCSP TODO
static u16 g_curr_get_adv_sync_tws_len = 0;

static void rcsp_sync_tws_func_t(void *data, u16 len, bool rx)
{
    if (rx) {
        g_curr_get_adv_sync_tws_len = len;
        deal_sibling_setting((u8 *)data);
    }
}

REGISTER_TWS_FUNC_STUB(adv_tws_sync) = {
    .func_id = TWS_FUNC_ID_ADV_SETTING_SYNC,
    .func    = rcsp_sync_tws_func_t,
};

static void tws_app_opt_sync_call_fun(int cmd, int err)
{
    struct sys_event event;

    event.type = SYS_BT_EVENT;
    event.arg = (void *)SYS_BT_EVENT_FROM_APP_OPT_TWS;

    event.u.bt.event = APP_OPT_TWS_EVENT_SYNC_FUN_CMD;
    event.u.bt.args[0] = 0;
    event.u.bt.args[1] = 0;
    event.u.bt.args[2] = cmd;
    sys_event_notify(&event);
}

TWS_SYNC_CALL_REGISTER(tws_tone_sync) = {
    .uuid = TWS_FUNC_APP_OPT_UUID,
    .func = tws_app_opt_sync_call_fun,
};

#if RCSP_ADV_EN
static void adv_sync_time_stamp_func_t(void *data, u16 len, bool rx)
{
    if (rx) {
        deal_sibling_time_stamp_setting_switch(data, len);
    }
}

REGISTER_TWS_FUNC_STUB(adv_time_stamp_sync) = {
    .func_id = TWS_FUNC_ID_TIME_STAMP_SYNC,
    .func    = adv_sync_time_stamp_func_t,
};


static void adv_sync_reset_sync_func_t(int args, int err)
{
    extern void cpu_reset();
    cpu_reset();
}

TWS_SYNC_CALL_REGISTER(adv_reset_sync) = {
    .uuid = TWS_FUNC_ID_ADV_RESET_SYNC,
    .func    = adv_sync_reset_sync_func_t,
};

static void adv_find_dev_sync_func_t(int args, int err)
{
    u8 args_tmp = (u8)args;
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_RESUME, &args_tmp, sizeof(args));
}

TWS_SYNC_CALL_REGISTER(adv_find_dev_sync) = {
    .uuid = TWS_FUNC_ID_ADV_FIND_DEV_SYNC,
    .func    = adv_find_dev_sync_func_t,
};

void find_device_sync(u8 *param, u32 msec)
{
    int priv = 0;
    if (TWS_ROLE_MASTER == tws_api_get_role()) {
        memcpy(&priv, param, 3);
        tws_api_sync_call_by_uuid(TWS_FUNC_ID_ADV_FIND_DEV_SYNC, priv, msec);
    }
}

static void adv_find_dev_stop_sync_timer_func_t(int args, int err)
{
    u8 args_tmp = (u8)args;
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_STOP, &args_tmp, sizeof(args));
}

TWS_SYNC_CALL_REGISTER(adv_find_dev_stop_sync) = {
    .uuid = TWS_FUNC_ID_ADV_FIND_DEV_STOP_TIMER_SYNC,
    .func    = adv_find_dev_stop_sync_timer_func_t,
};

#endif

void find_device_stop_timer(u8 *param, u32 msec)
{
    int priv = 0;
    if (TWS_ROLE_MASTER == tws_api_get_role()) {
        memcpy(&priv, param, 3);
        tws_api_sync_call_by_uuid(TWS_FUNC_ID_ADV_FIND_DEV_STOP_TIMER_SYNC, priv, msec);
    }
}

u16 get_adv_sync_tws_len(void)
{
    return (g_curr_get_adv_sync_tws_len > 2) ? (g_curr_get_adv_sync_tws_len - 2) : g_curr_get_adv_sync_tws_len;
}



#endif
