/**
 * @file nertc_protocol.c
 * @brief NERtc Protocol Wrapper for Jieli Platform
 * 
 * 基于 nertc_protocol_test.cc 参考实现
 * 为杰理平台（C语言）封装 NERTC SDK
 */

#include "app_config.h"
#include "system/includes.h"
#include "nertc_protocol.h"
#include "nertc_sdk.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================================
 * 日志宏定义
 * ============================================================================ */
#define TAG "NERTC"
#define NERTC_LOGI(fmt, ...) printf("[%s] " fmt "\n", TAG, ##__VA_ARGS__)
#define NERTC_LOGE(fmt, ...) printf("[%s][E] " fmt "\n", TAG, ##__VA_ARGS__)
#define NERTC_LOGW(fmt, ...) printf("[%s][W] " fmt "\n", TAG, ##__VA_ARGS__)

/* 默认 custom_config JSON 字符串 */
#define NERTC_DEFAULT_CUSTOM_CONFIG  "{\"test_mode\":true,\"asr\":true}"

/* Join 超时时间（毫秒） */
#define NERTC_JOIN_TIMEOUT_MS        10000

/* ============================================================================
 * 私有数据定义
 * ============================================================================ */

/* 协议上下文结构体 */
typedef struct {
    /* 引擎实例 */
    nertc_sdk_engine_t engine;
    
    /* 状态标志 */
    volatile bool initialized;
    volatile bool joined;
    volatile bool audio_channel_opened;
    
    /* Join 同步信号量 */
    OS_SEM join_sem;
    volatile int join_result;  /* 0: 成功, 其他: 错误码 */
    
    /* 房间信息 */
    char cname[64];
    uint64_t cid;
    uint64_t uid;
    
    /* 音频配置（从服务端推荐） */
    int server_sample_rate;
    int server_out_sample_rate;
    int server_frame_duration;
    int samples_per_channel;
    nertc_sdk_audio_config_t recommended_audio_config;
    
    /* 功能开关 */
    bool asr_enabled;
    
    /* 用户配置副本 */
    nertc_protocol_config_t config;
    
    /* 回调 */
    nertc_protocol_event_callback_t event_callback;
    void *event_user_data;
    nertc_protocol_audio_callback_t audio_callback;
    void *audio_user_data;
    
} nertc_protocol_context_t;

/* 全局上下文 */
static nertc_protocol_context_t g_ctx = {0};

/* ============================================================================
 * 辅助函数
 * ============================================================================ */

/* 发送事件到回调 */
static void dispatch_event(nertc_protocol_event_e event_type)
{
    if (g_ctx.event_callback) {
        nertc_protocol_event_t evt;
        memset(&evt, 0, sizeof(evt));
        evt.event = event_type;
        g_ctx.event_callback(&evt, g_ctx.event_user_data);
    }
}

/* 发送错误事件 */
static void dispatch_error_event(int code, const char *msg)
{
    if (g_ctx.event_callback) {
        nertc_protocol_event_t evt;
        memset(&evt, 0, sizeof(evt));
        evt.event = NERTC_PROTOCOL_EVENT_ERROR;
        evt.data.error.error_code = code;
        evt.data.error.error_msg = msg;
        g_ctx.event_callback(&evt, g_ctx.event_user_data);
    }
}

/* 发送连接成功事件 */
static void dispatch_connected_event(uint64_t cid, uint64_t uid, 
                                     const nertc_sdk_recommended_config_t *cfg)
{
    if (g_ctx.event_callback) {
        nertc_protocol_event_t evt;
        memset(&evt, 0, sizeof(evt));
        evt.event = NERTC_PROTOCOL_EVENT_CONNECTED;
        evt.data.connected.cid = cid;
        evt.data.connected.uid = uid;
        if (cfg) {
            evt.data.connected.sample_rate = cfg->recommended_audio_config.sample_rate;
            evt.data.connected.out_sample_rate = cfg->recommended_audio_config.out_sample_rate;
            evt.data.connected.frame_duration = cfg->recommended_audio_config.frame_duration;
            evt.data.connected.samples_per_channel = cfg->recommended_audio_config.samples_per_channel;
        }
        g_ctx.event_callback(&evt, g_ctx.event_user_data);
    }
}

