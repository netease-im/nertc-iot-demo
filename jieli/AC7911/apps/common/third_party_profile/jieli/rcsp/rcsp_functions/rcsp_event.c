#include "rcsp_config.h"
#include "rcsp_event.h"
#include "rcsp.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "rcsp_function.h"

#if TCFG_APP_FM_EMITTER_EN
#include "fm_emitter/fm_emitter_manage.h"
#endif
/* #include "app_task.h" *////RCSP TODO
#include "key_event_deal.h"
/* #include "audio_config.h" *////RCSP TODO
#include "rcsp_misc_setting.h"

#if (RCSP_MODE)

extern void bt_search_device(void);

extern void emitter_search_stop(u8 result);

static void file_browser_stop_resp(u8 reason)
{
    JL_CMD_send(JL_OPCODE_FILE_BROWSE_REQUEST_STOP, &reason, 1, JL_NEED_RESPOND);
}

static void file_browser_stop(u8 reason)
{
    int argv[3];
    argv[0] = (int)file_browser_stop_resp;
    argv[1] = 1;
    argv[2] = reason;
    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, argv);
}

static void rcsp_common_event_deal(int msg, int argc, int *argv)
{
    int err = 0;
    struct RcspModel *rcspModel = (struct RcspModel *)argv[0];
    if (rcspModel == NULL) {
        printf("rcspModelhdl NULL err!!\n");
        return ;
    }

    switch (msg) {
    case USER_MSG_RCSP_DISCONNECT_EDR:
        printf("USER_MSG_RCSP_DISCONNECT_EDR\n");
        if (get_curr_channel_state() != 0) {
            user_send_cmd_prepare(USER_CTRL_A2DP_CMD_CLOSE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
        }
        break;
#if TCFG_USER_EMITTER_ENABLE
    case USER_MSG_RCSP_BT_SCAN_OPEN:
        printf("USER_MSG_RCSP_BT_SCAN_OPEN\n");
        bt_search_device();
        break;
    case USER_MSG_RCSP_BT_SCAN_STOP:
        printf("USER_MSG_RCSP_BT_SCAN_STOP\n");
        emitter_search_stop(0);
        break;

    case USER_MSG_RCSP_BT_CONNECT_SPEC_ADDR:
        printf("USER_MSG_RCSP_BT_CONNECT_SPEC_ADDR\n");
        put_buf(rcspModel->emitter_con_addr, 6);
        user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, rcspModel->emitter_con_addr);
        break;
#endif//TCFG_USER_EMITTER_ENABLE

    case USER_MSG_RCSP_SET_VOL:
        printf("USER_MSG_RCSP_SET_VOL\n");
        rcsp_function_update(COMMON_FUNCTION, (u32)argv[1]);
        break;
    case USER_MSG_RCSP_SET_EQ_PARAM:
        printf("USER_MSG_RCSP_SET_EQ_PARAM\n");
        rcsp_function_update(COMMON_FUNCTION, (u32)argv[1]);
        break;
    case USER_MSG_RCSP_SET_FMTX_FREQ:
        printf("USER_MSG_RCSP_SET_FMTX_FREQ\n");
#if (TCFG_APP_FM_EMITTER_EN && TCFG_FM_EMITTER_INSIDE_ENABLE)
        u16 freq = (u16)argv[1];
        printf("freq %d\n", freq);
        fm_emitter_manage_set_fre(freq);
#endif
        /* #if TCFG_UI_ENABLE */
        /* 			ui_set_menu_ioctl(MENU_FM_DISP_FRE, freq); */
        /* #endif */
        break;
#if TCFG_USER_EMITTER_ENABLE
    case USER_MSG_RCSP_SET_BT_EMITTER_SW:
        printf("USER_MSG_RCSP_SET_BT_EMITTER_SW\n");
        break;
    case USER_MSG_RCSP_SET_BT_EMITTER_CONNECT_STATES:
        printf("USER_MSG_RCSP_SET_BT_EMITTER_CONNECT_STATES, state = %d\n", argv[1]);
        if (argv[1]) {
            emitter_search_stop(0);
            put_buf(rcspModel->emitter_con_addr, 6);
            user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, rcspModel->emitter_con_addr);
        } else {
            if (get_curr_channel_state() != 0) {
                printf("-----------------11111111111111111\n");
                user_send_cmd_prepare(USER_CTRL_A2DP_CMD_CLOSE, 0, NULL);
                user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
            }
        }
        break;
#endif//TCFG_USER_EMITTER_ENABLE

#if RCSP_FILE_OPT
    case USER_MSG_RCSP_BS_END:
        printf("USER_MSG_RCSP_BS_END\n");
        u8 reason = (u8)argv[1];
        rcsp_browser_stop();
        printf("reason = %d\n", reason);
        if (2 == reason) {
#if TCFG_APP_MUSIC_EN
            char *dev_logo = (char *)argv[2];
            char *cur_dev_logo = music_player_get_dev_cur();
            u32 sclust = (u32)argv[3];
            u8 app = app_get_curr_task();
            printf("app= %d, dev_loop = %s, sclust = %x\n", app, dev_logo, sclust);
            if (app == APP_MUSIC_TASK) {
                printf("is music mode \n");
                if ((music_player_get_file_sclust() == sclust) //簇号相同
                    && (cur_dev_logo && (strcmp(cur_dev_logo, dev_logo) == 0)) //设备相同
                    && (music_player_get_play_status() == FILE_DEC_STATUS_PLAY) //正在播放
                   ) {
                    //同一个设备的同一首歌曲，在播放的情况，浏览选中不重新播放
                    printf("the same music file!!\n");
                } else {
                    dev_manager_set_active_by_logo(dev_logo);
                    app_task_put_key_msg(KEY_MUSIC_PLAYE_BY_DEV_SCLUST, sclust);
                }
            } else {
                printf("is not music mode\n");
                ///设定音乐模式初次播放参数为按照簇号播放
                music_task_set_parm(MUSIC_TASK_START_BY_SCLUST, sclust);
                ///将选定的设备设置为活动设备
                dev_manager_set_active_by_logo(dev_logo);
#if (RCSP_MODE == RCSP_MODE_WATCH)
                //进入music后自动播放
                music_set_start_auto_play(1);
#endif
                ///切换模式
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
#if 0 ///RCSP TODO
                app_task_switch_to(APP_MUSIC_TASK, NULL_VALUE);
#endif
#else
#if 0 ///RCSP TODO
                app_task_switch_to(APP_MUSIC_TASK);
#endif
#endif
            }
#endif /* #if TCFG_APP_MUSIC_EN */
        }
        file_browser_stop(reason);
        break;
#endif
    case USER_MSG_RCSP_MODE_SWITCH:
        printf("USER_MSG_RCSP_MODE_SWITCH\n");
        bool ret = true;
        u8 mode = (u8)argv[1];
        switch (mode) {
        case FM_FUNCTION_MASK:
#if TCFG_APP_FM_EN
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_FM_TASK, NULL_VALUE);
#endif

#else
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_FM_TASK);
#endif
#endif
#endif
            break;
        case BT_FUNCTION_MASK:
