#include "rcsp_config.h"
#include "sport_data_func.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"

#include "sport_data_heart_rate.h"
#include "sport_data_air_pressure.h"
#include "sport_data_altitude.h"
#include "sport_data_blood_oxygen.h"
#include "sport_data_exercise_recovery_time.h"
#include "sport_data_exercise_steps.h"
#include "sport_data_max_oxygen_uptake.h"
#include "sport_data_pressure_detection.h"
#include "sport_data_sports_information.h"
#include "sport_data_training_load.h"

/* #include "sensor_info_weather.h" */

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)

#define SPROT_DATA_PROTOCOL_VERSION		0
#define FUNCTION_UPDATE_MAX_LEN			(256 - 32)	// 预留32个byte放别的突发的命令

//#define SPORT_DATA_TIMED_NOFIFYCATION_TIME	(5 * 60 * 1000) // 5min
#define SPORT_DATA_TIMED_NOFIFYCATION_TIME	0	// 默认关闭连上app后定时发送运动数据

#define ASSET_CMD_DATA_LEN(len, limit) 	\
	do{	\
		if(len >= limit){	\
		}else{				\
			return ;   \
		}\
	}while(0);

extern void data_func_attr_weather_set(void *priv, u8 attr, u8 *data, u16 len);
extern void func_attr_notice_add_info(void *priv, u8 attr, u8 *data, u16 len);
extern void func_attr_notice_remove_info(void *priv, u8 attr, u8 *data, u16 len);
extern u8 rcsp_eflash_update_flag_get(void);

static OS_SEM g_sport_data_get_sem;

static const sport_attr_get_func target_common_sport_func_get_tab[SPORTS_DATA_FUNC_ATTR_TYPE_GET_MAX] = {
    [SPORTS_DATA_FUNC_ATTR_TYPE_HEART_RATE 		          ] = sport_data_func_attr_heart_rate_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_AIR_PRESSURE 		      ] = sport_data_func_attr_air_pressure_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_ALTITUDE 		          ] = sport_data_func_attr_altitude_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_EXERCISE_STEPS 		      ] = sport_data_func_attr_exercise_steps_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_PRESSURE_DETECTION        ] = sport_data_func_attr_pressure_detection_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_BLOOD_OXYGEN 	          ] = sport_data_func_attr_blood_oxygen_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_TRAINING_LOAD 	          ] = sport_data_func_attr_training_load_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_MAX_OXYGEN_UPTAKE         ] = sport_data_func_attr_max_oxygen_uptake_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_EXERCISE_RECOVERY_TIME    ] = sport_data_func_attr_exercise_recovery_time_get,
    [SPORTS_DATA_FUNC_ATTR_TYPE_SPORTS_INFORMATION 	      ] = sport_data_func_attr_sports_information_get,
};

static const attr_set_func target_common_func_set_tab[DATA_FUNC_ATTR_TYPE_SET_MAX] = {
    [DATA_FUNC_ATTR_TYPE_LOCATION	 		          ] = NULL,
#if TCFG_UI_ENABLE_WEATHER
    [DATA_FUNC_ATTR_TYPE_WEATHER	 		          ] = data_func_attr_weather_set,
#endif /* #if TCFG_UI_ENABLE_WEATHER */
#if TCFG_UI_ENABLE_NOTICE
    [DATA_FUNC_ATTR_TYPE_NOTICE 		              ] = func_attr_notice_add_info,
    [DATA_FUNC_ATTR_TYPE_NOTICE_REMOVE 		          ] = func_attr_notice_remove_info,
#endif /* #if TCFG_UI_ENABLE_NOTICE */
};

static int sport_data_func_multi_pack_update(void *priv, u8 *resp_pack, u16 resp_pack_len, sport_resp_data *sport_data, u16 sport_data_len);