/* 发送用户事件 */
static void dispatch_user_event(nertc_protocol_event_e event_type, 
                                const nertc_sdk_user_info_t *user)
{
    if (g_ctx.event_callback && user) {
        nertc_protocol_event_t evt;
        memset(&evt, 0, sizeof(evt));
        evt.event = event_type;
        evt.data.user.uid = user->uid;
        evt.data.user.name = user->name;
        evt.data.user.user_type = user->type;
        g_ctx.event_callback(&evt, g_ctx.event_user_data);
    }
}

/* 发送音频数据事件 */
static void dispatch_audio_data_event(uint64_t uid,
                                      nertc_sdk_audio_encoded_frame_t *frame,
                                      bool is_mute)
{
    if (g_ctx.audio_callback && frame && frame->data && frame->length > 0) {
        /* VBR header 缓冲区: 8字节头 + 原始数据 */
        #define VBR_HEADER_SIZE 8
        static uint8_t vbr_buf[VBR_HEADER_SIZE + 2048];  /* 静态缓冲区，避免频繁分配 */
        
        uint32_t packet_len = frame->length;
        
        /* 检查数据长度是否超出缓冲区 */
        if (packet_len + VBR_HEADER_SIZE > sizeof(vbr_buf)) {
            NERTC_LOGE("Audio frame too large: %u", packet_len);
            return;
        }
        
        /* 添加大端帧头 (4字节包长 + 4字节range校验值) */
        vbr_buf[0] = (packet_len >> 24) & 0xFF;
        vbr_buf[1] = (packet_len >> 16) & 0xFF;
        vbr_buf[2] = (packet_len >> 8) & 0xFF;
        vbr_buf[3] = packet_len & 0xFF;
        vbr_buf[4] = vbr_buf[5] = vbr_buf[6] = vbr_buf[7] = 0;  /* range校验值设为0 */
        
        /* 拷贝原始OPUS数据 */
        memcpy(vbr_buf + VBR_HEADER_SIZE, frame->data, packet_len);
        
        nertc_protocol_audio_data_t audio;
        memset(&audio, 0, sizeof(audio));
        audio.uid = uid;
        audio.data = vbr_buf;
        audio.length = packet_len + VBR_HEADER_SIZE;
        audio.timestamp_ms = frame->timestamp_ms;
        audio.encoded_timestamp = frame->encoded_timestamp;
        audio.sample_rate = g_ctx.recommended_audio_config.out_sample_rate;
        audio.frame_duration = g_ctx.server_frame_duration;
        audio.is_mute_packet = is_mute;
        g_ctx.audio_callback(&audio, g_ctx.audio_user_data);
        static int index = 0;
    }
}

/* ============================================================================
 * NERTC SDK 事件回调实现
 * ============================================================================ */

static void nertc_on_error(const nertc_sdk_callback_context_t* ctx, 
                           nertc_sdk_error_code_e code, 
                           const char* msg)
{
    NERTC_LOGE("on_error: code=%d, msg=%s", code, msg ? msg : "null");
    dispatch_error_event((int)code, msg);
}

static void nertc_on_license_expire_warning(const nertc_sdk_callback_context_t* ctx, 
                                            int remaining_days)
{
    NERTC_LOGW("on_license_expire_warning: remaining_days=%d", remaining_days);
}

static void nertc_on_channel_status_changed(const nertc_sdk_callback_context_t* ctx, 
                                            nertc_sdk_channel_state_e status, 
                                            const char* msg)
{
    NERTC_LOGI("on_channel_status_changed: status=%d, msg=%s", status, msg ? msg : "null");
    dispatch_event(NERTC_PROTOCOL_EVENT_CHANNEL_CHANGED);
}

