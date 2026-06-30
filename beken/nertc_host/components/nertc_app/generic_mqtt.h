/**
 * @file    generic_mqtt.h
 * @brief   通用 MQTT 客户端接口 — 封装 ali_mqtt, 隐藏阿里云内部细节
 */

#ifndef GENERIC_MQTT_H
#define GENERIC_MQTT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gen_mqtt_client_s gen_mqtt_client_t;

typedef enum {
    GEN_MQTT_QOS0 = 0,
    GEN_MQTT_QOS1 = 1,
    GEN_MQTT_QOS2 = 2,
} gen_mqtt_qos_t;

typedef enum {
    GEN_MQTT_EVENT_CONNECTED = 0,
    GEN_MQTT_EVENT_DISCONNECTED,
} gen_mqtt_event_t;

/* 配置 */
typedef struct {
    const char *host;
    uint16_t    port;
    const char *client_id;
    const char *username;
    const char *password;
    const char *ca_cert;          /* NULL = plain TCP */
    uint16_t    keepalive_interval; /* 秒, 0=默认60 */
    uint16_t    request_timeout_ms; /* 0=默认2000 */
    int         buf_size;          /* 0=默认2048 */
    int         clean_session;     /* 0/1 */
} gen_mqtt_config_t;

/* 回调类型 */
typedef void (*gen_mqtt_event_cb_t)(gen_mqtt_client_t *client,
                                    gen_mqtt_event_t event, void *ctx);
typedef void (*gen_mqtt_msg_cb_t)(gen_mqtt_client_t *client,
                                   const char *topic, int topic_len,
                                   const void *payload, int payload_len,
                                   void *ctx);
typedef void (*gen_mqtt_raw_msg_cb_t)(gen_mqtt_client_t *client,
                                       const char *topic, int topic_len,
                                       const void *payload, int payload_len,
                                       void *ctx);

/* Hello 握手 */
typedef struct {
    struct { int channels; int frame_duration; int sample_rate; } audio;
    struct { int pt; int seq; int timestamp; } rtp;
    struct { int mcp; } features;
} gen_mqtt_hello_params_t;

typedef struct {
    char request_id[16];
    int  version;
    char transport[16];
    char raw[512];
} gen_mqtt_hello_response_t;

typedef void (*gen_mqtt_hello_cb_t)(gen_mqtt_client_t *client,
                                    gen_mqtt_hello_response_t *resp,
                                    void *ctx);

/* API */
void gen_mqtt_config_defaults(gen_mqtt_config_t *cfg);
gen_mqtt_client_t *gen_mqtt_new(gen_mqtt_config_t *cfg);
int gen_mqtt_free(gen_mqtt_client_t **client);
int gen_mqtt_yield(gen_mqtt_client_t *client, int timeout_ms);
int gen_mqtt_subscribe(gen_mqtt_client_t *client, const char *topic,
                       gen_mqtt_qos_t qos, gen_mqtt_msg_cb_t cb, void *ctx);
int gen_mqtt_unsubscribe(gen_mqtt_client_t *client, const char *topic);
int gen_mqtt_publish(gen_mqtt_client_t *client, const char *topic,
                     gen_mqtt_qos_t qos, const void *data, int len);
int gen_mqtt_is_connected(gen_mqtt_client_t *client);
int gen_mqtt_set_event_cb(gen_mqtt_client_t *client,
                          gen_mqtt_event_cb_t cb, void *ctx);
int gen_mqtt_set_hello_cb(gen_mqtt_client_t *client,
                          gen_mqtt_hello_cb_t cb, void *ctx);
int gen_mqtt_set_raw_msg_cb(gen_mqtt_client_t *client,
                            gen_mqtt_raw_msg_cb_t cb, void *ctx);
int gen_mqtt_send_hello(gen_mqtt_client_t *client, const char *topic,
                        gen_mqtt_hello_params_t *params);

#ifdef __cplusplus
}
#endif
#endif /* GENERIC_MQTT_H */
