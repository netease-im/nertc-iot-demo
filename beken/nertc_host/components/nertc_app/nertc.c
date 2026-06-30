#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
// #include "cJSON.h"
#include "aud_intf.h"
#include "nertc.h"
#include "nertc_config.h"

#if CONFIG_NERTC_ENGINE_MODE == NERTC_SDK_ENGINE_MODE_LITE
#include "nertc_ext_mqtt_bk.h"
#include "nertc_ota.h"
#endif


#define TAG "nertc"
#define NERTC_JOIN_DELAY_MS 2000
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)


static nertc_t nertc_obj = {0};

#if CONFIG_NERTC_ENGINE_MODE == NERTC_SDK_ENGINE_MODE_LITE
static nertc_sdk_ext_net_handle_t s_ext_net_handle;
/* Static MQTT config strings — must outlive __nertc_init() (SDK holds raw pointers) */
static char s_mqtt_endpoint[NERTC_OTA_MQTT_ENDPOINT_LEN];
static char s_mqtt_client_id[NERTC_OTA_MQTT_CLIENT_ID_LEN];
static char s_mqtt_username[NERTC_OTA_MQTT_USERNAME_LEN];
static char s_mqtt_password[NERTC_OTA_MQTT_PASSWORD_LEN];
static char s_mqtt_publish_topic[NERTC_OTA_MQTT_PUBLISH_TOPIC_LEN];
#endif

static int __clamp_log_len(int len, int max_len)
{
    if (len <= 0)
    {
        return 0;
    }

    return (len > max_len) ? max_len : len;
}

static bool __bytes_equal(const char *buf, int buf_len, const char *literal)
{
    size_t literal_len;

    if (!buf || buf_len < 0 || !literal)
    {
        return false;
    }

    literal_len = strlen(literal);
    if ((size_t)buf_len != literal_len)
    {
        return false;
    }

    return memcmp(buf, literal, literal_len) == 0;
}

static const char *__find_bytes(const char *buf, size_t buf_len, const char *needle, size_t needle_len)
{
    size_t i;

    if (!buf || !needle || needle_len == 0 || buf_len < needle_len)
    {
        return NULL;
    }

    for (i = 0; i + needle_len <= buf_len; ++i)
    {
        if (memcmp(buf + i, needle, needle_len) == 0)
        {
            return buf + i;
        }
    }

    return NULL;
}