static void nertc_on_join(const nertc_sdk_callback_context_t* ctx,
                          uint64_t cid,
                          uint64_t uid,
                          nertc_sdk_error_code_e code,
                          uint64_t elapsed,
                          const nertc_sdk_recommended_config_t* recommended_config)
{
    NERTC_LOGI("on_join: cid=%llu, uid=%llu, code=%d, elapsed=%llu",
               (unsigned long long)cid, (unsigned long long)uid, 
               code, (unsigned long long)elapsed);
    
    if (code != NERTC_SDK_ERR_SUCCESS) {
        NERTC_LOGE("Failed to join room, error: %d", code);
        /* 设置错误结果并释放信号量 */
        g_ctx.join_result = (int)code;
        os_sem_post(&g_ctx.join_sem);
        dispatch_error_event((int)code, "Join failed");
        return;
    }
    
    /* 保存房间信息 */
    g_ctx.joined = true;
    g_ctx.cid = cid;
    g_ctx.uid = uid;
    
    /* 保存推荐的音频配置 */
    if (recommended_config) {
        g_ctx.server_sample_rate = recommended_config->recommended_audio_config.sample_rate;
        g_ctx.server_out_sample_rate = recommended_config->recommended_audio_config.out_sample_rate;
        g_ctx.server_frame_duration = recommended_config->recommended_audio_config.frame_duration;
        g_ctx.samples_per_channel = recommended_config->recommended_audio_config.samples_per_channel;
        g_ctx.recommended_audio_config = recommended_config->recommended_audio_config;
        
        NERTC_LOGI("Recommended audio: sample_rate=%d, out_sample_rate=%d, frame_duration=%d",
                   g_ctx.server_sample_rate, g_ctx.server_out_sample_rate, 
                   g_ctx.server_frame_duration);
    }
    
    /* 设置成功结果并释放信号量 */
    g_ctx.join_result = 0;
    os_sem_post(&g_ctx.join_sem);
    
    dispatch_connected_event(cid, uid, recommended_config);
}

static void nertc_on_disconnect(const nertc_sdk_callback_context_t* ctx, 
                                nertc_sdk_error_code_e code, 
                                int reason)
{
    NERTC_LOGI("on_disconnect: code=%d, reason=%d", code, reason);
    
    g_ctx.joined = false;
    g_ctx.audio_channel_opened = false;
    
    if (g_ctx.event_callback) {
        nertc_protocol_event_t evt;
        memset(&evt, 0, sizeof(evt));
        evt.event = NERTC_PROTOCOL_EVENT_DISCONNECTED;
        evt.data.disconnected.error_code = (int)code;
        evt.data.disconnected.reason = reason;
        g_ctx.event_callback(&evt, g_ctx.event_user_data);
    }
}

static void nertc_on_user_joined(const nertc_sdk_callback_context_t* ctx, 
                                 const nertc_sdk_user_info_t* user)
{
    if (!user) return;
    
    NERTC_LOGI("User joined: uid=%llu, name=%s, type=%d",
               (unsigned long long)user->uid, user->name ? user->name : "", user->type);
    
    dispatch_user_event(NERTC_PROTOCOL_EVENT_USER_JOINED, user);
}

static void nertc_on_user_left(const nertc_sdk_callback_context_t* ctx, 
                               const nertc_sdk_user_info_t* user, 
                               int reason)
{
    if (!user) return;
    
    NERTC_LOGI("User left: uid=%llu, reason=%d", (unsigned long long)user->uid, reason);
    dispatch_user_event(NERTC_PROTOCOL_EVENT_USER_LEFT, user);
}

static void nertc_on_user_audio_start(const nertc_sdk_callback_context_t* ctx, 
                                      uint64_t uid, 
                                      nertc_sdk_media_stream_e stream_type)
{
    NERTC_LOGI("User audio start: uid=%llu, stream_type=%d", 
               (unsigned long long)uid, stream_type);
    dispatch_event(NERTC_PROTOCOL_EVENT_USER_AUDIO_START);
}

static void nertc_on_user_audio_mute(const nertc_sdk_callback_context_t* ctx,
                                     uint64_t uid,
                                     nertc_sdk_media_stream_e stream_type,
                                     bool mute)
{
    NERTC_LOGI("User audio mute: uid=%llu, mute=%d", (unsigned long long)uid, mute);
}

static void nertc_on_user_audio_stop(const nertc_sdk_callback_context_t* ctx, 
                                     uint64_t uid, 
                                     nertc_sdk_media_stream_e stream_type)
{
    NERTC_LOGI("User audio stop: uid=%llu, stream_type=%d", 
               (unsigned long long)uid, stream_type);
    dispatch_event(NERTC_PROTOCOL_EVENT_USER_AUDIO_STOP);
}

static void nertc_on_asr_caption_state_changed(const nertc_sdk_callback_context_t* ctx,
                                               nertc_sdk_asr_caption_state_e state,
                                               nertc_sdk_error_code_e code,
                                               const char* msg)
{
    NERTC_LOGI("ASR caption state changed: state=%d, code=%d", state, code);
}