#if (TCFG_APP_BT_EN)
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_BT_TASK, NULL_VALUE);
#endif
#else
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_BT_TASK);
#endif
#endif
#endif
            break;
        case MUSIC_FUNCTION_MASK:
#if (TCFG_APP_MUSIC_EN && !RCSP_APP_MUSIC_EN)
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_MUSIC_TASK, NULL_VALUE);
#endif
#else
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_MUSIC_TASK);
#endif
#endif
#endif
            break;
        case RTC_FUNCTION_MASK:
#if (TCFG_APP_RTC_EN && !RCSP_APP_RTC_EN)
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_RTC_TASK, NULL_VALUE);
#endif
#else
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_RTC_TASK);
#endif
#endif
#endif
            break;
        case LINEIN_FUNCTION_MASK:
#if TCFG_APP_LINEIN_EN
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_LINEIN_TASK, NULL_VALUE);
#endif
#else
#if 0 ///RCSP TODO
            ret = app_task_switch_to(APP_LINEIN_TASK);
#endif
#endif
#endif
            break;
        case FMTX_FUNCTION_MASK:
            break;
        }
        if (false == ret) {
            extern void function_change_inform(u8 app_mode, u8 ret);
            function_change_inform(app_get_curr_task(), ret);
        }
        break;
    case USER_MSG_RCSP_FM_UPDATE_STATE:
        printf("USER_MSG_RCSP_FM_UPDATE_STATE\n");
        extern void rcsp_fm_msg_deal(int msg);
        rcsp_fm_msg_deal(-1);
        break;
    case USER_MSG_RCSP_RTC_UPDATE_STATE:
        /* printf("USER_MSG_RCSP_RTC_UPDATE_STATE\n"); */
        /* extern void rcsp_rtc_msg_deal(int msg); */
        /* rcsp_rtc_msg_deal(-1); */
        break;
    case USER_MSG_RCSP_CONNECT_RESET:
        printf("USER_MSG_RCSP_CONNECT_RESET\n");
        extern void file_transfer_close(void);
        file_transfer_close();
        break;
    case USER_MSG_RCSP_FILE_TRANS_BACK:
        printf("USER_MSG_RCSP_FILE_TRANS_BACK\n");
        extern void file_trans_back_close(void);
        file_trans_back_close();
        break;
    case USER_MSG_RCSP_NFC_FILE_TRANS_BACK:
        printf("USER_MSG_RCSP_NFC_FILE_TRANS_BACK\n");
        extern void nfc_file_trans_back_end(void *priv, u32 handler, u8 op, int result, int param);
        nfc_file_trans_back_end((void *)rcspModel, (u32)argv[1], (u8)argv[2], argv[3], argv[4]);
        break;
    case USER_MSG_RCSP_SPORT_DATA_EVENT:
        printf("USER_MSG_RCSP_SPORT_DATA_EVENT\n");
        extern void sport_data_func_event(void *priv, u8 flag);
        sport_data_func_event((void *)rcspModel, (u8)argv[1]);
        break;
    default:
        break;
    }
}