static bool __ai_data_state_is(const char *data, int data_len, const char *state)
{
    static const char k_state_key[] = "\"state\"";
    const char *cursor;
    size_t remain;
    size_t state_len;

    if (!data || data_len <= 0 || !state)
    {
        return false;
    }

    cursor = __find_bytes(data, (size_t)data_len, k_state_key, sizeof(k_state_key) - 1);
    if (!cursor)
    {
        return false;
    }

    cursor += sizeof(k_state_key) - 1;
    remain = (size_t)data_len - (size_t)(cursor - data);

    while (remain > 0 && (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n'))
    {
        ++cursor;
        --remain;
    }

    if (remain == 0 || *cursor != ':')
    {
        return false;
    }

    ++cursor;
    --remain;

    while (remain > 0 && (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n'))
    {
        ++cursor;
        --remain;
    }

    if (remain == 0 || *cursor != '"')
    {
        return false;
    }

    ++cursor;
    --remain;
    state_len = strlen(state);

    if (remain < state_len + 1)
    {
        return false;
    }

    if (memcmp(cursor, state, state_len) != 0)
    {
        return false;
    }

    cursor += state_len;
    remain -= state_len;

    return remain > 0 && *cursor == '"';
}

static nertc_t *__get_rtc_instance(void)
{
    return &nertc_obj;
}

static void __rtc_started(nertc_t *rtc)
{
    rtc->state = NERTC_STATE_WORKING;
}

static bool __is_rtc_started(nertc_t *rtc)
{
    if (rtc->state == NERTC_STATE_WORKING)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void __rtc_stopped(nertc_t *rtc)
{
    rtc->state = NERTC_STATE_IDLE;
}

static nertc_state_t __get_rtc_status(nertc_t *rtc)
{
    return rtc->state;
}

static void __send_message_2_user(nertc_t *rtc, nertc_msg_t *msg)
{
    if (rtc->user_message_callback)
    {
        rtc->user_message_callback(msg);
    }
}

static void __on_error(const nertc_sdk_callback_context_t* ctx, nertc_sdk_error_code_e code, const char* msg) {
    LOGE("__on_error: %d, %s\r\n", code, msg);
}

static void __on_license_expire_warning(const nertc_sdk_callback_context_t* ctx, int remaining_days)
{
    LOGW("__on_license_expire_warning: %d days remaining\r\n", remaining_days);
}

static void __on_channel_status_changed(const nertc_sdk_callback_context_t* ctx, nertc_sdk_channel_state_e status, const char *msg)
{
    LOGI("__on_channel_status_changed: %d, %s\r\n", (int)status, msg);
}

static void __on_join(const nertc_sdk_callback_context_t* ctx, uint64_t cid, uint64_t uid, nertc_sdk_error_code_e code, uint64_t elapsed, const nertc_sdk_recommended_config_t* recommended_config)
{
    LOGI("__on_join: cid:%llu, uid:%llu, code:%d, elapsed:%llu sample_rate:%d samples_per_channel:%d frame_duration:%d out_sample_rate:%d\r\n",
         cid, uid, code, elapsed,
         recommended_config->recommended_audio_config.sample_rate, recommended_config->recommended_audio_config.samples_per_channel,
         recommended_config->recommended_audio_config.frame_duration, recommended_config->recommended_audio_config.out_sample_rate);

    nertc_t *rtc = __get_rtc_instance();
    if (!rtc || __is_rtc_started(rtc))
    {
        LOGW("__on_join: rtc is null or already started\r\n");
        return;
    }

    __rtc_started(rtc);
    rtc->has_channel_joined = true;

    nertc_msg_t msg;
    msg.msg_code = NERTC_MSG_JOIN_CHANNEL_RESULT;
    msg.code = code;
    msg.uid = uid;
    __send_message_2_user(rtc, &msg);
}

static void __on_disconnect(const nertc_sdk_callback_context_t* ctx, nertc_sdk_error_code_e code, int reason)
{
    LOGE("__on_disconnect: code:%d reason:%d\r\n", code, reason);
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc || !__is_rtc_started(rtc))
    {
        LOGW("__on_disconnect: rtc is null or not started\r\n");
        return;
    }

    rtc->has_channel_joined = false;
    rtc->has_ai_started = false;

    nertc_msg_t msg;
    msg.msg_code = NERTC_MSG_CONNECTION_LOST;
    msg.code = code;
    msg.uid = 0;
    __send_message_2_user(rtc, &msg);
}

static void __on_user_joined(const nertc_sdk_callback_context_t* ctx, const nertc_sdk_user_info_t* user)
{
    LOGI("__on_user_joined: %llu, %s type:%d\r\n", user->uid, user->name, user->type);
#if 0
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc || !__is_rtc_started(rtc))
    {
        LOGW("__on_user_joined: rtc is null or not started\r\n");
        return;
    }

    rtc->has_ai_started = true;

    nertc_msg_t msg;
    msg.msg_code = NERTC_MSG_USER_JOINED;
    msg.code = 0;
    msg.uid = user->uid;
    __send_message_2_user(rtc, &msg);
#endif
    if (bk_nertc_start_listen() != BK_OK)
    {
        LOGW("failed to start manual listen on user join\r\n");
    }
}

static void __on_user_left(const nertc_sdk_callback_context_t* ctx, const nertc_sdk_user_info_t* user, int reason)
{
    LOGI("__on_user_left: %llu, %d\r\n", user->uid, reason);
#if 0
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc || !__is_rtc_started(rtc))
    {
        LOGW("__on_user_left: rtc is null or not started\r\n");
        return;
    }

    rtc->has_ai_started = false;

    nertc_msg_t msg;
    msg.msg_code = NERTC_MSG_USER_LEAVED;
    msg.code = 0;
    msg.uid = user->uid;
    __send_message_2_user(rtc, &msg);
#endif
}

static void __on_user_audio_start(const nertc_sdk_callback_context_t* ctx, uint64_t uid, nertc_sdk_media_stream_e stream_type)
{
    LOGI("__on_user_audio_start: %llu, %d\r\n", uid, stream_type);
}

static void __on_user_audio_stop(const nertc_sdk_callback_context_t* ctx, uint64_t uid, nertc_sdk_media_stream_e stream_type)
{
    LOGI("__on_user_audio_stop: %llu, %d\r\n", uid, stream_type);
}

static void __on_asr_caption_result(const nertc_sdk_callback_context_t* ctx, nertc_sdk_asr_caption_result_t* results, int result_count)
{
    for (size_t i = 0; i < result_count; i++)
    {
        LOGI(">> OnAsr:%s\r\n", results[i].content);
    }
}

static void __on_ai_data(const nertc_sdk_callback_context_t* ctx, nertc_sdk_ai_data_result_t* ai_data)
{
    int type_len;
    int data_len;

    if (!ai_data || !ai_data->type || !ai_data->data) {
        return;
    }

    type_len = ai_data->type_len;
    data_len = ai_data->data_len;

    LOGD("OnAiData type:%.*s data_len:%d\r\n",
         __clamp_log_len(type_len, 64), ai_data->type, data_len);

    /* 解析 tts 类型的 AI 数据，派发 TTS 开始/停止事件 */
    if (__bytes_equal(ai_data->type, type_len, "tts")) {
        if (__ai_data_state_is(ai_data->data, data_len, "start")) {
            LOGI(">>> TTS started (AI speaking)");
            if (bk_nertc_stop_listen() != BK_OK)
            {
                LOGW("failed to stop manual listen on tts start\r\n");
            }
        } else if (__ai_data_state_is(ai_data->data, data_len, "stop")) {
            LOGI(">>> TTS stopped (AI finished)");
            if (bk_nertc_start_listen() != BK_OK)
            {
                LOGW("failed to start manual listen on tts stop\r\n");
            }
        }
    }
}

static void __on_audio_data(const nertc_sdk_callback_context_t* ctx, uint64_t uid, nertc_sdk_media_stream_e stream_type, nertc_sdk_audio_encoded_frame_t* encoded_frame, bool is_mute_packet)
{
    nertc_t *rtc = __get_rtc_instance();
    if (!__is_rtc_started(rtc) || !rtc->has_channel_joined || !rtc->has_ai_started)
    {
        LOGD("Wrong rtc state, rtc_started: %s, channel_joined: %s, user_joined: %s \n",
             __is_rtc_started(rtc) ? "true" : "false", rtc->has_channel_joined ? "true" : "false", rtc->has_ai_started ? "true" : "false");
        return;
    }

#if 0
    static int recv_count = 0;
    static size_t recv_total_length = 0;
    if (recv_count%5 == 0)
    {
        LOGW("__on_audio_data, total_length:%d decodec_type:%d\r\n", recv_total_length, bk_aud_get_decoder_type());
    }
    recv_count++;
    recv_total_length += encoded_frame->length;
#endif

    if (rtc->audio_rx_data_handler)
    {
        rtc->audio_rx_data_handler(uid, encoded_frame, is_mute_packet);
    }
    else
    {
        LOGD("aud_rx_data_handle is NULL \n");
    }
}

static int32_t __nertc_init(nertc_config_t *p_config)
{
    // 第一步：准备创建引擎的配置
    nertc_sdk_configuration_t nertc_sdk_config = { 0 };
    nertc_sdk_configuration_init(&nertc_sdk_config);
    nertc_sdk_config.app_key = p_config->p_app_key;
    nertc_sdk_config.device_id = p_config->p_device_id;
    nertc_sdk_config.force_unsafe_mode = true;

    // license config
    nertc_sdk_config.licence_cfg.license = p_config->p_license;

    // 音频配置
    nertc_sdk_config.audio_config.channels = 1;
    nertc_sdk_config.audio_config.frame_duration = CONFIG_AUDIO_FRAME_DURATION_MS;
    nertc_sdk_config.audio_config.sample_rate = CONFIG_AUDIO_ADC_SAMP_RATE;
    nertc_sdk_config.audio_config.out_sample_rate = CONFIG_AUDIO_DAC_SAMP_RATE;
    nertc_sdk_config.audio_config.codec_type = NERTC_SDK_AUDIO_CODEC_TYPE_OPUS;

    // 可选配置
    nertc_sdk_config.optional_config.device_performance_level = NERTC_SDK_DEVICE_LEVEL_NORMAL;
    nertc_sdk_config.optional_config.enable_server_aec = p_config->enable_server_aec;
#if CONFIG_PSRAM_AS_SYS_MEMORY
    nertc_sdk_config.optional_config.prefer_use_psram = true;
#else
    nertc_sdk_config.optional_config.prefer_use_psram = false;
#endif
    nertc_sdk_config.optional_config.custom_config = p_config->p_custom_setting;

#if CONFIG_NERTC_ENGINE_MODE == NERTC_SDK_ENGINE_MODE_LITE
    /* Try OTA version check to get MQTT config from server */
    {
        nertc_ota_result_t ota_result;
        memset(&ota_result, 0, sizeof(ota_result));
        int ota_ret = nertc_ota_check_version(p_config->p_app_key,
                                               p_config->p_device_id, &ota_result);
        if (ota_ret == BK_OK && ota_result.valid) {
            /* Server returned MQTT config — copy to static buffers */
            strncpy(s_mqtt_endpoint, ota_result.mqtt_endpoint, sizeof(s_mqtt_endpoint) - 1);
            strncpy(s_mqtt_client_id, ota_result.mqtt_client_id, sizeof(s_mqtt_client_id) - 1);
            strncpy(s_mqtt_username, ota_result.mqtt_username, sizeof(s_mqtt_username) - 1);
            strncpy(s_mqtt_password, ota_result.mqtt_password, sizeof(s_mqtt_password) - 1);
            strncpy(s_mqtt_publish_topic, ota_result.mqtt_publish_topic, sizeof(s_mqtt_publish_topic) - 1);

            nertc_sdk_config.mqtt_config.endpoint      = s_mqtt_endpoint;
            nertc_sdk_config.mqtt_config.client_id     = s_mqtt_client_id;
            nertc_sdk_config.mqtt_config.username      = s_mqtt_username;
            nertc_sdk_config.mqtt_config.password      = s_mqtt_password;
            nertc_sdk_config.mqtt_config.publish_topic = s_mqtt_publish_topic;
            LOGI("OTA: using server MQTT config");
        } 
    }
#endif

    // 日志配置
    nertc_sdk_config.log_cfg.log_level = NERTC_SDK_LOG_INFO;

    // 创建引擎
    nertc_t *rtc = __get_rtc_instance();
    rtc->engine = nertc_create_engine_with_config(&nertc_sdk_config);
    if (rtc->engine == NULL) {
        LOGE(TAG, "failed to create NERtc SDK engine\r\n");
        return BK_FAIL;
    }

    // 第二步：准备初始化引擎的配置
    nertc_sdk_engine_config_t nertc_engine_config = {};
    nertc_sdk_engine_config_init(&nertc_engine_config);
    nertc_engine_config.engine_mode = CONFIG_NERTC_ENGINE_MODE;

#if CONFIG_NERTC_ENGINE_MODE == NERTC_SDK_ENGINE_MODE_LITE
    /* Fill ext_net_handle with MQTT implementation (static lifetime required by SDK) */
    memset(&s_ext_net_handle, 0, sizeof(s_ext_net_handle));
    nertc_ext_mqtt_fill_handle(&s_ext_net_handle);
    nertc_engine_config.ext_net_handle = &s_ext_net_handle;
#endif

    // 事件回调配置
    nertc_engine_config.event_handler.on_error = __on_error;
    nertc_engine_config.event_handler.on_channel_status_changed = __on_channel_status_changed;
    nertc_engine_config.event_handler.on_join = __on_join;
    nertc_engine_config.event_handler.on_disconnect = __on_disconnect;
    nertc_engine_config.event_handler.on_user_joined = __on_user_joined;
    nertc_engine_config.event_handler.on_user_left = __on_user_left;
    nertc_engine_config.event_handler.on_user_audio_start = __on_user_audio_start;
    nertc_engine_config.event_handler.on_user_audio_stop = __on_user_audio_stop;
    nertc_engine_config.event_handler.on_asr_caption_result = __on_asr_caption_result;
    nertc_engine_config.event_handler.on_ai_data = __on_ai_data;
    nertc_engine_config.event_handler.on_audio_encoded_data = __on_audio_data;
    // 用户数据
    nertc_engine_config.user_data = NULL;

    // 初始化引擎
    int ret = nertc_init_engine(rtc->engine, &nertc_engine_config);
    if (ret != 0)
    {
        LOGE(TAG, "failed to initialize NERtc SDK, error: %d\r\n", ret);
        nertc_destroy_engine(rtc->engine);
        rtc->engine = NULL;
        return BK_FAIL;
    }

    LOGI("NERtc SDK v%s \n", nertc_get_version());
    return BK_OK;
}

static void __register_message_router(nertc_t *rtc, nertc_msg_notify_cb message_callback)
{
    rtc->user_message_callback = message_callback;
}


/* API */
bk_err_t bk_nertc_create(nertc_config_t *p_config, nertc_msg_notify_cb message_callback)
{
    if (0 != __nertc_init(p_config))
    {
        LOGE("__nertc_init fail.\n");
        return BK_FAIL;
    }
    else
    {
        LOGI("__nertc_init ok.\n");
    }

    nertc_t *rtc = __get_rtc_instance();
    if (rtc)
    {
        __register_message_router(rtc, message_callback);
    }
    else
    {
        return BK_FAIL;
    }

    LOGI("nertc create successfully.\n");
    return BK_OK;
}

bk_err_t bk_nertc_destroy(void)
{
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc)
    {
        return BK_FAIL;
    }

    if (!rtc->engine)
    {
        return BK_FAIL;
    }

    nertc_destroy_engine(rtc->engine);
    rtc->state = NERTC_STATE_IDLE;
    rtc->engine = NULL;

    LOGI("nertc destroy successfully.\n");
    return BK_OK;
}

bk_err_t bk_nertc_start(nertc_option_t *p_option)
{
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc)
    {
        return BK_FAIL;
    }

    if (!rtc->engine)
    {
        return BK_FAIL;
    }

    LOGI("delay %d ms before nertc_join\r\n", NERTC_JOIN_DELAY_MS);
    rtos_delay_milliseconds(NERTC_JOIN_DELAY_MS);

    int ret = nertc_join(rtc->engine, p_option->p_channel_name, NULL, p_option->uid);
    if (ret != 0) {
        LOGE("join failed, error: %d", ret);
        return BK_FAIL;
    }

    LOGI("joining channel %s ... \n", p_option->p_channel_name);
    return BK_OK;
}