static u32 rcsp_sport_data_function_get(void *priv, u32 mask, u8 *sub_mask, u8 *buf, u16 buf_size)
{
    printf("rcsp_sport_data_function_get, mask = %x\n", mask);
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    u32 offset = 0;
    u32 record_len = 0;
    sport_attr_get_func func = NULL;
    sport_resp_data *resp_data = (sport_resp_data *)zalloc(SPORTS_DATA_FUNC_ATTR_TYPE_GET_MAX * sizeof(sport_resp_data));
    for (u8 i = 0, j = 0; i < SPORTS_DATA_FUNC_ATTR_TYPE_GET_MAX; i++) {
        if (mask & BIT(i)) {
            rcspModel->priv = (void *)(resp_data + i);
            func = target_common_sport_func_get_tab[i];
            if (func) {
                offset += func(priv, i, sub_mask[j++], buf, buf_size, offset);
            }
            if (resp_data[i].resp_data_len) {
                record_len += resp_data[i].resp_data_len + 2;
            }
        }
    }
    if (rcspModel->err_code) {
        sport_data_func_multi_pack_update(priv, buf, buf_size, resp_data, record_len);
        offset = 0;
    }
    for (u8 i = 0; i < SPORTS_DATA_FUNC_ATTR_TYPE_GET_MAX; i++) {
        if (resp_data[i].call_back) {
            resp_data[i].call_back(resp_data[i].priv);
        }
    }
    if (resp_data) {
        free(resp_data);
    }
    return offset;
}

static bool rcsp_data_function_set(void *priv, u8 function, u8 *data, u16 len)
{
    printf("rcsp_data_function_set\n");
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return false;
    }
    put_buf(data, len);
    attr_set_func func = target_common_func_set_tab[function];
    if (func) {
        func(priv, function, data, len);
    }
    if (rcspModel->err_code) {
        rcspModel->err_code = 0;
        return false;
    }
    return true;
}

static void get_sport_data_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return;
    }
    // 加锁
    sport_data_func_get_prepare_deal();
    rcspModel->OpCode_record = OpCode;
    rcspModel->OpCode_SN_record = OpCode_SN;
    u8 offset = 0;
    u32 mask = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    u8 version = data[offset++];
    u8 *sub_mask = data + offset;
    u8 *resp = zalloc(TARGET_FEATURE_RESP_BUF_SIZE);
    u32 rlen = rcsp_sport_data_function_get(priv, mask, sub_mask, resp + 2, TARGET_FEATURE_RESP_BUF_SIZE - 2);
    resp[0] = (rlen == 0);
    resp[1] = 1; // 附带response data
    if (rcspModel->err_code) {
        rcspModel->err_code = 0;
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp, (u16)rlen + 2);
    }
    if (resp) {
        free(resp);
    }
    sport_data_func_get_finish_deal();
}

static void set_sport_data_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 offset = 0;
    u8 function = data[offset++];
    u8 version = data[offset++];
    bool ret = rcsp_data_function_set(priv, function, data + offset, len - offset);
    u8 resp_data[2] = {function, ret};
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp_data, sizeof(resp_data));
}

void sport_data_func_update(u32 mask, u8 *sub_mask)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL || 0 == JL_rcsp_get_auth_flag()) {
        return ;
    }
    // 加锁
    sport_data_func_get_prepare_deal();
    u32 rlen = 0;
    u8 *buf = zalloc(FUNCTION_UPDATE_MAX_LEN);
    if (NULL == buf) {
        printf("%s, no ram err!!\n", __func__);
        return;
    }

    buf[0] = SPROT_DATA_PROTOCOL_VERSION; // version
    buf[1] = 1;	// package count
    buf[2] = 0; // package id

    rcspModel->OpCode_record = JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE;
    rlen = rcsp_sport_data_function_get((void *)rcspModel, mask, sub_mask, buf + 3, FUNCTION_UPDATE_MAX_LEN - 3);

    if (rlen) {
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE, buf, rlen + 3, JL_NOT_NEED_RESPOND);
    }

    if (buf) {
        free(buf);
    }
    sport_data_func_get_finish_deal();
}