#define APP_RCSP_MSG_VAL_MAX	8
bool rcsp_msg_post(int msg, int argc, ...)
{
    int argv[APP_RCSP_MSG_VAL_MAX] = {0};
    bool ret = true;
    va_list argptr;
    va_start(argptr, argc);

    if (argc > APP_RCSP_MSG_VAL_MAX) {
        printf("%s, msg argc err\n", __FUNCTION__);
        ret = false;
    } else {
        argv[0] = (int)	rcsp_common_event_deal;
        argv[2] = msg;
        for (int i = 0; i < argc; i++) {
            argv[i + 3] = va_arg(argptr, int);
        }

        if (argc >= 2) {
            argv[1] = argc + 1;
        } else {
            argv[1] = 3;
            argc = 3;
        }
        int r = os_taskq_post_type("app_core", Q_CALLBACK, argc + 3, argv);
        if (r) {
            printf("app_next post msg err %x\n", r);
            ret = false;
        }
    }
    return ret;
}

int rcsp_bt_key_event_deal(u8 key_event, int ret)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return ret;
    }
    if (BT_CALL_HANGUP != get_call_status()) {
        return ret;
    }
    switch (key_event) {
    case  KEY_VOL_DOWN:
    case  KEY_VOL_UP:
        rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_VOL));
        break;
    }
    return ret;
}

int rcsp_common_key_event_deal(u8 key_event, int ret)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return ret;
    }
    if (BT_CALL_HANGUP != get_call_status()) {
        return ret;
    }
    switch (key_event) {
#if !JL_EARPHONE_APP_EN
    case KEY_MINOR_OPT:
        printf("rcsp_find_dev\n");
        extern void rcsp_find_device(void);
        rcsp_find_device();
        ret = true;
        break;
#endif
    case  KEY_VOL_DOWN:
    case  KEY_VOL_UP:
        rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_VOL));
        break;
    default:
        if (rcsp_misc_event_deal(key_event, NULL)) {
            break;
        }
        break;
    }
    return ret;
}

bool rcsp_key_event_filter_before(int key_event)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL) {
        return false;
    }
    if (0 == rcspModel->dev_vol_sync) {
        return false;
    }
    bool ret = false;
    switch (key_event) {
    case KEY_VOL_UP:
        printf("COMMON KEY_VOL_UP\n");
        extern void volume_up(void);
        volume_up();
#if TCFG_UI_ENABLE
        UI_SHOW_MENU(MENU_MAIN_VOL, 1000, app_audio_get_volume(APP_AUDIO_CURRENT_STATE), NULL);
#endif
        ret = true;
        break;
    case KEY_VOL_DOWN:
        printf("COMMON KEY_VOL_DOWN\n");
        extern void volume_down(void);
        volume_down();
#if TCFG_UI_ENABLE
        UI_SHOW_MENU(MENU_MAIN_VOL, 1000, app_audio_get_volume(APP_AUDIO_CURRENT_STATE), NULL);
#endif
        ret = true;
        break;
    }
    if (ret) {
        rcsp_function_update(COMMON_FUNCTION, BIT(COMMON_FUNCTION_ATTR_TYPE_VOL));
    }
    return ret;
}

#endif

