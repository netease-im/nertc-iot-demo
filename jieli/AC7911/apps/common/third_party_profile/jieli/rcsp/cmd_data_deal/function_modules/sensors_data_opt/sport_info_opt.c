#include "rcsp_config.h"
#include "sport_info_opt.h"
#include "rcsp_event.h"
#include "rcsp_manage.h"


#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)

#define FUNCTION_UPDATE_MAX_LEN			(256)

#define ASSET_CMD_DATA_LEN(len, limit) 	\
	do{	\
		if(len >= limit){	\
		}else{				\
			return ;   \
		}\
	}while(0);

enum {
    SPORTS_INFO_OPT_GET,
    SPORTS_INFO_OPT_SET,
    SPORTS_INFO_OPT_NOTIFY,
};

extern void sport_info_sensor_opt_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_sensor_opt_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_sedentary_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_sedentary_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_continuous_heart_rate_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_continuous_heart_rate_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_exercise_heart_rate_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_exercise_heart_rate_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_pressure_detection_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_pressure_detection_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_sleep_detection_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_sleep_detection_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_fall_detection_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_fall_detection_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_raise_wrist_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_raise_wrist_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_personal_info_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_personal_info_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
extern void sport_info_bt_disconn_attr_set(void *priv, u8 attr, u8 *data, u16 len);
extern u32 sport_info_bt_disconn_attr_get(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);


static const attr_get_func target_common_sport_info_get_tab[SPORTS_INFO_OPT_FUNC_ATTR_TYPE_MAX] = {
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_RESERVE] = NULL,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_SENSOR_OPT] = sport_info_sensor_opt_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_SEDENTARY] = sport_info_sedentary_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_CONTINUOUS_HEART_RATE] = sport_info_continuous_heart_rate_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_EXERCISE_HEART_RATE] = sport_info_exercise_heart_rate_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_PRESSURE_DETECTION] = sport_info_pressure_detection_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_SLEEP_DETECTION] = sport_info_sleep_detection_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_FALL_DETECTION] = sport_info_fall_detection_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_RAISE_WRIST] = sport_info_raise_wrist_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_PERSONAL_INFO] = sport_info_personal_info_attr_get,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_BT_DISCONN] = sport_info_bt_disconn_attr_get,
};

static const attr_set_func target_common_sport_info_set_tab[SPORTS_INFO_OPT_FUNC_ATTR_TYPE_MAX] = {
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_RESERVE] = NULL,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_SENSOR_OPT] = sport_info_sensor_opt_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_SEDENTARY] = sport_info_sedentary_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_CONTINUOUS_HEART_RATE] = sport_info_continuous_heart_rate_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_EXERCISE_HEART_RATE] = sport_info_exercise_heart_rate_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_PRESSURE_DETECTION] = sport_info_pressure_detection_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_SLEEP_DETECTION] = sport_info_sleep_detection_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_FALL_DETECTION] = sport_info_fall_detection_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_RAISE_WRIST] = sport_info_raise_wrist_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_PERSONAL_INFO] = sport_info_personal_info_attr_set,
    [SPORTS_INFO_OPT_FUNC_ATTR_TYPE_BT_DISCONN] = sport_info_bt_disconn_attr_set,
};

static void get_sport_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    printf("get_sport_info\n");
    u8 offset = 0;
    u32 rlen = 0;
    u32 mask = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];
    u8 *resp = zalloc(TARGET_FEATURE_RESP_BUF_SIZE);
    if (NULL == resp) {
        printf("%s, %d, no ram!!\n", __func__, __LINE__);
        goto __get_sport_info_end;
    }

    rlen = attr_get(priv, resp, TARGET_FEATURE_RESP_BUF_SIZE, target_common_sport_info_get_tab, SPORTS_INFO_OPT_FUNC_ATTR_TYPE_MAX, mask);

__get_sport_info_end:
    if (0 == rlen) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, resp, (u16)rlen);
    }

    if (resp) {
        free(resp);
    }
}

static void set_sport_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    printf("set_sport_info\n");
    struct RcspModel *rcspModel = (struct RcspModel *)priv;
    if (rcspModel == NULL) {
        return;
    }
    put_buf(data, len);
    attr_set(priv, data, len, target_common_sport_info_set_tab, SPORTS_INFO_OPT_FUNC_ATTR_TYPE_MAX);
    if (rcspModel->err_code) {
        rcspModel->err_code = 0;
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
    }
}

void sport_info_opt_update(u32 mask)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL || 0 == JL_rcsp_get_auth_flag()) {
        return ;
    }
    u32 rlen = 0;
    u8 *buf = zalloc(FUNCTION_UPDATE_MAX_LEN);
    if (buf == NULL) {
        printf("no ram err\n");
        return;
    }
    buf[0] = SPORTS_INFO_OPT_NOTIFY;

    rlen = attr_get((void *)rcspModel, buf + 1, FUNCTION_UPDATE_MAX_LEN - 1, target_common_sport_info_get_tab, SPORTS_INFO_OPT_FUNC_ATTR_TYPE_MAX, mask);
    if (rlen) {
        JL_CMD_send(JL_OPCODE_SPORTS_DATA_INFO_OPT, buf, (u16)rlen + 1, JL_NOT_NEED_RESPOND);
    }
    if (buf) {
        free(buf);
    }
}

int JL_rcsp_sports_info_funciton(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    int ret = -1;
    if (JL_OPCODE_SPORTS_DATA_INFO_OPT == OpCode) {
        u8 op = data[0];
        switch (op) {
        case SPORTS_INFO_OPT_GET:
            get_sport_info(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORTS_INFO_OPT_SET:
            set_sport_info(priv, OpCode, OpCode_SN, data + 1, len - 1);
            break;
        case SPORTS_INFO_OPT_NOTIFY:
            break;
        }
        ret = 0;
    }
    return ret;
}

#else

void sport_info_opt_update(u32 mask)
{

}

#endif

