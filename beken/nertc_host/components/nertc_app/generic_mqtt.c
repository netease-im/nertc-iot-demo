/**
 * @file    generic_mqtt.c
 * @brief   Generic MQTT client implementation.
 *
 *          Wraps ali_mqtt's IOT_MQTT_* bottom layer while hiding all
 *          Alibaba Cloud IoT internals.  Uses MQTT_DIRECT mode which
 *          is already enabled in the current build (sdkconfig).
 */

#include <string.h>
#include <stdlib.h>

#include "utils/iot_import.h"
#include "utils/lite-system.h"
#include "iot_export_mqtt.h"

#include "generic_mqtt.h"

#include <os/os.h>           /* os_printf for unconditional debug output */
#include <os/str.h>           /* os_snprintf, os_strtoul */

/* Keep only error-path diagnostics by default. */
#define GMQ_DEBUG 0
#if GMQ_DEBUG
#define GMQ_LOG(fmt, ...)  os_printf("[gmq] " fmt, ##__VA_ARGS__)
#else
#define GMQ_LOG(fmt, ...)  do{}while(0)
#endif

/* -------------------------------------------------------------------------- */
/*  Internals                                                                 */
/* -------------------------------------------------------------------------- */

/* Dummy credentials used to satisfy ali_mqtt's internal device-info requirement.
 * Under MQTT_DIRECT these are only used for iotx_mc_report_mid() — a harmless
 * one-shot publish at connect time. */
#define DUMMY_PRODUCT_KEY    "gen_mqtt"
#define DUMMY_DEVICE_NAME    "client"
#define DUMMY_DEVICE_SECRET  "gen_mqtt_dummy_secret"

/* Trampoline: stores user callback + ctx for subscribe */
typedef struct sub_trampoline_s {
    gen_mqtt_client_t *client;   /* back-pointer to client */
    gen_mqtt_msg_cb_t  user_cb;
    void              *user_ctx;
    struct sub_trampoline_s *next;
} sub_trampoline_t;

typedef struct {
    void               *mqtt_handle;      /* IOT_MQTT_Construct handle */
    char               *read_buf;         /* receive buffer */
    char               *write_buf;        /* send buffer */
    int                 buf_size;
    int                 is_connected;      /* track connection state */
    gen_mqtt_event_cb_t event_cb;         /* user connection-event callback */
    void               *event_ctx;
    gen_mqtt_hello_cb_t  hello_cb;          /* server hello callback */
    void                *hello_ctx;         /* server hello user context */
    gen_mqtt_raw_msg_cb_t raw_cb;           /* unmatched PUBLISH callback */
    void                *raw_ctx;           /* raw callback user context */
    sub_trampoline_t    *sub_list;         /* linked list of active subscriptions */
} gen_mqtt_internal_t;

/* -------------------------------------------------------------------------- */
/*  Forward declarations                                                      */
/* -------------------------------------------------------------------------- */
static void internal_event_handler(void *pcontext, void *pclient,
                                   iotx_mqtt_event_msg_pt msg);
static void internal_sub_callback(void *pcontext, void *pclient,
                                  iotx_mqtt_event_msg_pt msg);
static int try_parse_hello_response(const char *payload, int payload_len,
                                     gen_mqtt_hello_response_t *resp);

/* -------------------------------------------------------------------------- */
/*  Config defaults                                                           */
/* -------------------------------------------------------------------------- */

void gen_mqtt_config_defaults(gen_mqtt_config_t *cfg)
{
    if (!cfg) return;
    if (cfg->port == 0)                  cfg->port = 1883;
    if (cfg->keepalive_interval == 0)    cfg->keepalive_interval = 60;
    if (cfg->request_timeout_ms == 0)    cfg->request_timeout_ms = 2000;
    if (cfg->buf_size == 0)              cfg->buf_size = 2048;
    /* clean_session: 0 means "false", but we treat 0 as "not set" and default to 1.
     * We use a separate check: if both 0 and 1 are valid, we need a sentinel.
     * For simplicity, default clean_session to 1 unless explicitly set.
     * We distinguish "not set" by checking if port was 0 (config freshly zeroed). */
}

/* -------------------------------------------------------------------------- */
/*  Public API                                                                */
/* -------------------------------------------------------------------------- */

