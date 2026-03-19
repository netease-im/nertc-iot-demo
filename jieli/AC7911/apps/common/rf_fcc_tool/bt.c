#include "rf_fcc_main.h"

#if (defined(RF_FCC_TEST_ENABLE) && defined(CONFIG_BT_ENABLE))

#include "btstack/avctp_user.h"
#include "btcontroller_modules.h"
#include "btstack/bluetooth.h"
#include "btstack/btstack_error.h"
#include "event/bt_event.h"

extern void bredr_set_dut_enble(u8 en, u8 phone);
extern void bredr_close_all_scan(void);
extern void __set_a2dp_auto_play_flag(u8 auto_en);
extern void __set_simple_pair_flag(bool flag);
extern void lmp_private_free_esco_packet(void *packet);
extern void *lmp_private_get_esco_packet(int *len, u32 *hash);
extern u32 esco_media_get_packet_num(void);

/*开关可发现可连接的函数接口*/
static void bt_wait_phone_connect_control(u8 enable)
{
    if (enable) {
        log_i("is_1t2_connection:%d \t total_conn_dev:%d\n", is_1t2_connection(), get_total_connect_dev());
        if (is_1t2_connection()) {
            /*达到最大连接数，可发现(0)可连接(0)*/
            user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
        } else {
            if (get_total_connect_dev() == 1) {
                /*支持连接2台，只连接一台的情况下，可发现(0)可连接(1)*/
                user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
                user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
            } else {
                /*可发现(1)可连接(1)*/
                user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
                user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
            }
        }
    } else {
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    }
}

static void bt_read_remote_name(u8 status, u8 *addr, u8 *name)
{
    if (status) {
        printf("remote_name fail \n");
    } else {
        printf("remote_name : %s \n", name);
    }

    put_buf(addr, 6);
}

static int bt_get_battery_value()
{
    //取消默认蓝牙定时发送电量给手机，需要更新电量给手机使用USER_CTRL_HFP_CMD_UPDATE_BATTARY命令
    /*电量协议的是0-9个等级，请比例换算*/
    return 9;
}

static void bt_dut_api(u8 value)
{
    log_i("bt in dut\n");
    bredr_close_all_scan();
}

static void bredr_handle_register(void)
{
#if BT_SUPPORT_MUSIC_VOL_SYNC
    ///蓝牙音乐和通话音量同步
    //music_vol_change_handle_register(bt_set_music_device_volume, phone_get_device_vol);
#endif
#if BT_SUPPORT_DISPLAY_BAT
    ///电量显示获取电量的接口
    get_battery_value_register(bt_get_battery_value);   /*电量显示获取电量的接口*/
#endif
    ///被测试盒链接上进入快速测试回调
    /* bt_fast_test_handle_register(bt_fast_test_api); */

    ///样机进入dut被测试仪器链接上回调
    bt_dut_test_handle_register(bt_dut_api);

    ///获取远端设备蓝牙名字回调
    read_remote_name_handle_register(bt_read_remote_name);
}

static void bt_function_select_init(void)
{
    __set_a2dp_auto_play_flag(1); /* 连接高级音频后自动播放歌曲 */
    __set_support_msbc_flag(1);
#if BT_SUPPORT_DISPLAY_BAT
    __bt_set_update_battery_time(60);
#else
    __bt_set_update_battery_time(0);
#endif
    __set_page_timeout_value(8000); /*回连搜索时间长度设置,可使用该函数注册使用，ms单位,u16*/
    __set_super_timeout_value(8000); /*回连时超时参数设置。ms单位。做主机有效*/

    __set_simple_pair_flag(1); //是否打开简易配对功能，打开后不需要输入pincode
    ////设置蓝牙加密的level
    //io_capabilities ; /*0: Display only 1: Display YesNo 2: KeyboardOnly 3: NoInputNoOutput*/
    //authentication_requirements: 0:not protect  1 :protect
    __set_simple_pair_param(3, 0, 2);

    void lmp_set_sniff_disable(void);
    lmp_set_sniff_disable();
}

static void a2dp_drop_frame(void *p)
{
    int len;
    u8 *frame;
    int num = a2dp_media_get_packet_num();
    if (num > 1) {
        for (int i = 0; i < (num - 1); i++) {
            len = a2dp_media_get_packet(&frame);
            if (len <= 0) {
                break;
            }
            /* log_i("a2dp_drop_frame: %d\n", len); */
            a2dp_media_free_packet(frame);
        }
    }
}

static void esco_drop_frame(void *p)
{
    u32 hash = 0;
    int len;
    void *frame;
    int num = esco_media_get_packet_num();
    if (num > 1) {
        for (int i = 0; i < (num - 1); i++) {
            frame = (u8 *)lmp_private_get_esco_packet(&len, &hash);
            if (len <= 0) {
                break;
            }
            /* log_i("a2dp_drop_frame: %d\n", len); */
            lmp_private_free_esco_packet(frame);
        }
    }
}