int JL_rcsp_sports_data_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    int ret = -1;
    switch (OpCode) {
    case JL_OPCODE_SPORTS_DATA_INFO_GET:
        ret = 0;
        get_sport_data_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SPORTS_DATA_INFO_SET:
        set_sport_data_info(priv, OpCode, OpCode_SN, data, len);
        ret = 0;
        break;
    }
    return ret;
}

// 处理多包的情况
static u16 sport_data_func_multi_pack_detailed_update(u8 *resp_pack, u16 resp_pack_len, sport_resp_data *sport_data, u16 offset)
{
    u8 *resp_data = resp_pack + 3;
    u8 resp_data_len = resp_pack_len - 3;

    if (!sport_data->resp_data_len) {
        return offset;
    }

    u16 data_len = sport_data->resp_data_len + 2;
    if (offset + 2 > resp_data_len) {
        resp_data[offset++] = ((u8 *)&data_len)[1];
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE, resp_pack, resp_pack_len, JL_NOT_NEED_RESPOND);
        resp_pack[2]++;
        offset = 0;
        resp_data[offset++] = ((u8 *)&data_len)[0];
    } else if (offset + 3 > resp_data_len) {
        resp_data[offset++] = ((u8 *)&data_len)[1];
        resp_data[offset++] = ((u8 *)&data_len)[0];
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE, resp_pack, resp_pack_len, JL_NOT_NEED_RESPOND);
        resp_pack[2]++;
        offset = 0;
    } else {
        resp_data[offset++] = ((u8 *)&data_len)[1];
        resp_data[offset++] = ((u8 *)&data_len)[0];
    }
    resp_data[offset++] = sport_data->resp_mask;
    resp_data[offset++] = sport_data->resp_sub_mask;
    for (data_len = 0; data_len < sport_data->resp_data_len;) {
        u16 len = 0;
        if (resp_data_len - offset > sport_data->resp_data_len - data_len) {
            len = sport_data->resp_data_len - data_len;
            memcpy(resp_data + offset, sport_data->resp_data + data_len, len);
            offset += len;
        } else {
            len = resp_data_len - offset;
            memcpy(resp_data + offset, sport_data->resp_data + data_len, len);
            offset = 0;
        }

        if (offset) {
            break;
        } else {
            JL_CMD_send(JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE, resp_pack, resp_pack_len, JL_NOT_NEED_RESPOND);
            resp_pack[2]++;
        }

        data_len += len;
    }

    return offset;
}

static int sport_data_func_multi_pack_update(void *priv, u8 *resp_pack, u16 resp_pack_len, sport_resp_data *sport_data, u16 sport_data_len)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return false;
    }
    // 假如rcspModel->err_code为0，则是普通推送
    if (JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE != rcspModel->OpCode_record) {
        // 假如不是主动推送，先回复命令
        resp_pack[0] = 0;
        resp_pack[1] = 0; // 不带response data
        JL_CMD_response_send(rcspModel->OpCode_record, JL_PRO_STATUS_SUCCESS, rcspModel->OpCode_SN_record, resp_pack, 2);
    }

    // 组装数据并发送
    resp_pack[0] = SPROT_DATA_PROTOCOL_VERSION;
    resp_pack[1] = sport_data_len / resp_pack_len + !!(sport_data_len % resp_pack_len);
    resp_pack[2] = 0;
    u16 offset = 0;
    for (u8 i = 0; i < SPORTS_DATA_FUNC_ATTR_TYPE_GET_MAX; i++) {
        offset = sport_data_func_multi_pack_detailed_update(resp_pack, resp_pack_len, sport_data + i, offset);
    }
    if (offset) {
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE, resp_pack, offset + 3, JL_NOT_NEED_RESPOND);
    }
    return 0;
}

static int sport_data_global_var_callback(void *priv)
{
    if (NULL == priv) {
        return 0;
    }
    u8 **data = (u8 **)priv;
    if (*data) {
        free(*data);
        *data = NULL;
    }
    return 0;
}

