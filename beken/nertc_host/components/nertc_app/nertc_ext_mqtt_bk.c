/**
 * @file    nertc_ext_mqtt.c
 * @brief   BK7258 ext_net_handle MQTT implementation.
 *
 *          Bridges generic_mqtt.c (ali_mqtt wrapper) to the SDK's
 *          nertc_sdk_ext_net_handle_t interface, enabling lite_mode
 *          MQTT-RTP signaling on BK7258.
 *
 *  时序保证:
 *    NERtcMqttClientExternal 构造顺序:
 *      1. create_mqtt()        → 只分配 wrapper，不连接
 *      2. set_mqtt_on_*()      → 注册 SDK 回调到 wrapper
 *      3. mqtt_connect()       → 发起 gen_mqtt_new 连接
 *    这样回调注册在连接之前，事件不会丢失。
 */

#include "nertc_ext_mqtt_bk.h"
#include "generic_mqtt.h"

#include <string.h>
#include <stdlib.h>
#include <os/os.h>

#define TAG "nertc_ext_mqtt"

/* -------------------------------------------------------------------------- */
/*  Wrapper struct — one per mqtt_handle                                       */
/* -------------------------------------------------------------------------- */

typedef struct {
    gen_mqtt_client_t *gmqtt;               /* generic_mqtt handle (created in mqtt_connect) */
    bool               gmqtt_freed;          /* guard against double-free */

    /* SDK callbacks — set by set_mqtt_on_* before mqtt_connect */
    mqtt_on_connected_func    on_connected;
    mqtt_on_disconnected_func on_disconnected;
    mqtt_on_message_func      on_message;
    mqtt_on_error_func        on_error;

    /* Yield task */
    beken_thread_t  yield_thread;
    volatile bool   yield_running;
} bk_mqtt_wrapper_t;

/* -------------------------------------------------------------------------- */
/*  Forward declarations                                                      */
/* -------------------------------------------------------------------------- */

static void bridge_event_cb(gen_mqtt_client_t *client, gen_mqtt_event_t event, void *ctx);
static void bridge_raw_msg_cb(gen_mqtt_client_t *client, const char *topic, int topic_len,
                               const void *payload, int payload_len, void *ctx);
static void yield_task(void *arg);

/* -------------------------------------------------------------------------- */
/*  create / destroy                                                          */
/* -------------------------------------------------------------------------- */

static mqtt_handle bk_create_mqtt(void) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)malloc(sizeof(bk_mqtt_wrapper_t));
    if (!w) {
        os_printf("[%s] create_mqtt: malloc failed\n", TAG);
        return NULL;
    }
    memset(w, 0, sizeof(*w));
    return (mqtt_handle)w;
}

static void bk_destroy_mqtt(mqtt_handle handle) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w) return;

    /* stop yield task */
    w->yield_running = false;

    /* free gen_mqtt if not already freed by disconnect */
    if (w->gmqtt && !w->gmqtt_freed) {
        gen_mqtt_free(&w->gmqtt);
    }

    free(w);
}

/* -------------------------------------------------------------------------- */
/*  callback setters — called BEFORE mqtt_connect                             */
/* -------------------------------------------------------------------------- */

static void bk_set_mqtt_on_connected(mqtt_handle handle, mqtt_on_connected_func callback) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (w) w->on_connected = callback;
}

static void bk_set_mqtt_on_disconnected(mqtt_handle handle, mqtt_on_disconnected_func callback) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (w) w->on_disconnected = callback;
}

static void bk_set_mqtt_on_message(mqtt_handle handle, mqtt_on_message_func callback) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (w) w->on_message = callback;
}

static void bk_set_mqtt_on_error(mqtt_handle handle, mqtt_on_error_func callback) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (w) w->on_error = callback;
}

/* -------------------------------------------------------------------------- */
/*  connect — called AFTER callback registration                              */
/* -------------------------------------------------------------------------- */

static bool bk_mqtt_connect(mqtt_handle handle, const char *host, int port,
                             const char *client_id, const char *username,
                             const char *password) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w) return false;

    os_printf("[%s] connect: host=%s:%d\n", TAG, host, port);

    /* free previous connection if reconnecting */
    if (w->gmqtt && !w->gmqtt_freed) {
        gen_mqtt_free(&w->gmqtt);
    }

    /* build config from SDK-supplied params */
    gen_mqtt_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.host               = host;
    cfg.port               = (uint16_t)port;
    cfg.client_id          = client_id;
    cfg.username           = username;
    cfg.password           = password;
    cfg.ca_cert            = NULL;   /* plain TCP */
    cfg.keepalive_interval = 120;
    cfg.clean_session      = 1;

    /* gen_mqtt_new is synchronous — blocks until MQTT CONNECT completes */
    w->gmqtt = gen_mqtt_new(&cfg);
    if (!w->gmqtt) {
        os_printf("[%s] connect: gen_mqtt_new failed\n", TAG);
        return false;
    }
    w->gmqtt_freed = false;

    /* register bridge callbacks (CONNECTED already fired inside gen_mqtt_new,
     * but subsequent reconnection events will be delivered via these callbacks) */
    gen_mqtt_set_event_cb(w->gmqtt, bridge_event_cb, w);
    gen_mqtt_set_raw_msg_cb(w->gmqtt, bridge_raw_msg_cb, w);

    /* manually fire initial CONNECTED (gen_mqtt_new completed synchronously) */
    if (w->on_connected) {
        w->on_connected(handle);
    }

    /* ali_mqtt already starts mqtt_recv_thread internally and drives
     * IOT_MQTT_Yield() there. A second external yield loop on the same handle
     * causes packet parsing and state corruption on BK. */
    w->yield_running = false;
    w->yield_thread = NULL;
    os_printf("[%s] connect: external yield disabled, ali_mqtt mqtt_recv_thread owns yield\n", TAG);

    os_printf("[%s] connect: OK\n", TAG);
    return true;
}

