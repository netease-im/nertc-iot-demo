#ifndef __RCSP_SENSOR_LOG_NOTIFY_H__
#define __RCSP_SENSOR_LOG_NOTIFY_H__

#include "typedef.h"
#include "app_config.h"

enum {
    SENSOR_LOG_ACCELERATION,
    SENSOR_LOG_HEART_RATE_BLOOD_OXYGEN,
    SENSOR_LOG_PEDOMETER,
};

void watch_sensor_log_notify(u8 type, u8 *data, u16 data_len);
int JL_rcsp_sensors_log_notify(void *priv, u8 OpCode, u8 *data, u16 len);

#endif