void fcc_bt_ble_module_init(void)
{
    bt_function_select_init();
    bredr_handle_register();
    btstack_init();
}

static int bt_connction_status_event_handler(struct bt_event *bt)
{
    static u16 a2dp_drop_frame_timer = 0;
    static u16 esco_drop_frame_timer = 0;
    log_i("-----------------------bt_connction_status_event_handler %d\n", bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        /*
         * 蓝牙初始化完成
         */
        log_i("===================BT_STATUS_INIT_OK\n");

        //extern u8 get_rf_analog_gain(void);
        //extern void wifi_set_pwr(unsigned char pwr_sel);
        //wifi_set_pwr(get_rf_analog_gain());

        if (BT_MODE_IS(BT_BQB) || BT_MODE_IS(BT_NORMAL)) {
            ble_bqb_test_thread_init();
        }

        bt_wait_phone_connect_control(1);
        break;
    case BT_STATUS_START_CONNECTED:
        log_i(" BT_STATUS_START_CONNECTED\n");
        break;
    case BT_STATUS_ENCRY_COMPLETE:
        log_i(" BT_STATUS_ENCRY_COMPLETE\n");
        break;
    case BT_STATUS_SECOND_CONNECTED:
        clear_current_poweron_memory_search_index(0);
    case BT_STATUS_FIRST_CONNECTED:
        //蓝牙连接成功
        log_i("BT_STATUS_CONNECTED\n");
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        //蓝牙连接断开
        log_i("BT_STATUS_DISCONNECT\n");
        break;
    //phone status deal
    case BT_STATUS_PHONE_INCOME:
        //接到来电
        log_i("BT_STATUS_PHONE_INCOME\n");
        user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        /* lmp_private_esco_suspend_resume(3); */
        break;
    case BT_STATUS_PHONE_OUT:
        //通话打出
        log_i("BT_STATUS_PHONE_OUT\n");
        lmp_private_esco_suspend_resume(4);
        break;
    case BT_STATUS_PHONE_ACTIVE:
        //通话接通
        log_i("BT_STATUS_PHONE_ACTIVE\n");
        lmp_private_esco_suspend_resume(4);
        break;
    case BT_STATUS_PHONE_HANGUP:
        //通话挂断
        log_i("phone_handup\n");
        lmp_private_esco_suspend_resume(4);
        break;
    case BT_STATUS_PHONE_NUMBER:
        log_i("BT_STATUS_PHONE_NUMBER\n");
        break;
    case BT_STATUS_INBAND_RINGTONE:
        log_i("BT_STATUS_INBAND_RINGTONE\n");
        break;
    case BT_STATUS_BEGIN_AUTO_CON:
        log_i("BT_STATUS_BEGIN_AUTO_CON\n");
        break;
    case BT_STATUS_A2DP_MEDIA_START:
        log_i(" BT_STATUS_A2DP_MEDIA_START");
        a2dp_drop_frame_timer = sys_timer_add(NULL, a2dp_drop_frame, 100);
        break;
    case BT_STATUS_A2DP_MEDIA_STOP:
        log_i(" BT_STATUS_A2DP_MEDIA_STOP");
        if (a2dp_drop_frame_timer) {
            sys_timer_del(a2dp_drop_frame_timer);
            a2dp_drop_frame_timer = 0;
        }
        break;
    case BT_STATUS_SCO_STATUS_CHANGE:
        log_i(" BT_STATUS_SCO_STATUS_CHANGE len:%d ,type:%d", (bt->value >> 16), (bt->value & 0x0000ffff));
        if (bt->value != 0xff) {
            esco_drop_frame_timer = sys_timer_add(NULL, esco_drop_frame, 100);
        } else {
            if (esco_drop_frame_timer) {
                sys_timer_del(esco_drop_frame_timer);
                esco_drop_frame_timer = 0;
            }
        }
        break;
    case BT_STATUS_CALL_VOL_CHANGE:
        break;
    case BT_STATUS_SNIFF_STATE_UPDATE:
        log_i(" BT_STATUS_SNIFF_STATE_UPDATE %d\n", bt->value);    //0退出SNIFF
        break;
    case BT_STATUS_LAST_CALL_TYPE_CHANGE:
        break;
    case BT_STATUS_CONN_A2DP_CH:
        break;
    case BT_STATUS_CONN_HFP_CH:
        break;
    default:
        break;
    }

    return 0;
}

static void bt_send_pair(u8 en)
{
    user_send_cmd_prepare(USER_CTRL_PAIR, 1, &en);
}