bk_err_t bk_nertc_stop(void)
{
    nertc_t *rtc = __get_rtc_instance();

    if (!rtc)
    {
        return BK_FAIL;
    }

    if (!rtc->engine)
    {
        return BK_FAIL;
    }

    __rtc_stopped(rtc);

    int ret = nertc_leave(rtc->engine);
    if (ret < 0)
    {
        LOGI("nertc_leave fail, ret=%d \n", ret);
        return BK_FAIL;
    }
    return BK_OK;
}


bk_err_t bk_nertc_start_ai(void)
{
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc)
    {
        return BK_FAIL;
    }

    if (__is_rtc_started(rtc))
    {
        nertc_sdk_asr_caption_config_t asr_config = {0};
        nertc_start_asr_caption(rtc->engine, &asr_config);
        nertc_start_ai(rtc->engine);

        rtc->has_ai_started = true;
        LOGI("start ai success!!!\r\n");
    }
    else
    {
        LOGW("start ai failed for rtc is not started\r\n");
    }
    return BK_OK;
}

bk_err_t bk_nertc_stop_ai(void)
{
    nertc_t *rtc = __get_rtc_instance();
    if (!rtc)
    {
        return BK_FAIL;
    }

    if (__is_rtc_started(rtc))
    {
        nertc_stop_ai(rtc->engine);
        nertc_stop_asr_caption(rtc->engine);

        rtc->has_ai_started = false;
        LOGI("stop ai success!!!\r\n");
    }
    else
    {
        LOGW("stop ai failed for rtc is not started\r\n");
    }
    return BK_OK;
}