/* -------------------------------------------------------------------------- */
/*  disconnect — TCP/MQTT teardown, keep wrapper alive                        */
/* -------------------------------------------------------------------------- */

static void bk_mqtt_disconnect(mqtt_handle handle) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w) return;
    os_printf("[%s] disconnect\n", TAG);

    /* stop yield task */
    w->yield_running = false;

    /* free gen_mqtt (disconnects TCP + MQTT) */
    if (w->gmqtt && !w->gmqtt_freed) {
        gen_mqtt_free(&w->gmqtt);
        w->gmqtt_freed = true;
    }
}

/* -------------------------------------------------------------------------- */
/*  query / publish / subscribe / unsubscribe                                 */
/* -------------------------------------------------------------------------- */

static bool bk_mqtt_is_connected(mqtt_handle handle) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w || !w->gmqtt || w->gmqtt_freed) return false;
    return gen_mqtt_is_connected(w->gmqtt) != 0;
}

static bool bk_mqtt_publish(mqtt_handle handle, const char *topic,
                             const char *payload, int qos) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w || !w->gmqtt || w->gmqtt_freed) return false;
    return gen_mqtt_publish(w->gmqtt, topic, (gen_mqtt_qos_t)qos,
                            payload, (int)strlen(payload)) >= 0;
}

static bool bk_mqtt_subscribe(mqtt_handle handle, const char *topic, int qos) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w || !topic) return false;

    os_printf("[%s] subscribe unsupported in publish-only mode: topic=%s qos=%d\n", TAG, topic, qos);
    return false;
}

static bool bk_mqtt_unsubscribe(mqtt_handle handle, const char *topic) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)handle;
    if (!w || !topic) return false;

    os_printf("[%s] unsubscribe unsupported in publish-only mode: topic=%s\n", TAG, topic);
    return false;
}

/* -------------------------------------------------------------------------- */
/*  Bridge callbacks: generic_mqtt → SDK                                      */
/* -------------------------------------------------------------------------- */

static void bridge_event_cb(gen_mqtt_client_t *client, gen_mqtt_event_t event, void *ctx) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)ctx;
    if (!w) return;

    mqtt_handle handle = (mqtt_handle)w;

    switch (event) {
    case GEN_MQTT_EVENT_CONNECTED:
        if (w->on_connected) {
            w->on_connected(handle);
        }
        break;
    case GEN_MQTT_EVENT_DISCONNECTED:
        os_printf("[%s] event: DISCONNECTED\n", TAG);
        if (w->on_disconnected) {
            w->on_disconnected(handle);
        }
        break;
    }
}

static void bridge_raw_msg_cb(gen_mqtt_client_t *client, const char *topic, int topic_len,
                               const void *payload, int payload_len, void *ctx) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)ctx;
    if (!w || !w->on_message) return;

    /* SDK callback expects null-terminated strings — copy to temp buffers */
    char topic_buf[256];
    int copy_len = (topic_len < (int)sizeof(topic_buf) - 1)
                   ? topic_len : (int)sizeof(topic_buf) - 1;
    memcpy(topic_buf, topic, (size_t)copy_len);
    topic_buf[copy_len] = '\0';

    char *payload_buf = (char *)malloc((size_t)payload_len + 1);
    if (!payload_buf) return;
    memcpy(payload_buf, payload, (size_t)payload_len);
    payload_buf[payload_len] = '\0';

    w->on_message((mqtt_handle)w, topic_buf, payload_buf);

    free(payload_buf);
}

/* -------------------------------------------------------------------------- */
/*  Yield task — drives gen_mqtt_yield every 50 ms                             */
/* -------------------------------------------------------------------------- */

static void yield_task(void *arg) {
    bk_mqtt_wrapper_t *w = (bk_mqtt_wrapper_t *)arg;
    if (!w) {
        rtos_delete_thread(NULL);
        return;
    }

    os_printf("[%s] yield_task: started\n", TAG);

    while (w->yield_running) {
        if (w->gmqtt && !w->gmqtt_freed) {
            gen_mqtt_yield(w->gmqtt, 50);
        }
        rtos_delay_milliseconds(50);
    }

    os_printf("[%s] yield_task: exiting\n", TAG);
    w->yield_thread = NULL;
    rtos_delete_thread(NULL);
}

/* -------------------------------------------------------------------------- */
/*  Public API                                                                */
/* -------------------------------------------------------------------------- */

void nertc_ext_mqtt_fill_handle(nertc_sdk_ext_net_handle_t *handle) {
    if (!handle) return;

    /* MQTT function pointers */
    handle->create_mqtt               = bk_create_mqtt;
    handle->destroy_mqtt              = bk_destroy_mqtt;
    handle->set_mqtt_on_connected     = bk_set_mqtt_on_connected;
    handle->set_mqtt_on_disconnected  = bk_set_mqtt_on_disconnected;
    handle->set_mqtt_on_message       = bk_set_mqtt_on_message;
    handle->set_mqtt_on_error         = bk_set_mqtt_on_error;
    handle->mqtt_connect              = bk_mqtt_connect;
    handle->mqtt_disconnect           = bk_mqtt_disconnect;
    handle->mqtt_is_connected         = bk_mqtt_is_connected;
    handle->mqtt_publish              = bk_mqtt_publish;
    handle->mqtt_subscribe            = bk_mqtt_subscribe;
    handle->mqtt_unsubscribe          = bk_mqtt_unsubscribe;

    /* HTTP / TCP / UDP remain NULL — SDK will use internal implementations */

    os_printf("[%s] ext_net_handle filled (MQTT only)\n", TAG);
}
