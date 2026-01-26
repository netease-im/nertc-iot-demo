#ifndef XIAOZHI2_H
#define XIAOZHI2_H

#include <stdint.h>
#include <rtthread.h>
#include "lwip/api.h"
#include "lwip/apps/websocket_client.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/apps/mqtt.h"
#include "xiaozhi.h"
#include "xiaozhi2.h"
#include "bf0_hal.h"
#include "bts2_global.h"
#include "bts2_app_pan.h"
#include <cJSON.h>
#include "button.h"
#include "audio_server.h"
/**
 * @brief xiaozhi websocket cntext 数据结构体
 */
typedef struct
{
    uint32_t  sample_rate;
    uint32_t frame_duration;
    uint8_t  session_id[12];
    wsock_state_t  clnt;
    rt_sem_t sem;
    uint8_t  is_connected;
} xiaozhi_ws_t;



#endif // XIAOZHI2_H