static void nertc_on_asr_caption_result(const nertc_sdk_callback_context_t* ctx,
                                        nertc_sdk_asr_caption_result_t* results,
                                        int result_count)
{
    for (int i = 0; i < result_count; i++) {
        NERTC_LOGI("ASR result[%d]: %s (is_local=%d, is_final=%d)", 
                   i, results[i].content ? results[i].content : "", 
                   results[i].is_local_user, results[i].is_final);
        
        /* 发送ASR事件 */
        if (g_ctx.event_callback) {
            nertc_protocol_event_t evt;
            memset(&evt, 0, sizeof(evt));
            evt.event = NERTC_PROTOCOL_EVENT_ASR_RESULT;
            evt.data.asr.user_id = results[i].user_id;
            evt.data.asr.is_local_user = results[i].is_local_user;
            evt.data.asr.timestamp = results[i].timestamp;
            evt.data.asr.content = results[i].content;
            evt.data.asr.is_final = results[i].is_final;
            g_ctx.event_callback(&evt, g_ctx.event_user_data);
        }
    }
}

static void nertc_on_ai_data(const nertc_sdk_callback_context_t* ctx, 
                             nertc_sdk_ai_data_result_t* ai_data)
{
    if (!ai_data) return;
    
    const char* type_str = ai_data->type;
    size_t type_len = ai_data->type_len;
    const char* data_str = ai_data->data;
    NERTC_LOGI("OnAiData type:%s data:%s", type_str ? type_str : "null", data_str ? data_str : "null");
    
    /* 如果音频通道未打开，忽略AI数据 */
    if (!g_ctx.audio_channel_opened) {
        NERTC_LOGW("AI data received but audio channel not opened");
        return;
    }
    
    /* 解析 event 类型的 AI 数据，派发 TTS 开始/停止事件 */
    if (type_str && type_len >= 5 && strncmp(type_str, "event", 5) == 0 && data_str) {
        /* 简单字符串匹配，避免引入 JSON 解析依赖 */
        if (strstr(data_str, "audio.agent.speech_started")) {
            NERTC_LOGI(">>> TTS started (AI speaking)");
            dispatch_event(NERTC_PROTOCOL_EVENT_TTS_START);
        } else if (strstr(data_str, "audio.agent.speech_stop")) {
            NERTC_LOGI(">>> TTS stopped (AI finished)");
            dispatch_event(NERTC_PROTOCOL_EVENT_TTS_STOP);
        }
    }
    
    /* 发送通用 AI 数据事件（保留原有逻辑） */
    if (g_ctx.event_callback) {
        nertc_protocol_event_t evt;
        memset(&evt, 0, sizeof(evt));
        evt.event = NERTC_PROTOCOL_EVENT_AI_DATA;
        evt.data.ai.type = ai_data->type;
        evt.data.ai.type_len = ai_data->type_len;
        evt.data.ai.data = ai_data->data;
        evt.data.ai.data_len = ai_data->data_len;
        g_ctx.event_callback(&evt, g_ctx.event_user_data);
    }
}

static void nertc_on_audio_encoded_data(const nertc_sdk_callback_context_t* ctx,
                                        uint64_t uid,
                                        nertc_sdk_media_stream_e stream_type,
                                        nertc_sdk_audio_encoded_frame_t* encoded_frame,
                                        bool is_mute_packet)
{
    if (!encoded_frame || !encoded_frame->data || encoded_frame->length <= 0) {
        return;
    }
    
    /* 只有音频通道打开时才处理下行音频数据 */
    if (!g_ctx.audio_channel_opened) {
        return;
    }
    
    dispatch_audio_data_event(uid, encoded_frame, is_mute_packet);
}

/* ============================================================================
 * 对外接口实现
 * ============================================================================ */

void nertc_protocol_config_init(nertc_protocol_config_t *config)
{
    if (!config) return;
    
    memset(config, 0, sizeof(nertc_protocol_config_t));
    
    /* 必要配置：AppKey 和 DeviceID */
    config->app_key = NERTC_DEFAULT_APP_KEY;
    config->device_id = NERTC_DEFAULT_DEVICE_ID;
    config->license = NERTC_DEFAULT_TEST_LICENSE;
    
    /* 音频配置 */
    config->sample_rate = NERTC_DEFAULT_SAMPLE_RATE;
    config->out_sample_rate = NERTC_DEFAULT_OUT_SAMPLE_RATE;
    config->channels = NERTC_DEFAULT_CHANNELS;
    config->frame_duration = NERTC_DEFAULT_FRAME_DURATION;
    
    /* 功能开关 */
    config->force_unsafe_mode = true;
    config->enable_server_aec = false;  /* 默认不使用服务端AEC */
    config->prefer_use_psram = true;    /* 高性能设备使用PSRAM */
    config->enable_asr = true;
    config->enable_mcp_server = true;
    
    /* 自定义配置 JSON */
    // config->custom_config = NERTC_DEFAULT_CUSTOM_CONFIG;
}