gen_mqtt_client_t *gen_mqtt_new(gen_mqtt_config_t *cfg)
{
    GMQ_LOG("=== gen_mqtt_new: entry ===\r\n");

    /* Step 1: validate required parameters */
    if (!cfg) {
        GMQ_LOG("FAIL step1: cfg is NULL\r\n");
        return NULL;
    }
    if (!cfg->host) {
        GMQ_LOG("FAIL step1: cfg->host is NULL\r\n");
        return NULL;
    }
    if (!cfg->client_id) {
        GMQ_LOG("FAIL step1: cfg->client_id is NULL\r\n");
        return NULL;
    }
    GMQ_LOG("step1 OK: host=%s, port=%d, client_id=%s\r\n",
            cfg->host, cfg->port, cfg->client_id);
    GMQ_LOG("step1: username=%s, password=%s, ca_cert=%s, clean_session=%d\r\n",
            cfg->username ? cfg->username : "(NULL)",
            cfg->password ? cfg->password : "(NULL)",
            cfg->ca_cert  ? "(set)" : "(NULL=plain TCP)",
            cfg->clean_session);

    gen_mqtt_config_defaults(cfg);
    if (cfg->port == 8883) {
        GMQ_LOG("step1: remap port 8883 -> 8884 for non-SSL MQTT broker\r\n");
        cfg->port = 8884;
    }
    GMQ_LOG("step1: after defaults — keepalive=%d, timeout=%d, buf_size=%d\r\n",
            cfg->keepalive_interval, cfg->request_timeout_ms, cfg->buf_size);

    /* Step 2: allocate internal structure */
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)LITE_malloc(sizeof(*g));
    if (!g) {
        GMQ_LOG("FAIL step2: LITE_malloc(internal) failed\r\n");
        return NULL;
    }
    memset(g, 0, sizeof(*g));
    g->buf_size = cfg->buf_size;
    GMQ_LOG("step2 OK: internal struct allocated (%d bytes)\r\n", (int)sizeof(*g));

    /* Step 3: allocate internal buffers */
    g->read_buf = (char *)LITE_malloc(cfg->buf_size);
    g->write_buf = (char *)LITE_malloc(cfg->buf_size);
    if (!g->read_buf || !g->write_buf) {
        GMQ_LOG("FAIL step3: buffer malloc failed (read=%p, write=%p, size=%d)\r\n",
                (void*)g->read_buf, (void*)g->write_buf, cfg->buf_size);
        LITE_free(g->read_buf);
        LITE_free(g->write_buf);
        LITE_free(g);
        return NULL;
    }
    GMQ_LOG("step3 OK: buffers allocated (read=%p, write=%p, each %d bytes)\r\n",
            (void*)g->read_buf, (void*)g->write_buf, cfg->buf_size);

    /* Step 4: initialize ali_mqtt device-info subsystem */
    iotx_device_info_init();
    iotx_device_info_set(DUMMY_PRODUCT_KEY, DUMMY_DEVICE_NAME, DUMMY_DEVICE_SECRET);
    GMQ_LOG("step4 OK: ali_mqtt device_info set (pk=%s, dn=%s)\r\n",
            DUMMY_PRODUCT_KEY, DUMMY_DEVICE_NAME);

    /* Step 5: build MQTT params and call IOT_MQTT_Construct */
    iotx_mqtt_param_t params;
    memset(&params, 0, sizeof(params));

    params.host                  = cfg->host;
    params.port                  = cfg->port;
    params.client_id             = cfg->client_id;
    params.username              = cfg->username;
    params.password              = cfg->password;
    params.pub_key               = cfg->ca_cert;       /* NULL = plain TCP */
    params.clean_session         = (uint8_t)cfg->clean_session;
    params.keepalive_interval_ms = cfg->keepalive_interval * 1000;
    params.request_timeout_ms    = cfg->request_timeout_ms;
    params.pread_buf             = g->read_buf;
    params.read_buf_size         = cfg->buf_size;
    params.pwrite_buf            = g->write_buf;
    params.write_buf_size        = cfg->buf_size;
    params.handle_event.h_fp     = internal_event_handler;
    params.handle_event.pcontext = g;

    GMQ_LOG("step5: calling IOT_MQTT_Construct (host=%s:%d, client=%s, tls=%s)...\r\n",
            params.host, params.port, params.client_id,
            params.pub_key ? "YES" : "NO (plain TCP)");

    g->mqtt_handle = IOT_MQTT_Construct(&params);
    if (!g->mqtt_handle) {
        GMQ_LOG("FAIL step5: IOT_MQTT_Construct returned NULL!\r\n");
        GMQ_LOG("  -> Check ali_mqtt logs above for STRING_PTR_SANITY_CHECK,\r\n");
        GMQ_LOG("     iotx_mc_init, iotx_mc_connect, or iotx_mc_report_mid errors.\r\n");
        LITE_free(g->read_buf);
        LITE_free(g->write_buf);
        LITE_free(g);
        return NULL;
    }

    GMQ_LOG("step5 OK: IOT_MQTT_Construct succeeded, handle=%p\r\n", g->mqtt_handle);

    g->is_connected = 1;
    GMQ_LOG("=== gen_mqtt_new: SUCCESS ===\r\n");
    return (gen_mqtt_client_t *)g;
}

