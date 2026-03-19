#include "rcsp_config.h"
#include "rcsp.h"
#include "rcsp_function.h"
#include "rcsp_feature.h"
#include "rcsp_switch_device.h"
#include "file_transfer.h"
#include "file_delete.h"
#include "file_env_prepare.h"
#include "file_trans_back.h"
#include "dev_format.h"
#include "rcsp_event.h"
#include "btstack/avctp_user.h"
/* #include "app_action.h" *////RCSP TODO
#include "rcsp_adv_bluetooth.h"
#include "rcsp_extra_flash_cmd.h"
#include "rcsp_update.h"
#include "fs/fs.h"
#include "file_bluk_trans_prepare.h"
#include "sensors_data_opt.h"
#include "file_simple_transfer.h"
#include "custom_cfg.h"
#include "JL_rcsp_api.h"
#include "app_config.h"

#if TCFG_USER_TWS_ENABLE
#include "classic/tws_api.h"
/* #include "bt_tws.h" *////RCSP TODO
#endif

#if	(defined CONFIG_DEBUG_RECORD_ENABLE && CONFIG_DEBUG_RECORD_ENABLE)
#include "asm/debug_record.h"
#endif

////<<<<<<<<APP 下发命令响应处理
#if (RCSP_MODE)

#define ASSET_CMD_DATA_LEN(len, limit) 	\
	do{	\
		if(len >= limit){	\
		}else{				\
			return ;   \
		}\
	}while(0);


static void get_target_feature(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }

    rcspModel->A_platform = data[4];

    u32 mask = READ_BIG_U32(data);
    u32 wlen = 0;
    u8 *resp = zalloc(TARGET_FEATURE_RESP_BUF_SIZE);
    ASSERT(resp, "func = %s, line = %d, no ram!!\n", __FUNCTION__, __LINE__);
    wlen = target_feature_parse_packet(priv, resp, TARGET_FEATURE_RESP_BUF_SIZE, mask);
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp, wlen);
    free(resp);
}

static void get_sys_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    ASSET_CMD_DATA_LEN(len, 1);
    u8 function = data[0];
    u8 *resp = zalloc(TARGET_FEATURE_RESP_BUF_SIZE);
    resp[0] = function;
    u32 rlen = rcsp_function_get(priv, function, data + 1, len - 1, resp + 1, TARGET_FEATURE_RESP_BUF_SIZE - 1);
    if (rlen == 0) {
        printf("get_sys_info LTV NULL !!!!\n");
    }
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp, (u16)rlen + 1);
    free(resp);
}

#if RCSP_ADV_ASSISTED_HEARING
static u8 _hearing_aid_operating_flag = 0;
void set_hearing_aid_operating_flag()
{
    _hearing_aid_operating_flag = 1;
}
#endif // RCSP_ADV_ASSISTED_HEARING

static void set_sys_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    ASSET_CMD_DATA_LEN(len, 1);

    u8 function = data[0];
    bool ret = rcsp_function_set(priv, OpCode, OpCode_SN, function, data + 1, len - 1);
#if (RCSP_ADV_ASSISTED_HEARING)
    if (_hearing_aid_operating_flag) {
        _hearing_aid_operating_flag = 0;
        return;
    }
#endif
    if (ret == true) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    }
}

static void disconnect_edr(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    printf("notify disconnect edr\n");
    u8 op = 0;
    if (len) {
        op = data[0];
    }

    switch (op) {
    case 0 :
        rcsp_msg_post(USER_MSG_RCSP_DISCONNECT_EDR, 1, (int)priv);
        break;
    case 1:
        rcsp_msg_post(USER_MSG_RCSP_CONNECT_RESET, 1, (int)priv);
#if (0 == BT_CONNECTION_VERIFY)
        JL_rcsp_auth_reset();
#endif
        break;
    }
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
}

static void file_bs_start(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    /* printf("JL_OPCODE_FILE_BROWSE_REQUEST_START\n"); */
    /* put_buf(data, len); */
#if RCSP_FILE_OPT
    if (rcsp_browser_busy() == false) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        rcsp_browser_start(data, len);
    }
#endif
}

static void open_bt_scan(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 result = 0;
    bool ret = rcsp_msg_post(USER_MSG_RCSP_BT_SCAN_OPEN, 1, (int)priv);
    if (ret == true) {
        result = 1;
    }
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &result, 1);
}

static void stop_bt_scan(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 result = 0;
    bool ret = rcsp_msg_post(USER_MSG_RCSP_BT_SCAN_STOP, 1, (int)priv);
    if (ret == true) {
        result = 1;
    }
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &result, 1);
}