/**
 * @brief 简化版初始化 - 仅打印版本信息，不真正初始化SDK
 *        用于第一步测试，后续再实现完整逻辑
 */
int nertc_protocol_init(void)
{
    NERTC_LOGI("===========================================");
    NERTC_LOGI("NERTC Protocol Init (Simple Version)");
    NERTC_LOGI("SDK Version: %s", nertc_get_version());
    NERTC_LOGI("===========================================");
    
    /* 第一步：只打印版本信息，不做任何初始化 */
    /* 后续再根据需求调用 nertc_protocol_init_with_config() */
    
    return 0;
}

/**
 * @brief 完整版初始化 - 真正初始化SDK引擎
 */
int nertc_protocol_init_with_config(const nertc_protocol_config_t *config)
{
    if (g_ctx.initialized) {
        NERTC_LOGW("Already initialized");
        return 0;
    }
    
    if (!config || !config->app_key || !config->device_id) {
        NERTC_LOGE("Invalid config: app_key and device_id required");
        return -1;
    }
    
    NERTC_LOGI("Protocol init start...");
    NERTC_LOGI("SDK Version: %s", nertc_get_version());
    
    /* 清空上下文 */
    memset(&g_ctx, 0, sizeof(g_ctx));
    
    /* 创建 Join 同步信号量 */
    if (os_sem_create(&g_ctx.join_sem, 0) != OS_NO_ERR) {
        NERTC_LOGE("Failed to create join semaphore");
        return -1;
    }
    
    /* 保存配置副本 */
    g_ctx.config = *config;
    g_ctx.asr_enabled = config->enable_asr;
    
    /* 1. 创建 SDK 配置 */
    nertc_sdk_configuration_t sdk_cfg;
    nertc_sdk_configuration_init(&sdk_cfg);
    
    sdk_cfg.app_key = config->app_key;
    sdk_cfg.device_id = config->device_id;
    sdk_cfg.force_unsafe_mode = config->force_unsafe_mode;
    
    /* 音频配置 */
    sdk_cfg.audio_config.sample_rate = config->sample_rate > 0 ? config->sample_rate : NERTC_DEFAULT_SAMPLE_RATE;
    sdk_cfg.audio_config.out_sample_rate = config->out_sample_rate > 0 ? config->out_sample_rate : NERTC_DEFAULT_OUT_SAMPLE_RATE;
    sdk_cfg.audio_config.channels = config->channels > 0 ? config->channels : NERTC_DEFAULT_CHANNELS;
    sdk_cfg.audio_config.frame_duration = config->frame_duration > 0 ? config->frame_duration : NERTC_DEFAULT_FRAME_DURATION;
    sdk_cfg.audio_config.codec_type = NERTC_SDK_AUDIO_CODEC_TYPE_OPUS;

    /* 可选配置 - 走 ESP32S3 逻辑（高性能设备） */
    sdk_cfg.optional_config.device_performance_level = NERTC_SDK_DEVICE_LEVEL_HIGH;
    sdk_cfg.optional_config.prefer_use_psram = config->prefer_use_psram;
    sdk_cfg.optional_config.enable_server_aec = config->enable_server_aec;
    sdk_cfg.optional_config.custom_config = config->custom_config;

    /* 日志配置 */
    sdk_cfg.log_cfg.log_level = NERTC_SDK_LOG_INFO;
    
    /* License配置 */
    sdk_cfg.licence_cfg.license = config->license ? config->license : NERTC_DEFAULT_TEST_LICENSE;
    
    NERTC_LOGI("Creating engine...");
    
    /* 2. 创建引擎实例 */
    g_ctx.engine = nertc_create_engine_with_config(&sdk_cfg);
    if (!g_ctx.engine) {
        NERTC_LOGE("Failed to create NERtc engine");
        return -2;
    }
    
    /* 3. 配置引擎 */
    nertc_sdk_engine_config_t engine_cfg;
    nertc_sdk_engine_config_init(&engine_cfg);
    
    /* 引擎模式: AI模式 */
    engine_cfg.engine_mode = NERTC_SDK_ENGINE_MODE_AI;
    
    /* 功能配置 */
    engine_cfg.feature_config.enable_mcp_server = config->enable_mcp_server;
    
    /* 设置事件回调 */
    engine_cfg.event_handler.on_error = nertc_on_error;
    engine_cfg.event_handler.on_license_expire_warning = nertc_on_license_expire_warning;
    engine_cfg.event_handler.on_channel_status_changed = nertc_on_channel_status_changed;
    engine_cfg.event_handler.on_join = nertc_on_join;
    engine_cfg.event_handler.on_disconnect = nertc_on_disconnect;
    engine_cfg.event_handler.on_user_joined = nertc_on_user_joined;
    engine_cfg.event_handler.on_user_left = nertc_on_user_left;
    engine_cfg.event_handler.on_user_audio_start = nertc_on_user_audio_start;
    engine_cfg.event_handler.on_user_audio_mute = nertc_on_user_audio_mute;
    engine_cfg.event_handler.on_user_audio_stop = nertc_on_user_audio_stop;
    engine_cfg.event_handler.on_asr_caption_state_changed = nertc_on_asr_caption_state_changed;
    engine_cfg.event_handler.on_asr_caption_result = nertc_on_asr_caption_result;
    engine_cfg.event_handler.on_ai_data = nertc_on_ai_data;
    engine_cfg.event_handler.on_audio_encoded_data = nertc_on_audio_encoded_data;
    engine_cfg.event_handler.on_server_time = NULL;
    
    /* 用户数据 */
    engine_cfg.user_data = &g_ctx;
    
    /* 外部网络接口(暂不使用) */
    engine_cfg.ext_net_handle = NULL;
    
    NERTC_LOGI("Initializing engine...");
    
    /* 4. 初始化引擎 */
    int ret = nertc_init_engine(g_ctx.engine, &engine_cfg);
    if (ret != 0) {
        NERTC_LOGE("Failed to initialize NERtc SDK, error: %d", ret);
        nertc_destroy_engine(g_ctx.engine);
        g_ctx.engine = NULL;
        return -3;
    }
    
    g_ctx.initialized = true;
    NERTC_LOGI("Protocol init success, version: %s", nertc_get_version());
    
    return 0;
}