int gen_mqtt_free(gen_mqtt_client_t **client)
{
    if (!client || !*client) return -1;

    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)*client;

    /* Free subscription trampolines */
    sub_trampoline_t *t = g->sub_list;
    while (t) {
        sub_trampoline_t *next = t->next;
        LITE_free(t);
        t = next;
    }
    g->sub_list = NULL;

    if (g->mqtt_handle) {
        IOT_MQTT_Destroy(&g->mqtt_handle);
        g->mqtt_handle = NULL;
    }

    LITE_free(g->read_buf);
    LITE_free(g->write_buf);
    LITE_free(g);
    *client = NULL;
    return 0;
}

int gen_mqtt_yield(gen_mqtt_client_t *client, int timeout_ms)
{
    if (!client) return -1;
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;
    return IOT_MQTT_Yield(g->mqtt_handle, timeout_ms);
}

int gen_mqtt_subscribe(gen_mqtt_client_t *client,
                       const char         *topic,
                       gen_mqtt_qos_t      qos,
                       gen_mqtt_msg_cb_t   cb,
                       void               *ctx)
{
    if (!client || !topic || !cb) return -1;

    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;

    /* Allocate a trampoline to bridge ali_mqtt callback → user callback */
    sub_trampoline_t *t = (sub_trampoline_t *)LITE_malloc(sizeof(*t));
    if (!t) return -1;
    memset(t, 0, sizeof(*t));
    t->client   = client;
    t->user_cb  = cb;
    t->user_ctx = ctx;

    int ret = IOT_MQTT_Subscribe(g->mqtt_handle, topic, (iotx_mqtt_qos_t)qos,
                                 internal_sub_callback, t);
    if (ret < 0) {
        LITE_free(t);
        return -1;
    }

    /* Prepend to subscription list (for cleanup in gen_mqtt_free) */
    t->next = g->sub_list;
    g->sub_list = t;

    return ret;
}

int gen_mqtt_unsubscribe(gen_mqtt_client_t *client, const char *topic)
{
    if (!client || !topic) return -1;
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;
    return IOT_MQTT_Unsubscribe(g->mqtt_handle, topic);
}

int gen_mqtt_publish(gen_mqtt_client_t *client,
                     const char         *topic,
                     gen_mqtt_qos_t      qos,
                     const void         *data,
                     int                 len)
{
    if (!client || !topic || !data || len <= 0) return -1;

    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;

    iotx_mqtt_topic_info_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.qos         = (uint8_t)qos;
    msg.retain      = 0;
    msg.dup         = 0;
    msg.payload     = (void *)data;
    msg.payload_len = (uint32_t)len;

    return IOT_MQTT_Publish(g->mqtt_handle, topic, &msg);
}

int gen_mqtt_is_connected(gen_mqtt_client_t *client)
{
    if (!client) return 0;
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;
    return g->is_connected;
}

int gen_mqtt_set_event_cb(gen_mqtt_client_t  *client,
                          gen_mqtt_event_cb_t  cb,
                          void                *ctx)
{
    if (!client) return -1;
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;
    g->event_cb  = cb;
    g->event_ctx = ctx;
    return 0;
}

int gen_mqtt_set_hello_cb(gen_mqtt_client_t  *client,
                           gen_mqtt_hello_cb_t cb,
                           void               *ctx)
{
    if (!client) return -1;
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;
    g->hello_cb  = cb;
    g->hello_ctx = ctx;
    return 0;
}

int gen_mqtt_set_raw_msg_cb(gen_mqtt_client_t    *client,
                             gen_mqtt_raw_msg_cb_t cb,
                             void                *ctx)
{
    if (!client) return -1;
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)client;
    g->raw_cb  = cb;
    g->raw_ctx = ctx;
    return 0;
}

/* -------------------------------------------------------------------------- */
/*  Hello handshake helpers (no cJSON — lightweight os_snprintf + strstr)    */
/* -------------------------------------------------------------------------- */

/** Generate a short random-ish request_id */
static void gen_hello_request_id(char *buf, int buf_len)
{
    static int s_seq = 0;
    unsigned int tick = (unsigned int)(uintptr_t)buf;  /* stack ASLR-ish */
    unsigned int mix = (tick ^ (unsigned int)++s_seq ^ 0x9E3779B9u);
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i;
    for (i = 0; i < 8 && i < buf_len - 1; i++) {
        buf[i] = table[(mix >> (i * 6)) & 0x3F];
    }
    buf[i] = '\0';
}