static u8 bt_hci_event_filter(struct bt_event *bt)
{
    if (bt->event == HCI_EVENT_VENDOR_REMOTE_TEST) {
        if (0 == bt->value) {
            set_remote_test_flag(0);
            /* log_debug("clear_test_box_flag"); */
            return 0;
        }
    }

    if ((bt->event != HCI_EVENT_CONNECTION_COMPLETE) ||
        ((bt->event == HCI_EVENT_CONNECTION_COMPLETE) && (bt->value != ERROR_CODE_SUCCESS))) {
        if (get_remote_test_flag() \
            && !(HCI_EVENT_DISCONNECTION_COMPLETE == bt->event) \
            && !(HCI_EVENT_VENDOR_REMOTE_TEST == bt->event)) {
            log_i("cpu reset\n");
            /* cpu_reset(); */
        }
    }

    return 1;
}

static int bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_i("------------------------bt_hci_event_handler reason %x", bt->event);

    if (bt_hci_event_filter(bt) == 0) {
        return 0;
    }

    switch (bt->event) {
    case HCI_EVENT_INQUIRY_COMPLETE:
        log_i(" HCI_EVENT_INQUIRY_COMPLETE \n");
        break;
    case HCI_EVENT_IO_CAPABILITY_REQUEST:
        log_i(" HCI_EVENT_IO_CAPABILITY_REQUEST \n");
        break;
    case HCI_EVENT_USER_CONFIRMATION_REQUEST:
        log_i(" HCI_EVENT_USER_CONFIRMATION_REQUEST \n");
        ///<可通过按键来确认是否配对 1：配对   0：取消
        bt_send_pair(1);
        break;
    case HCI_EVENT_USER_PASSKEY_REQUEST:
        log_i(" HCI_EVENT_USER_PASSKEY_REQUEST \n");
        ///<可以开始输入6位passkey
        break;
    case HCI_EVENT_USER_PRESSKEY_NOTIFICATION:
        log_i(" HCI_EVENT_USER_PRESSKEY_NOTIFICATION %x\n", bt->value);
        ///<可用于显示输入passkey位置 value 0:start  1:enrer  2:earse   3:clear  4:complete
        break;
    case HCI_EVENT_PIN_CODE_REQUEST :
        log_i("HCI_EVENT_PIN_CODE_REQUEST  \n");
        bt_send_pair(1);
        break;
    case HCI_EVENT_VENDOR_NO_RECONN_ADDR :
        log_i("HCI_EVENT_VENDOR_NO_RECONN_ADDR \n");
    case HCI_EVENT_DISCONNECTION_COMPLETE :
        log_i("HCI_EVENT_DISCONNECTION_COMPLETE \n");
        bt_wait_phone_connect_control(1);
        break;
    case BTSTACK_EVENT_HCI_CONNECTIONS_DELETE:
    case HCI_EVENT_CONNECTION_COMPLETE:
        log_i(" HCI_EVENT_CONNECTION_COMPLETE \n");
        switch (bt->value) {
        case ERROR_CODE_SUCCESS :
            log_i("ERROR_CODE_SUCCESS  \n");
            bt_wait_phone_connect_control(0);
            user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
            break;

        case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION   :
        case ERROR_CODE_PIN_OR_KEY_MISSING:
            log_i(" ERROR_CODE_PIN_OR_KEY_MISSING \n");
        case ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED :
        case ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES:
        case ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR:
        case ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED  :
        case ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST :
        case ERROR_CODE_AUTHENTICATION_FAILURE :
        case CUSTOM_BB_AUTO_CANCEL_PAGE:
            bt_wait_phone_connect_control(1);
            break;
        case ERROR_CODE_PAGE_TIMEOUT:
            log_i(" ERROR_CODE_PAGE_TIMEOUT \n");
            bt_wait_phone_connect_control(1);
            break;
        case ERROR_CODE_CONNECTION_TIMEOUT:
            log_i(" ERROR_CODE_CONNECTION_TIMEOUT \n");
            bt_wait_phone_connect_control(1);
            break;
        case ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS  :
            log_i("ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS   \n");
            bt_wait_phone_connect_control(1);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return 0;
}


static s32 bt_event_handler(struct sys_event *event)
{
    if (event->from == BT_EVENT_FROM_CON) {
        return bt_connction_status_event_handler((struct bt_event *)event->payload);
    } else if (event->from == BT_EVENT_FROM_HCI) {
        return bt_hci_event_handler((struct bt_event *)event->payload);
    }
    return false;
}


SYS_EVENT_STATIC_HANDLER_REGISTER(fcc_bt_event) = {
    .event_type     = SYS_BT_EVENT,
    .prob_handler   = bt_event_handler,
    .post_handler   = NULL,
};


#endif