void nertc_protocol_deinit(void)
{
    NERTC_LOGI("Protocol deinit");
    
    if (g_ctx.joined) {
        nertc_protocol_stop();
    }
    
    if (g_ctx.engine) {
        nertc_destroy_engine(g_ctx.engine);
        g_ctx.engine = NULL;
    }
    
    memset(&g_ctx, 0, sizeof(g_ctx));
}

void nertc_protocol_set_event_callback(nertc_protocol_event_callback_t callback, void *user_data)
{
    g_ctx.event_callback = callback;
    g_ctx.event_user_data = user_data;
}

void nertc_protocol_set_audio_callback(nertc_protocol_audio_callback_t callback, void *user_data)
{
    g_ctx.audio_callback = callback;
    g_ctx.audio_user_data = user_data;
}

int nertc_protocol_start(const char *cname, uint64_t uid)
{
    if (!g_ctx.initialized || !g_ctx.engine) {
        NERTC_LOGE("Not initialized");
        return -1;
    }
    
    if (g_ctx.joined) {
        NERTC_LOGW("Already joined");
        return 0;
    }
    
    /* 生成房间名(如果未提供) */
    if (cname) {
        strncpy(g_ctx.cname, cname, sizeof(g_ctx.cname) - 1);
    } else {
        /* 生成随机房间名: 80 + 6位随机数 
         * 使用杰里硬件随机数 JL_RAND 和 timer_get_ms 增加随机性 */
        extern u32 timer_get_ms(void);
        uint32_t hw_rand = JL_RAND->R64L ^ JL_RAND->R64H ^ timer_get_ms();
        uint32_t random_num = 100000 + (hw_rand % 900000);
        snprintf(g_ctx.cname, sizeof(g_ctx.cname), "80%u", random_num);
    }
    
    /* 使用提供的UID或默认值 */
    uint64_t join_uid = (uid > 0) ? uid : NERTC_DEFAULT_UID;
    
    NERTC_LOGI("Joining room: cname=%s, uid=%llu", g_ctx.cname, (unsigned long long)join_uid);
    
    /* 重置 Join 结果 */
    g_ctx.join_result = -1;
    
    /* 非安全模式下token可以为空 */
    int ret = nertc_join(g_ctx.engine, g_ctx.cname, "", join_uid);
    if (ret != 0) {
        NERTC_LOGE("Join failed, error: %d", ret);
        return ret;
    }
    
    /* 同步等待 on_join 回调 */
    NERTC_LOGI("Waiting for join result (timeout: %d ms)...", NERTC_JOIN_TIMEOUT_MS);
    ret = os_sem_pend(&g_ctx.join_sem, NERTC_JOIN_TIMEOUT_MS);
    if (ret != OS_NO_ERR) {
        NERTC_LOGE("Join timeout or error: %d", ret);
        return -2;
    }
    
    /* 检查 Join 结果 */
    if (g_ctx.join_result != 0) {
        NERTC_LOGE("Join failed with code: %d", g_ctx.join_result);
        return g_ctx.join_result;
    }
    
    NERTC_LOGI("Join succeeded, cid=%llu, uid=%llu", 
               (unsigned long long)g_ctx.cid, (unsigned long long)g_ctx.uid);
    return 0;
}