static void connect_bt_spec_addr(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 result = 0;
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    if (len != 6) {
        printf("connect_bt_spec_addr data len err = %d\n", len);
    }
    memcpy(rcspModel->emitter_con_addr, data, 6);
    bool ret = rcsp_msg_post(USER_MSG_RCSP_BT_CONNECT_SPEC_ADDR, 1, (int)priv);
    if (ret == true) {
        result = 1;
    }
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &result, 1);
}

static void function_cmd_handle(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    ASSET_CMD_DATA_LEN(len, 1);

    u8 function = data[0];
    bool ret = rcsp_function_cmd_set(priv, OpCode, OpCode_SN, function, data + 1, len - 1);
    if (ret == true) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    }
}

static void find_device_handle(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
#if RCSP_ADV_FIND_DEVICE_ENABLE
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    if (!rcspModel->find_dev_en) {
        return;
    }
    if (BT_CALL_HANGUP != get_call_status()) {
        return;
    }
    u8 type = data[0];
    u8 opt = data[1];
    // 0 - way, 1 - player, 2 - find_device_key_flag
    u8 other_opt[3] = {0};

#if JL_EARPHONE_APP_EN

    if (opt) {
        u16 timeout = READ_BIG_U16(data + 2);
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            memcpy(other_opt, &timeout, sizeof(timeout));
            extern void find_device_stop_timer(u8 * param, u32 msec);
            find_device_stop_timer(other_opt, 300);
        } else
#endif
        {
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_STOP, &timeout, sizeof(timeout));
        }
    } else {
        extern void find_device_stop(void *priv);
        find_device_stop(NULL);
    }

#else // JL_EARPHONE_APP_EN

    if (opt) {

        // 播放铃声
        u16 timeout = READ_BIG_U16(data + 2);
        extern void find_device_timeout_handle(u32 sec);
        find_device_timeout_handle(timeout);
    } else {
        // 关闭铃声
        extern void rcsp_stop_find_device(void *priv);
        rcsp_stop_find_device(NULL);
#if (RCSP_MODE == RCSP_MODE_WATCH)
        extern void rcsp_find_phone_reset(void);
        rcsp_find_phone_reset();
#if 0 ///RCSP TODO
        if (UI_GET_WINDOW_ID() == ID_WINDOW_FINDPHONE) {
            UI_WINDOW_BACK_SHOW(2);
        }
#endif
#endif
    }

#endif // JL_EARPHONE_APP_EN

    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);

#if JL_EARPHONE_APP_EN

    // len为4时没有way和player两种设置
    if (4 == len) {
        return;
    }

    // 铃声播放时才给way和player赋值
    if (6 == len && opt) {
        other_opt[0] = data[4]; // way
        other_opt[1] = data[5]; // player
    }

    // 300ms超时，发送对端同步执行
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        extern void find_device_sync(u8 * param, u32 msec);
        find_device_sync(other_opt, 300);
        return;
    }
#endif
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_RESUME, other_opt, sizeof(other_opt));

#endif // JL_EARPHONE_APP_EN

#endif // RCSP_ADV_FIND_DEVICE_ENABLE

}

#define RES_MD5_FILE	SDFILE_RES_ROOT_PATH"md5.bin"
static void get_md5_handle(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    u8 md5[32] = {0};
    FILE *fp = fopen(RES_MD5_FILE, "r");
    if (!fp) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        return;
    }
#if (RCSP_MODE == RCSP_MODE_EARPHONE_V2022)
    u32 r_len = fread((void *)md5, 32, 1, fp);
#else
    /* u32 r_len = fread(fp, (void *)md5, 32); */

    u32 r_len = fread((void *)md5, 32, 1, fp);
#endif
    if (r_len != 32) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        return;
    }
    /* printf("get [md5] succ:"); */
    /* put_buf(md5, 32); */
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, md5, 32);
    fclose(fp);
}

static void get_low_latency_param(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    u8 low_latency_param[6] = {0};
#if JL_EARPHONE_APP_EN
    low_latency_param[0] = 0;
    low_latency_param[1] = 20;
    low_latency_param[2] = 100 >> 24;
    low_latency_param[3] = 100 >> 16;
    low_latency_param[4] = 100 >> 8;
    low_latency_param[5] = 100;
#endif // JL_EARPHONE_APP_EN

    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, low_latency_param, 6);
}

