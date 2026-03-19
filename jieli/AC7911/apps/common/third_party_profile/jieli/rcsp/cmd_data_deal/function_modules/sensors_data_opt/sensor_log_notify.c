#include "rcsp_config.h"
#include "sensor_log_notify.h"

#if (RCSP_MODE && JL_RCSP_SENSORS_DATA_OPT)
#define FUNCTION_UPDATE_MAX_LEN			(256)

static int pedometer_data_get(u8 *data, u16 max_len)
{
    int rlen = 0;
#if TCFG_GSENSOR_ENABLE
    extern int gSensor_read_data(u8 * buf[]);
    u8 *tmp_data = NULL;
    rlen = gSensor_read_data(&tmp_data);
    if (rlen > max_len) {
        rlen = 0;
    } else {
        memcpy(data, tmp_data, rlen);
    }
#endif
    return rlen;
}

static void sensors_log_nofity_trigger(void *priv, u8 OpCode, u8 *data, u16 len)
{
    u8 ret = 0;
    u8 offset = 0;
    u32 mask = data[offset++] << 24 | data[offset++] << 16 | data[offset++] << 8 | data[offset++];

    u8 *resp_data = zalloc(FUNCTION_UPDATE_MAX_LEN);
    if (NULL == resp_data) {
        ret = -1;
        printf("%s, no ram err !!!\n", __func__);
        goto __sensors_log_nofity_trigger_end;
    }

    if (mask & BIT(SENSOR_LOG_ACCELERATION)) {
        // 触发加速度
        u16 data_len = 0;
        resp_data[data_len++] = 0xa5;
        watch_sensor_log_notify(SENSOR_LOG_ACCELERATION, resp_data, data_len);
    }

    if (mask & BIT(SENSOR_LOG_HEART_RATE_BLOOD_OXYGEN)) {
        // 触发血氧
        u16 data_len = 0;
        resp_data[data_len++] = 0x5a;
        watch_sensor_log_notify(SENSOR_LOG_HEART_RATE_BLOOD_OXYGEN, resp_data, data_len);
    }

    if (mask & BIT(SENSOR_LOG_PEDOMETER)) {
        u16 data_len = 0;
        data_len = pedometer_data_get(resp_data, len);
        watch_sensor_log_notify(SENSOR_LOG_PEDOMETER, resp_data, data_len);
    }

__sensors_log_nofity_trigger_end:
    if (resp_data) {
        free(resp_data);
    }
}

int JL_rcsp_sensors_log_notify(void *priv, u8 OpCode, u8 *data, u16 len)
{
    int ret = -1;
    switch (OpCode) {
    case JL_OPCODE_SENSOR_LOG_DATA_AUTO_UPDATE:
        ret = 0;
        sensors_log_nofity_trigger(priv, OpCode, data, len);
        break;
    }
    return ret;
}

void watch_sensor_log_notify(u8 type, u8 *data, u16 data_len)
{
    struct RcspModel *rcspModel = rcsp_handle_get();
    if (rcspModel == NULL || 0 == JL_rcsp_get_auth_flag()) {
        return ;
    }

    u8 *resp_data = zalloc(FUNCTION_UPDATE_MAX_LEN);
    if (NULL == resp_data) {
        printf("%s, no ram err !!!\n", __func__);
        return;
    }

    for (u16 offset = 0, rlen = 0; offset < data_len; offset += FUNCTION_UPDATE_MAX_LEN - 1) {
        rlen = (data_len - offset) > (FUNCTION_UPDATE_MAX_LEN - 1)	? (FUNCTION_UPDATE_MAX_LEN - 1) : (data_len - offset);
        u32 mask = BIT(type);
        resp_data[0] = ((u8 *)&mask)[3];
        resp_data[1] = ((u8 *)&mask)[2];
        resp_data[2] = ((u8 *)&mask)[1];
        resp_data[3] = ((u8 *)&mask)[0];
        memcpy(resp_data + 4, data + offset, rlen);
        if (JL_CMD_send(JL_OPCODE_SENSOR_LOG_DATA_AUTO_UPDATE, resp_data, rlen + 4, JL_NOT_NEED_RESPOND)) {
            break;
        }
    }

    if (resp_data) {
        free(resp_data);
    }
}

#else

int JL_rcsp_sensors_log_notify(void *priv, u8 OpCode, u8 *data, u16 len)
{
    return 0;
}

void watch_sensor_log_notify(u8 type, u8 *data, u16 data_len)
{

}

#endif