int gen_mqtt_send_hello(gen_mqtt_client_t       *client,
                         const char              *topic,
                         gen_mqtt_hello_params_t *params)
{
    if (!client || !topic || !params) {
        GMQ_LOG("send_hello: invalid args (client=%p, topic=%p, params=%p)\r\n",
                (void*)client, (void*)topic, (void*)params);
        return -1;
    }

    char request_id[16];
    gen_hello_request_id(request_id, sizeof(request_id));

    /* Build JSON with os_snprintf — lightweight, no cJSON dependency */
    char json_buf[1024];
    int len = os_snprintf(json_buf, sizeof(json_buf),
        "{"
        "\"audio_params\":{"
            "\"channels\":%d,"
            "\"format\":\"opus\","
            "\"frame_duration\":%d,"
            "\"sample_rate\":%d"
        "},"
        "\"features\":{"
            "\"mcp\":%s"
        "},"
        "\"request_id\":\"%s\","
        "\"rtp_params\":{"
            "\"pt\":%d,"
            "\"seq\":%d,"
            "\"timestamp\":%d"
        "},"
        "\"transport\":\"rtp\","
        "\"type\":\"hello\","
        "\"version\":4"
        "}",
        params->audio.channels,
        params->audio.frame_duration,
        params->audio.sample_rate,
        params->features.mcp ? "true" : "false",
        request_id,
        params->rtp.pt,
        params->rtp.seq,
        params->rtp.timestamp
    );

    if (len < 0 || len >= (int)sizeof(json_buf)) {
        GMQ_LOG("send_hello: JSON buffer overflow (len=%d)\r\n", len);
        return -1;
    }

    GMQ_LOG("send_hello: topic=%s, req_id=%s\r\n", topic, request_id);
    GMQ_LOG("send_hello: JSON=%s\r\n", json_buf);

    return gen_mqtt_publish(client, topic, GEN_MQTT_QOS0, json_buf, len);
}

/* -------------------------------------------------------------------------- */
/*  Internal event handler                                                    */
/* -------------------------------------------------------------------------- */

static void internal_event_handler(void *pcontext, void *pclient,
                                   iotx_mqtt_event_msg_pt msg)
{
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)pcontext;

    if (!g) return;

    switch (msg->event_type) {
    case IOTX_MQTT_EVENT_RECONNECT:
        g->is_connected = 1;
        if (g->event_cb) {
            g->event_cb((gen_mqtt_client_t *)g, GEN_MQTT_EVENT_CONNECTED, g->event_ctx);
        }
        break;

    case IOTX_MQTT_EVENT_DISCONNECT:
        g->is_connected = 0;
        if (g->event_cb) {
            g->event_cb((gen_mqtt_client_t *)g, GEN_MQTT_EVENT_DISCONNECTED, g->event_ctx);
        }
        break;

    case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED: {
        /*
         * Server pushes PUBLISH packets directly on the TCP connection
         * WITHOUT requiring the client to subscribe first.  When no
         * subscription matches, ali_mqtt falls through to the default
         * event handler (this function).  We catch those here — this is
         * the "MQTT-as-transport" receive path.
         */
        iotx_mqtt_topic_info_pt info = (iotx_mqtt_topic_info_pt)msg->msg;
        if (!info) break;

        GMQ_LOG("direct_pub: topic=%.*s, payload=%.*s (%d bytes)\r\n",
                (int)info->topic_len, info->ptopic,
                (int)info->payload_len, (const char *)info->payload,
                (int)info->payload_len);

        /* Check for server hello response */
        if (g->hello_cb) {
            gen_mqtt_hello_response_t resp;
            if (try_parse_hello_response((const char *)info->payload,
                                          (int)info->payload_len, &resp)) {
                GMQ_LOG("hello_response: req_id=%s, version=%d, transport=%s\r\n",
                        resp.request_id, resp.version, resp.transport);
                g->hello_cb((gen_mqtt_client_t *)g, &resp, g->hello_ctx);
            }
        }

        /* Deliver to raw message callback (unmatched topics) */
        if (g->raw_cb) {
            g->raw_cb((gen_mqtt_client_t *)g,
                      info->ptopic,   (int)info->topic_len,
                      info->payload,  (int)info->payload_len,
                      g->raw_ctx);
        }
        break;
    }

    default:
        /* SUBACK, UNSUBACK, PUBACK etc. — silently ignore */
        break;
    }
}