int nertc_protocol_stop(void)
{
    if (!g_ctx.engine) {
        return -1;
    }
    
    NERTC_LOGI("Stopping protocol (leave room)");
    
    /* 先关闭音频通道 */
    if (g_ctx.audio_channel_opened) {
        nertc_protocol_close_audio_channel();
    }
    
    int ret = nertc_leave(g_ctx.engine);
    g_ctx.joined = false;
    
    return ret;
}

int nertc_protocol_open_audio_channel(void)
{
    if (!g_ctx.engine || !g_ctx.joined) {
        NERTC_LOGE("Cannot open audio channel: engine=%p, joined=%d", 
                   g_ctx.engine, g_ctx.joined);
        return -1;
    }
    
    if (g_ctx.audio_channel_opened) {
        NERTC_LOGW("Audio channel already opened");
        return 0;
    }
    
    NERTC_LOGI("Opening audio channel (start AI)...");
    
    /* 启动AI服务 */
    int ret = nertc_start_ai_with_config(g_ctx.engine, NULL);
    if (ret != 0) {
        NERTC_LOGE("Start AI failed, error: %d", ret);
        return ret;
    }
    
    /* 启动ASR(如果启用) */
    if (g_ctx.asr_enabled) {
        nertc_sdk_asr_caption_config_t asr_cfg;
        nertc_sdk_asr_caption_config_init(&asr_cfg);
        ret = nertc_start_asr_caption(g_ctx.engine, &asr_cfg);
        if (ret != 0) {
            NERTC_LOGE("Start ASR caption failed, error: %d", ret);
            /* ASR失败不影响主流程 */
        }
    }
    
    g_ctx.audio_channel_opened = true;
    dispatch_event(NERTC_PROTOCOL_EVENT_AI_READY);
    
    NERTC_LOGI("Audio channel opened");
    return 0;
}

void nertc_protocol_close_audio_channel(void)
{
    if (!g_ctx.engine) {
        return;
    }
    
    if (!g_ctx.audio_channel_opened) {
        return;
    }
    
    NERTC_LOGI("Closing audio channel (stop AI)...");
    
    nertc_stop_ai(g_ctx.engine);
    nertc_stop_asr_caption(g_ctx.engine);
    
    g_ctx.audio_channel_opened = false;
    NERTC_LOGI("Audio channel closed");
}

bool nertc_protocol_is_audio_channel_opened(void)
{
    return g_ctx.joined && g_ctx.audio_channel_opened;
}

int nertc_protocol_push_audio_pcm(const int16_t *data, int length)
{
    if (!g_ctx.engine || !g_ctx.joined) {
        return -1;
    }
    
    nertc_sdk_audio_frame_t frame;
    nertc_sdk_audio_frame_init(&frame);
    
    frame.type = NERTC_SDK_AUDIO_PCM_16;
    frame.data = (void *)data;
    frame.length = length;
    
    return nertc_push_audio_frame(g_ctx.engine, NERTC_SDK_MEDIA_MAIN_AUDIO, &frame);
}