#if TCFG_RTC_ENABLE
static void rcsp_alarm_ex(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u16 rlen = 0;
    u8 op = data[0];
    bool ret = 1;
    u8 *resp = zalloc(TARGET_FEATURE_RESP_BUF_SIZE);
    resp[0] = op;
    if (op == 0x00) {
        rlen = rtc_func_get_ex(priv, resp + 1, TARGET_FEATURE_RESP_BUF_SIZE - 1, data[1]) + 1;
        put_buf(resp, rlen);
    } else if (op == 0x01) {
        put_buf(data, len);
        ret = rtc_func_set_ex(priv, data + 1, len - 1);
        rlen = 1;
    }
    if (ret == true) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp, rlen);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, resp, rlen);
    }
    free(resp);
}
#endif

static void rcsp_device_parm_extra(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 op = data[0];
    switch (op) {
    case 0x0:
        file_transfer_download_parm_extra(OpCode_SN, data, len);
        break;;
    case 0x1:
        break;;
    case 0x2:
        break;;
    default:
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_PARAM_ERR, OpCode_SN, 0, 0);
        break;
    }
}

#if	(defined CONFIG_DEBUG_RECORD_ENABLE && CONFIG_DEBUG_RECORD_ENABLE)
#define EXCEPTION_DATA_LEN 240//要小于rcsp的mtu
extern u8 check_le_pakcet_sent_finish_flag(void);
extern bool rcsp_send_list_is_empty(void);
static void rcsp_request_exception_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return ;
    }
    /* printf("LXF 0 start analyse exception request!"); */
    u8 sub_code = data[0];
    if (sub_code == 0) {
        /* printf("LXF 1 get exception info!"); */
        u8 exception_info_param[6] = {0};
        struct debug_record_info m_debug_record_info;
        user_debug_record_info_get(&m_debug_record_info);
        /* put_buf(m_debug_record_info.record_buf, m_debug_record_info.record_len); */
        if (m_debug_record_info.record_len > 0) {
            u32 record_len = m_debug_record_info.record_len;
            /* printf("LXF 2 has exception info, record_len:%x", record_len); */
            /* put_buf((u8 *)&record_len, 4); */
            record_len = READ_BIG_U32(&record_len);
            /* printf("LXF 2.1 has exception info, record_len:%x", record_len); */
            /* put_buf((u8 *)&record_len, 4); */
            memcpy(exception_info_param, &record_len, 4);
            // 计算crc
            JL_ERR err = 0;
            u32 offset = 0;
            u32 len_tmp = 0;
            u16 file_crc = 0;//占用2byte
            while (offset < m_debug_record_info.record_len) {
                len_tmp = (m_debug_record_info.record_len - offset) > EXCEPTION_DATA_LEN ? EXCEPTION_DATA_LEN : (m_debug_record_info.record_len - offset);
                printf("===================%s======%d=========yuring==\n\r", __func__, __LINE__);
#if 0 ///RCSP TODO
                file_crc = CRC16_with_initval(m_debug_record_info.record_buf + offset, len_tmp, file_crc);
#endif
                offset += len_tmp;
            }
            /* file_crc = CRC16(m_debug_record_info.record_buf, m_debug_record_info.record_len); */
            /* printf("LXF 3 crc info:%x", file_crc); */
            /* put_buf((u8 *)&file_crc, 2); */
            file_crc = READ_BIG_U16(&file_crc);
            /* printf("LXF 3.1 crc info:%x", file_crc); */
            /* put_buf((u8 *)&file_crc, 2); */
            memcpy(exception_info_param + 4, &file_crc, 2);
            /* printf("LXF 4 response:"); */
            /* put_buf(exception_info_param, sizeof(exception_info_param)); */
            err = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, exception_info_param, 6);
            if (err == 0) {
                // 发送异常信息
                offset = 0;
                len_tmp = 0;
                /* printf("LXF 5 start send exception data"); */
                while (offset < m_debug_record_info.record_len) {
                    len_tmp = (m_debug_record_info.record_len - offset) > EXCEPTION_DATA_LEN ? EXCEPTION_DATA_LEN : (m_debug_record_info.record_len - offset);
                    /* printf("LXF 6 sending exception data, offset:%d, total_len:%d, tmp_data_len:%d", offset, m_debug_record_info.record_len, len_tmp); */
                    err = JL_DATA_send(JL_OPCODE_DATA, JL_OPCODE_REQUEST_EXCEPTION_INFO, m_debug_record_info.record_buf + offset, len_tmp, JL_NOT_NEED_RESPOND);
                    if (JL_ERR_SEND_BUSY == err) {
                        continue;
                    }
                    offset += len_tmp;
                }
                /* printf("LXF 7 %s end", __FUNCTION__); */

                while (!(rcsp_send_list_is_empty() && check_le_pakcet_sent_finish_flag())) {
                    os_time_dly(10);
                }

                u8 end_param[1] = {1};
                err = JL_CMD_send(OpCode, end_param, 1, JL_NEED_RESPOND);
                return ;
            } else {
                printf("%s fail!", __func__);
            }
        } else {
            /* printf("LXF 2 no exception info"); */
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, exception_info_param, 6);
        }
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    }
}
#endif /* #if (define CONFIG_DEBUG_RECORD_ENABLE && CONFIG_DEBUG_RECORD_ENABLE) */

