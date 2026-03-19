//
// Created by bytedance on 2025/2/6.
//

#ifndef ONESDK_IOT_MQTT_H
#define ONESDK_IOT_MQTT_H

#include <pthread.h>
#include <libwebsockets.h>
#include "iot_basic.h"
#include "aws/common/string.h"

#define IOT_DEFAULT_PING_INTERVAL_S 60 // 60s

typedef struct {
    const char *mqtt_host;
    iot_basic_config_t *basic_config;
    struct aws_string *username;
    struct aws_string *password;
    uint16_t keep_alive;
    bool auto_reconnect;
    int32_t ping_interval;
    bool enable_mqtts;
} iot_mqtt_config_t;

typedef enum {
    MQTT_EVENT_CONNECTED,
    MQTT_EVENT_DISCONNECTED,
    MQTT_EVENT_PUBLISHED,
    MQTT_EVENT_SUBSCRIBED,
    MQTT_EVENT_UNSUBSCRIBED,
    MQTT_EVENT_MESSAGE_RECEIVED,
    MQTT_EVENT_ERROR,
} iot_mqtt_event_type_t;

typedef enum {
    IOT_MQTT_QOS0,
    IOT_MQTT_QOS1,
    // QOS2, // 暂不支持
} iot_mqtt_qos_t;

typedef void (*message_callback)(const char *topic, const uint8_t *payload, size_t len, void *user_data);
typedef void (*event_callback)(iot_mqtt_event_type_t event_type, void *user_data);

typedef struct {
    const char *topic;
    message_callback message_callback;
    event_callback event_callback;
    void *user_data;
    iot_mqtt_qos_t qos;
} iot_mqtt_topic_map_t;

// 定义待订阅列表
typedef struct {
    iot_mqtt_topic_map_t *topic_maps;
    size_t count;
} iot_mqtt_pending_sub_list_t;

typedef struct {
    lws_mqtt_publish_param_t *publish_params;
    bool *sent_list;
    size_t count;
} iot_mqtt_pending_pub_list_t;

typedef struct {
    iot_mqtt_config_t *config;
    struct lws *wsi;

    struct lws_context *context;
    struct lws_client_connect_info ccinfo;
    iot_mqtt_topic_map_t *sub_topic_maps;
    size_t sub_topic_count;
    iot_mqtt_pending_sub_list_t *pending_sub_list;
    iot_mqtt_pending_pub_list_t *pending_pub_list;
    pthread_mutex_t sub_topic_mutex;
    pthread_mutex_t pub_topic_mutex;
    bool is_connected;
    void *user_data;
    volatile bool waiting_for_suback;
    volatile bool waiting_for_puback;
} iot_mqtt_ctx_t;

int iot_mqtt_init(iot_mqtt_ctx_t *ctx, iot_mqtt_config_t *config);

void iot_mqtt_deinit(iot_mqtt_ctx_t *ctx);

int iot_mqtt_connect(iot_mqtt_ctx_t *ctx);

int iot_mqtt_disconnect(iot_mqtt_ctx_t *ctx);

int iot_mqtt_reconnect(iot_mqtt_ctx_t *ctx);

int iot_mqtt_publish(iot_mqtt_ctx_t *ctx, const char *topic,
                     const uint8_t *payload, size_t len, int qos);

int iot_mqtt_subscribe(iot_mqtt_ctx_t *ctx, iot_mqtt_topic_map_t *topic_map);

int iot_mqtt_run_event_loop(iot_mqtt_ctx_t *ctx, int timeout_ms);

#endif //ONESDK_IOT_MQTT_H