u16 add_one_attr_with_submask_huge(u8 *buf, u16 max_len, u8 offset, u8 type, u8 sub_mask, u8 *data, u16 size)
{
    if (offset + size + 4 > max_len) {
        /* printf("\n\nadd attr err!\n\n"); */
        return 0;
    }

    u16 data_len = size + 2;
    buf[offset] = ((u8 *)&data_len)[1];
    buf[offset + 1] = ((u8 *)&data_len)[0];
    buf[offset + 2] = type;
    buf[offset + 3] = sub_mask;
    memcpy(&buf[offset + 4], data, size);
    return size + 4;
}

void sport_data_global_var_deal(void *priv, u8 type, u8 sub_mask, u8 *data, u16 data_len, u8 result_len, void *callback_param)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    sport_resp_data *resp_data = (sport_resp_data *)rcspModel->priv;
    resp_data->resp_mask = type;
    resp_data->resp_sub_mask = sub_mask;
    resp_data->resp_data_len = data_len;
    resp_data->resp_data = data;
    resp_data->call_back = sport_data_global_var_callback;
    resp_data->priv = callback_param;
    if (!result_len) {
        rcspModel->err_code = 1;
    }
}

static void sport_data_regular_update(void *priv)
{
    // 隔一段时间向app推送运动数据，这里以获取心率统计值为例子
    if (!rcsp_eflash_update_flag_get()) {
        u32 mask = BIT(SPORTS_DATA_FUNC_ATTR_TYPE_HEART_RATE) | BIT(SPORTS_DATA_FUNC_ATTR_TYPE_BLOOD_OXYGEN) | BIT(SPORTS_DATA_FUNC_ATTR_TYPE_ALTITUDE) | BIT(SPORTS_DATA_FUNC_ATTR_TYPE_AIR_PRESSURE) | BIT(SPORTS_DATA_FUNC_ATTR_TYPE_ALTITUDE) | BIT(SPORTS_DATA_FUNC_ATTR_TYPE_EXERCISE_STEPS);
        u8 sub_mask[5] = {0};
        sub_mask[0] = 0x01;
        sub_mask[1] = 0x01;
        sub_mask[2] = 0x01;
        sub_mask[3] = 0x01;
        sub_mask[4] = 0x01;
        sport_data_func_update(mask, sub_mask);
    }
}

void sport_data_func_event(void *priv, u8 flag)
{
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    static u16 sport_data_update_timer = 0;
    if (flag) {
        // 连上
        if (SPORT_DATA_TIMED_NOFIFYCATION_TIME && 0 == sport_data_update_timer) {
            sport_data_update_timer = sys_timer_add(NULL, sport_data_regular_update, SPORT_DATA_TIMED_NOFIFYCATION_TIME);
        }
    } else {
        // 断开
        if (sport_data_update_timer) {
            sys_timer_del(sport_data_update_timer);
            sport_data_update_timer = 0;
        }
    }
}

void sport_data_func_init(void)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    os_sem_create(&g_sport_data_get_sem, 1);
    rcsp_msg_post(USER_MSG_RCSP_SPORT_DATA_EVENT, 2, (int)rcspModel, 1);
}

void sport_data_func_release(void)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    os_sem_del(&g_sport_data_get_sem, 0);
    rcsp_msg_post(USER_MSG_RCSP_SPORT_DATA_EVENT, 2, (int)rcspModel, 0);
}

void sport_data_func_get_prepare_deal(void)
{
    if (get_rcsp_connect_status()) {
        os_sem_pend(&g_sport_data_get_sem, 0);
    }
}

void sport_data_func_get_finish_deal(void)
{
    if (get_rcsp_connect_status()) {
        os_sem_post(&g_sport_data_get_sem);
    }
}

#else

void sport_data_func_update(u32 mask, u8 *sub_mask)
{

}

void sport_data_func_init(void)
{

}

void sport_data_func_release(void)
{

}

void sport_data_func_event(void *priv, u8 flag)
{

}

void sport_data_func_get_prepare_deal(void)
{

}

void sport_data_func_get_finish_deal(void)
{

}
#endif