int nertc_protocol_push_audio_encoded(const uint8_t *data, int length, uint32_t timestamp)
{
    if (!g_ctx.engine || !g_ctx.joined) {
        return -1;
    }
    
    /* 只有音频通道打开时才发送数据 */
    if (!g_ctx.audio_channel_opened) {
        return 0;  /* 静默丢弃，返回0表示没有发送 */
    }

    nertc_sdk_audio_encoded_frame_t frame;
    nertc_sdk_audio_encoded_frame_init(&frame);
    
    frame.data = (nertc_sdk_audio_data_t *)data;
    frame.length = length;
    frame.encoded_timestamp = timestamp;
    
    nertc_sdk_audio_config_t audio_cfg;
    audio_cfg.sample_rate = g_ctx.server_sample_rate;
    audio_cfg.channels = 1;
    audio_cfg.samples_per_channel = g_ctx.samples_per_channel;
    
    int ret = nertc_push_audio_encoded_frame(g_ctx.engine, NERTC_SDK_MEDIA_MAIN_AUDIO, 
                                          audio_cfg, 100, &frame);

    if (ret == 0) {
        return length;
    } else {
        NERTC_LOGI("Push audio encoded failed, error: %d", ret);
        return 0;
    }
}

int nertc_protocol_push_aec_reference(const uint8_t *encoded_data, int encoded_len,
                                       const int16_t *pcm_data, int pcm_len,
                                       int64_t timestamp)
{
    if (!g_ctx.engine || !g_ctx.joined) {
        return -1;
    }
    
    nertc_sdk_audio_encoded_frame_t encoded_frame;
    nertc_sdk_audio_encoded_frame_init(&encoded_frame);
    encoded_frame.data = (nertc_sdk_audio_data_t *)encoded_data;
    encoded_frame.length = encoded_len;
    encoded_frame.timestamp_ms = timestamp;
    
    nertc_sdk_audio_frame_t pcm_frame;
    nertc_sdk_audio_frame_init(&pcm_frame);
    pcm_frame.type = NERTC_SDK_AUDIO_PCM_16;
    pcm_frame.data = (void *)pcm_data;
    pcm_frame.length = pcm_len;
    
    return nertc_push_audio_reference_frame(g_ctx.engine, NERTC_SDK_MEDIA_MAIN_AUDIO,
                                            &encoded_frame, &pcm_frame);
}

int nertc_protocol_send_tts(const char *text, int interrupt_mode, bool add_context)
{
    if (!g_ctx.engine || !g_ctx.joined) {
        return -1;
    }
    
    if (!text) {
        return -2;
    }
    
    NERTC_LOGI("Send TTS: %s", text);
    return nertc_ai_external_tts(g_ctx.engine, text, interrupt_mode, add_context);
}

int nertc_protocol_send_llm_text(const char *text, int interrupt_mode)
{
    if (!g_ctx.engine || !g_ctx.joined) {
        return -1;
    }
    
    if (!text) {
        return -2;
    }
    
    NERTC_LOGI("Send LLM text: %s", text);
    return nertc_ai_llm_prompt(g_ctx.engine, text, interrupt_mode);
}

int nertc_protocol_manual_interrupt(void)
{
    if (!g_ctx.engine) {
        return -1;
    }
    
    NERTC_LOGI("Manual interrupt");
    return nertc_ai_manual_interrupt(g_ctx.engine);
}

int nertc_protocol_manual_start_listen(void)
{
    if (!g_ctx.engine) {
        return -1;
    }
    
    NERTC_LOGI("Manual start listen");
    return nertc_ai_manual_start_listen(g_ctx.engine);
}

int nertc_protocol_manual_stop_listen(void)
{
    if (!g_ctx.engine) {
        return -1;
    }
    
    NERTC_LOGI("Manual stop listen");
    return nertc_ai_manual_stop_listen(g_ctx.engine);
}

int nertc_protocol_reply_mcp_tool(const char *payload, int payload_len)
{
    if (!g_ctx.engine) {
        return -1;
    }
    
    if (!payload || payload_len <= 0) {
        return -2;
    }
    
    NERTC_LOGI("Reply MCP tool: len=%d", payload_len);
    
    nertc_sdk_mcp_tool_result_t result;
    nertc_sdk_mcp_tool_result_init(&result);
    result.payload = payload;
    result.payload_len = payload_len;
    
    return nertc_ai_reply_mcp_tool_call(g_ctx.engine, &result);
}

const char* nertc_protocol_get_version(void)
{
    return nertc_get_version();
}

const char* nertc_protocol_get_cname(void)
{
    if (g_ctx.joined && g_ctx.cname[0] != '\0') {
        return g_ctx.cname;
    }
    return NULL;
}

bool nertc_protocol_is_joined(void)
{
    return g_ctx.joined;
}