void cmd_recieve(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    switch (OpCode) {
    case JL_OPCODE_GET_TARGET_FEATURE:
        get_target_feature(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_DISCONNECT_EDR:
        disconnect_edr(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SWITCH_DEVICE:
        switch_device(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SYS_INFO_GET:
        get_sys_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SYS_INFO_SET:
        set_sys_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_FILE_BROWSE_REQUEST_START:
        file_bs_start(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SYS_OPEN_BT_SCAN:
        open_bt_scan(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SYS_STOP_BT_SCAN:
        stop_bt_scan(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SYS_BT_CONNECT_SPEC:
        connect_bt_spec_addr(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_FUNCTION_CMD:
        function_cmd_handle(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SYS_FIND_DEVICE:
        find_device_handle(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_GET_MD5:
        get_md5_handle(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_LOW_LATENCY_PARAM:
        get_low_latency_param(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_CUSTOMER_USER:
        rcsp_user_cmd_recieve(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_FILE_TRANSFER_START:
        file_transfer_download_start(priv, OpCode_SN, data, len);
        break;
    case JL_OPCODE_FILE_TRANSFER_CANCEL:
        file_transfer_download_passive_cancel(OpCode_SN, data, len);
        break;
    case JL_OPCODE_FILE_DELETE:
        file_delete_start(OpCode_SN, data, len);
        break;
    case JL_OPCODE_ACTION_PREPARE:
        app_rcsp_task_prepare(1, data[0], OpCode_SN);
        break;
    case JL_OPCODE_DEVICE_FORMAT:
        dev_format_start(OpCode_SN, data, len);
        break;
#if TCFG_RTC_ENABLE
    case JL_OPCODE_ALARM_EXTRA:
        rcsp_alarm_ex(priv, OpCode, OpCode_SN, data, len);
        break;
#endif // TCFG_RTC_ENABLE

    case JL_OPCODE_DEVICE_PARM_EXTRA:
        rcsp_device_parm_extra(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_ONE_FILE_DELETE:
        file_delete_one_file(OpCode_SN, data, len);
        break;
    case JL_OPCODE_ONE_FILE_TRANS_BACK:
        file_trans_back_opt(priv, OpCode_SN, data, len);
        break;
    case JL_OPCODE_FILE_BLUK_TRANSFER:
        file_bluk_trans_prepare(priv, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SIMPLE_FILE_TRANS:
        file_simple_transfer_for_small_file(priv, OpCode_SN, data, len);
        break;

#if	(defined CONFIG_DEBUG_RECORD_ENABLE && CONFIG_DEBUG_RECORD_ENABLE)
    case JL_OPCODE_REQUEST_EXCEPTION_INFO:
        rcsp_request_exception_info(priv, OpCode, OpCode_SN, data, len);
        break;
#endif

    default:
        if (0 == JL_rcsp_sensors_data_opt(priv, OpCode, OpCode_SN, data, len)) {
            break;
        }
#if JL_RCSP_EXTRA_FLASH_OPT
        if (0 == JL_rcsp_extra_flash_cmd_resp(priv, OpCode, OpCode_SN, data, len)) {
            break;
        }
#endif
#if 1//RCSP_ADV_EN
#if 0 ///RCSP TODO
        if (0 == JL_rcsp_adv_cmd_resp(priv, OpCode, OpCode_SN, data, len)) {
            break;
        }
#endif
#endif
#if RCSP_UPDATE_EN
        if (0 == JL_rcsp_update_cmd_resp(priv, OpCode, OpCode_SN, data, len)) {
            break;
        }
#endif
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_UNKOWN_CMD, OpCode_SN, 0, 0);
        break;
    }
}

#endif//RCSP_MODE