/**
 * @brief  Extract a string value from JSON by key.
 * Looks for "key":"value" pattern, returns the value (without quotes).
 * Modifies *buf in-place (null-terminates the value).
 * Returns pointer to value on success, NULL if not found.
 */
static char *json_get_str(char *buf, const char *key)
{
    char search[128];
    int slen = os_snprintf(search, sizeof(search), "\"%s\":\"", key);
    if (slen < 0 || slen >= (int)sizeof(search)) return NULL;

    char *start = strstr(buf, search);
    if (!start) return NULL;

    start += slen;  /* skip past "key":" */
    char *end = strchr(start, '"');
    if (!end) return NULL;

    *end = '\0';
    return start;
}

/**
 * @brief  Extract an integer value from JSON by key.
 * Looks for "key":number pattern.
 * Returns 1 on success (value written to *out), 0 if not found.
 */
static int json_get_int(const char *buf, const char *key, int *out)
{
    char search[128];
    int slen = os_snprintf(search, sizeof(search), "\"%s\":", key);
    if (slen < 0 || slen >= (int)sizeof(search)) return 0;

    const char *start = strstr(buf, search);
    if (!start) return 0;

    start += slen;
    /* Parse integer (skip whitespace, handle negative) */
    while (*start == ' ') start++;
    *out = (int)os_strtoul(start, NULL, 10);
    return 1;
}

/**
 * @brief  Try to parse an incoming PUBLISH payload as a server hello response.
 *
 * Uses lightweight string scanning — no cJSON dependency.
 * If the payload contains "type":"hello", populates *resp and returns 1.
 * Otherwise returns 0 (not a hello message).
 */
static int try_parse_hello_response(const char *payload, int payload_len,
                                     gen_mqtt_hello_response_t *resp)
{
    /* Quick pre-check */
    if (payload_len < 20) return 0;
    if (!strstr(payload, "\"hello\"")) return 0;

    /* Make a null-terminated mutable copy */
    char *buf = (char *)LITE_malloc(payload_len + 1);
    if (!buf) return 0;
    memcpy(buf, payload, payload_len);
    buf[payload_len] = '\0';

    memset(resp, 0, sizeof(*resp));

    /* Extract key fields */
    char *rid = json_get_str(buf, "request_id");
    if (rid) {
        strncpy(resp->request_id, rid, sizeof(resp->request_id) - 1);
    }

    json_get_int(buf, "version", &resp->version);

    char *trans = json_get_str(buf, "transport");
    if (trans) {
        strncpy(resp->transport, trans, sizeof(resp->transport) - 1);
    }

    /* Save raw JSON */
    strncpy(resp->raw, buf, sizeof(resp->raw) - 1);
    resp->raw[sizeof(resp->raw) - 1] = '\0';

    LITE_free(buf);
    return 1;
}

/**
 * @brief  Trampoline: ali_mqtt subscription callback → user gen_mqtt_msg_cb_t.
 *
 * ali_mqtt calls this for ALL events on this subscription (SUBACK, PUBLISH, etc.).
 * We filter for IOTX_MQTT_EVENT_PUBLISH_RECVEIVED and extract topic+payload info.
 * Additionally, if the payload is a JSON "hello" response, we also deliver it
 * to the registered hello callback.
 */
static void internal_sub_callback(void *pcontext, void *pclient,
                                  iotx_mqtt_event_msg_pt msg)
{
    sub_trampoline_t *t = (sub_trampoline_t *)pcontext;

    if (!t || !t->user_cb) return;

    if (msg->event_type != IOTX_MQTT_EVENT_PUBLISH_RECVEIVED) {
        return;  /* SUBACK, NACK, TIMEOUT — silently ignore */
    }

    iotx_mqtt_topic_info_pt info = (iotx_mqtt_topic_info_pt)msg->msg;
    if (!info) return;

    /* ---- Check for hello response (before user callback) ---- */
    gen_mqtt_internal_t *g = (gen_mqtt_internal_t *)t->client;
    if (g && g->hello_cb) {
        gen_mqtt_hello_response_t resp;
        if (try_parse_hello_response((const char *)info->payload,
                                      (int)info->payload_len, &resp)) {
            GMQ_LOG("hello_response: req_id=%s, version=%d, transport=%s\r\n",
                    resp.request_id, resp.version, resp.transport);
            g->hello_cb(t->client, &resp, g->hello_ctx);
            /* Fall through — also deliver to user's per-topic callback */
        }
    }

    t->user_cb(t->client,                                         /* client */
               info->ptopic,   (int)info->topic_len,          /* topic, topic_len */
               info->payload,  (int)info->payload_len,        /* payload, payload_len */
               t->user_ctx);                                  /* ctx */
}