bk_err_t bk_nertc_start_listen(void)
{
    nertc_t *rtc = __get_rtc_instance();
    int ret;
    if (!rtc)
    {
        return BK_FAIL;
    }

    if (!rtc->engine)
    {
        return BK_FAIL;
    }

    ret = nertc_ai_manual_start_listen(rtc->engine);
    if (ret != 0)
    {
        LOGW("nertc_ai_manual_start_listen failed: %d\r\n", ret);
        return BK_FAIL;
    }

    return BK_OK;
}

bk_err_t bk_nertc_stop_listen(void)
{
    nertc_t *rtc = __get_rtc_instance();
    int ret;
    if (!rtc)
    {
        return BK_FAIL;
    }

    if (!rtc->engine)
    {
        return BK_FAIL;
    }

    ret = nertc_ai_manual_stop_listen(rtc->engine);
    if (ret != 0)
    {
        LOGW("nertc_ai_manual_stop_listen failed: %d\r\n", ret);
        return BK_FAIL;
    }

    return BK_OK;
}

uint8_t bk_nertc_audio_codec_type_mapping(uint8_t codec_type)
{
    uint8_t aud_data_type = NERTC_SDK_AUDIO_CODEC_TYPE_PCM;
    switch(codec_type)
    {
        case AUD_INTF_VOC_DATA_TYPE_G711A:
        {
            aud_data_type = NERTC_SDK_AUDIO_CODEC_TYPE_G711;
            break;
        }
        case AUD_INTF_VOC_DATA_TYPE_PCM:
        {
            aud_data_type = NERTC_SDK_AUDIO_CODEC_TYPE_PCM;
            break;
        }

        case AUD_INTF_VOC_DATA_TYPE_OPUS:
        {
            aud_data_type = NERTC_SDK_AUDIO_CODEC_TYPE_OPUS;
            break;
        }
        default:
        {
            LOGE("%s unknown codec type:%d \r\n", __func__, codec_type);
            break;
        }
    }

    return aud_data_type;
}

