#ifndef _APP_GIZWITS_MQTT_H
#define _APP_GIZWITS_MQTT_H
#include "system/includes.h"

#define FIXED_HEADER                0x00000003
#define CMD_VERSION_REPORT          0x021c

#define MODULE_BITS                 (BIT(1))   
#define MODULE_HW_SW_VER          "000001,CUBE001"          // 硬件版本和软件版本


#define MQTT_PUBLISH_VERSION        1
#define MQTT_PUBLISH_GET_ROOM       2

#define MQTT_VERSION_PUBLISH_TOPIC             "cli2ser_req"
#define MQTT_GET_ROOM_PUBLISH_TOPIC             "llm/%s/config/request"

#define MQTT_INFO_QOS                           QOS0

int mqtt_publish_info(u8 flag);

void mqtt_reconnect(void);

void mqtt_task_kill(void);

#endif