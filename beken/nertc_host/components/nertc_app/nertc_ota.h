/**
 * @file    nertc_ota.h
 * @brief   OTA 配置查询接口
 */

#ifndef NERTC_OTA_H
#define NERTC_OTA_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MQTT 配置字段最大长度 */
#define NERTC_OTA_MQTT_ENDPOINT_LEN       128
#define NERTC_OTA_MQTT_CLIENT_ID_LEN      128
#define NERTC_OTA_MQTT_USERNAME_LEN       128
#define NERTC_OTA_MQTT_PASSWORD_LEN       128
#define NERTC_OTA_MQTT_PUBLISH_TOPIC_LEN  128
#define NERTC_OTA_DEVICE_SDK_CONFIG_LEN   512

typedef struct {
    bool valid;
    char mqtt_endpoint[NERTC_OTA_MQTT_ENDPOINT_LEN];
    char mqtt_client_id[NERTC_OTA_MQTT_CLIENT_ID_LEN];
    char mqtt_username[NERTC_OTA_MQTT_USERNAME_LEN];
    char mqtt_password[NERTC_OTA_MQTT_PASSWORD_LEN];
    char mqtt_publish_topic[NERTC_OTA_MQTT_PUBLISH_TOPIC_LEN];

    bool has_device_sdk_config;
    char device_sdk_config[NERTC_OTA_DEVICE_SDK_CONFIG_LEN];
} nertc_ota_result_t;

/**
 * 查询 OTA 服务器获取 MQTT 配置。
 * @return BK_OK 成功且 result->valid 为 true, BK_FAIL 失败
 */
int nertc_ota_check_version(const char *app_key, const char *device_id,
                            nertc_ota_result_t *result);

#ifdef __cplusplus
}
#endif
#endif /* NERTC_OTA_H */