int bk_nertc_audio_data_send(uint8_t *data_ptr, size_t data_len)
{
#if 0
    BK_LOGI("AUDIO", "Sending audio data: length=%zu\n", data_len);

    static uint32_t last_time = 0;
    static int total_interval = 0;
    static int count = 0;

    uint32_t current_time = bk_get_tick();

    // 计算调用间隔
    if (last_time != 0) {
        int interval = current_time - last_time;
        total_interval += interval;
        count++;

        // 每100次调用打印一次平均间隔
        if (count >= 100) {
            BK_LOGI("AUDIO", "Average call interval: %d ms (total samples: %d)\n",
                    total_interval / count, count);
            // 重置统计
            total_interval = 0;
            count = 0;
        }
    }
    last_time = current_time;
#endif

    nertc_t *rtc = __get_rtc_instance();
    if (!rtc || !rtc->engine)
    {
        return BK_FAIL;
    }

    if (!rtc->has_channel_joined || !rtc->has_ai_started)
    {
        return BK_FAIL;
    }

    nertc_sdk_audio_config_t audio_config = {0};
    audio_config.channels = 1;
    audio_config.frame_duration = 60;
    audio_config.sample_rate = 16000;
    audio_config.out_sample_rate = 16000;
    audio_config.codec_type = bk_nertc_audio_codec_type_mapping(bk_aud_get_encoder_type());

    nertc_sdk_audio_encoded_frame_t audio_encoded_frame = {0};
    audio_encoded_frame.data = data_ptr;
    audio_encoded_frame.length = data_len;
    audio_encoded_frame.payload_type = NERTC_SDK_AUDIO_ENCODE_OPUS;
    int rval = nertc_push_audio_encoded_frame(rtc->engine, NERTC_SDK_MEDIA_MAIN_AUDIO, audio_config, 100, &audio_encoded_frame);
    if (rval < 0)
    {
        LOGE("Failed to send audio data: %d\n", rval);
        return BK_FAIL;
    }
    return BK_OK;
}

bk_err_t bk_nertc_register_audio_rx_handle(nertc_audio_rx_data_handler audio_rx_handler)
{
    nertc_t *rtc = __get_rtc_instance();

    if (!rtc)
    {
        return BK_FAIL;
    }

    rtc->audio_rx_data_handler = audio_rx_handler;
    return BK_OK;
}
