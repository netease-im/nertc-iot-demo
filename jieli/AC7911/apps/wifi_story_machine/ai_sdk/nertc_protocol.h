/**
 * @file nertc_protocol.h
 * @brief NERtc Protocol Wrapper for Jieli Platform
 * 
 * 基于 nertc_protocol_test.h/.cc 参考实现
 * 为杰理平台（C语言）封装 NERTC SDK
 */

#ifndef _NERTC_PROTOCOL_JIELI_H_
#define _NERTC_PROTOCOL_JIELI_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 默认配置宏定义
 * ============================================================================ */
#define NERTC_DEFAULT_UID                   6669
#define NERTC_DEFAULT_APP_KEY               "xxxxxxxxxx" // 替换成您的appkey
#define NERTC_DEFAULT_DEVICE_ID             "jieli-test" // 设备id, 建议换成mac地址
#define NERTC_DEFAULT_SAMPLE_RATE           16000
#define NERTC_DEFAULT_OUT_SAMPLE_RATE       16000
#define NERTC_DEFAULT_CHANNELS              1
#define NERTC_DEFAULT_FRAME_DURATION        60      /* 60ms */

/* 默认测试 License */
#define NERTC_DEFAULT_TEST_LICENSE "eyJhbGdvcml0aG0iOiJkZWZhdWx0IiwiY3JlZGVudGlhbCI6eyJhY3RpdmF0ZURhdGUiOjE3NTkxMjAyMTQsImV4cGlyZURhdGUiOjE3OTMyNDgyMTQsImxpY2Vuc2VLZXkiOiJ5dW54aW5Db21tb25UZXN0Iiwibm9uY2UiOiJQLW85S3o5RTI2WSJ9LCJzaWduYXR1cmUiOiJFcUt4c2s5TTFNWGp2dWE5Z3J4MGVYVkxxdHBrMm5aWUoyMHNIM0x4LzVMT0tFL3BOTktadDdmNG04eVE5ZWFIQmxHS2NaYUdmaVlNeTdtZ1pYTjVLVFRvZzk2K2RYZmhuZ1N4UG9YUDZBUkNHdHlmZ1N1SDFtbGlxYUYyNWVrWVlRQzUwV3V5ZHFMRzJyaDB1cVhrR05oSkhtVWtRdnVndU1sVlZjc2JLelRCNGRubmtzVVhOcm12aEZXQS9IUTNqd0kyYmFaOTNlMzM4UkFjYTRlVHBTeUhkb1EzRlNGVkpxUXBWRHlJVXhOb21ORXhJT1Z2bzN5dkdRcERLTldjVUFFejFQN1R0Z3ozS1U4RnZYT09sRXNPTU5UYzRxdU9JSWp1SWNRaE1aK2FrSHlDRURHSE04TUthYkdTU2JZOXN1NWVwdjZEZUZxWjEza29wK1pHK2c9PSIsInZlcnNpb24iOiIxLjAifQ=="
/* ============================================================================
 * 协议事件定义
 * ============================================================================ */
typedef enum {
    NERTC_PROTOCOL_EVENT_NONE = 0,
    NERTC_PROTOCOL_EVENT_ERROR,              /* SDK错误 */
    NERTC_PROTOCOL_EVENT_CONNECTED,          /* 加入房间成功 */
    NERTC_PROTOCOL_EVENT_DISCONNECTED,       /* 与服务器断开 */
    NERTC_PROTOCOL_EVENT_CHANNEL_CHANGED,    /* 房间状态变化 */
    NERTC_PROTOCOL_EVENT_USER_JOINED,        /* 远端用户加入 */
    NERTC_PROTOCOL_EVENT_USER_LEFT,          /* 远端用户离开 */
    NERTC_PROTOCOL_EVENT_USER_AUDIO_START,   /* 远端音频开始 */
    NERTC_PROTOCOL_EVENT_USER_AUDIO_STOP,    /* 远端音频停止 */
    NERTC_PROTOCOL_EVENT_AI_READY,           /* AI服务就绪 */
    NERTC_PROTOCOL_EVENT_AI_DATA,            /* AI数据到达 */
    NERTC_PROTOCOL_EVENT_ASR_RESULT,         /* ASR识别结果 */
    NERTC_PROTOCOL_EVENT_AUDIO_DATA,         /* 音频数据到达 */
    NERTC_PROTOCOL_EVENT_TTS_START,          /* TTS开始 */
    NERTC_PROTOCOL_EVENT_TTS_STOP,           /* TTS停止 */
    NERTC_PROTOCOL_EVENT_LICENSE_WARNING,    /* License过期警告 */
} nertc_protocol_event_e;

/* ============================================================================
 * 事件数据结构
 * ============================================================================ */

/* 错误事件数据 */
typedef struct {
    int error_code;
    const char *error_msg;
} nertc_protocol_error_data_t;

/* 连接成功事件数据 */
typedef struct {
    uint64_t cid;       /* 房间ID */
    uint64_t uid;       /* 用户ID */
    int sample_rate;
    int out_sample_rate;
    int frame_duration;
    int samples_per_channel;
} nertc_protocol_connected_data_t;

/* 断开连接事件数据 */
typedef struct {
    int error_code;
    int reason;
} nertc_protocol_disconnected_data_t;

/* 用户事件数据 */
typedef struct {
    uint64_t uid;
    const char *name;
    int user_type;      /* nertc_sdk_user_type_e */
} nertc_protocol_user_data_t;

/* ASR结果数据 */
typedef struct {
    uint64_t user_id;
    bool is_local_user;
    uint64_t timestamp;
    const char *content;
    bool is_final;
} nertc_protocol_asr_data_t;

/* AI数据 */
typedef struct {
    const char *type;
    int type_len;
    const char *data;
    int data_len;
} nertc_protocol_ai_data_t;

/* 音频数据 */
typedef struct {
    uint64_t uid;
    uint8_t *data;
    int length;
    int64_t timestamp_ms;
    uint32_t encoded_timestamp;
    int sample_rate;
    int frame_duration;
    bool is_mute_packet;
} nertc_protocol_audio_data_t;

/* 统一事件结构体 */
typedef struct {
    nertc_protocol_event_e event;
    union {
        nertc_protocol_error_data_t error;
        nertc_protocol_connected_data_t connected;
        nertc_protocol_disconnected_data_t disconnected;
        nertc_protocol_user_data_t user;
        nertc_protocol_asr_data_t asr;
        nertc_protocol_ai_data_t ai;
        nertc_protocol_audio_data_t audio;
        int license_remaining_days;
    } data;
} nertc_protocol_event_t;

/* ============================================================================
 * 配置结构体
 * ============================================================================ */
typedef struct {
    const char *app_key;            /* 应用AppKey */
    const char *device_id;          /* 设备ID */
    const char *license;            /* License，可选，为NULL使用默认 */
    const char *custom_config;      /* 自定义配置JSON，可选 */
    
    /* 音频配置 */
    int sample_rate;                /* 采样率，默认16000 */
    int out_sample_rate;            /* 输出采样率，默认16000 */
    int channels;                   /* 声道数，默认1 */
    int frame_duration;             /* 帧时长(ms)，默认60 */
    
    /* 可选功能配置 */
    bool force_unsafe_mode;         /* 非安全模式，默认true */
    bool enable_server_aec;         /* 服务端AEC，默认false */
    bool prefer_use_psram;          /* 优先使用PSRAM，默认true(高性能设备) */
    bool enable_asr;                /* 开启ASR，默认true */
    bool enable_mcp_server;         /* 开启MCP服务，默认true */
    
    /* 用户数据 */
    void *user_data;
} nertc_protocol_config_t;

/* ============================================================================
 * 回调函数类型
 * ============================================================================ */

/**
 * @brief 事件回调函数
 * @param event 事件数据
 * @param user_data 用户数据
 */
typedef void (*nertc_protocol_event_callback_t)(const nertc_protocol_event_t *event, void *user_data);

/**
 * @brief 音频数据回调函数（独立，用于高频音频数据）
 * @param audio 音频数据
 * @param user_data 用户数据
 */
typedef void (*nertc_protocol_audio_callback_t)(const nertc_protocol_audio_data_t *audio, void *user_data);

/* ============================================================================
 * API 函数声明
 * ============================================================================ */

/**
 * @brief 获取默认配置
 * @param config 配置结构体指针
 */
void nertc_protocol_config_init(nertc_protocol_config_t *config);

/**
 * @brief 初始化协议层（简化版，使用默认配置）
 * @return 0成功，非0失败
 */
int nertc_protocol_init(void);

/**
 * @brief 初始化协议层（完整版，使用自定义配置）
 * @param config 配置参数
 * @return 0成功，非0失败
 */
int nertc_protocol_init_with_config(const nertc_protocol_config_t *config);

/**
 * @brief 销毁协议层
 */
void nertc_protocol_deinit(void);

/**
 * @brief 设置事件回调
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void nertc_protocol_set_event_callback(nertc_protocol_event_callback_t callback, void *user_data);

/**
 * @brief 设置音频回调（可选，用于独立处理高频音频数据）
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void nertc_protocol_set_audio_callback(nertc_protocol_audio_callback_t callback, void *user_data);

/**
 * @brief 启动协议（加入房间）
 * @param cname 房间名，为NULL则自动生成
 * @param uid 用户ID，为0则使用默认
 * @return 0成功，非0失败
 */
int nertc_protocol_start(const char *cname, uint64_t uid);

/**
 * @brief 停止协议（离开房间）
 * @return 0成功，非0失败
 */
int nertc_protocol_stop(void);

/**
 * @brief 打开音频通道（启动AI）
 * @return 0成功，非0失败
 */
int nertc_protocol_open_audio_channel(void);

/**
 * @brief 关闭音频通道（停止AI）
 */
void nertc_protocol_close_audio_channel(void);

/**
 * @brief 音频通道是否已打开
 * @return true/false
 */
bool nertc_protocol_is_audio_channel_opened(void);

/**
 * @brief 推送PCM音频帧
 * @param data PCM数据 (int16_t)
 * @param length 数据长度(int16_t个数)
 * @return 0成功，非0失败
 */
int nertc_protocol_push_audio_pcm(const int16_t *data, int length);

/**
 * @brief 推送编码后的音频帧（OPUS）
 * @param data 编码数据
 * @param length 数据长度
 * @param timestamp 编码时间戳
 * @return 0成功，非0失败
 */
int nertc_protocol_push_audio_encoded(const uint8_t *data, int length, uint32_t timestamp);

/**
 * @brief 推送AEC参考音频
 * @param encoded_data 编码数据
 * @param encoded_len 编码数据长度
 * @param pcm_data PCM数据
 * @param pcm_len PCM数据长度
 * @param timestamp 时间戳
 * @return 0成功，非0失败
 */
int nertc_protocol_push_aec_reference(const uint8_t *encoded_data, int encoded_len,
                                       const int16_t *pcm_data, int pcm_len,
                                       int64_t timestamp);

/**
 * @brief 发送TTS文本
 * @param text 文本内容
 * @param interrupt_mode 打断模式 (1:高优先级 2:中优先级 3:低优先级)
 * @param add_context 是否添加到上下文
 * @return 0成功，非0失败
 */
int nertc_protocol_send_tts(const char *text, int interrupt_mode, bool add_context);

/**
 * @brief 发送LLM文本
 * @param text 文本内容
 * @param interrupt_mode 打断模式
 * @return 0成功，非0失败
 */
int nertc_protocol_send_llm_text(const char *text, int interrupt_mode);

/**
 * @brief 手动打断AI
 * @return 0成功，非0失败
 */
int nertc_protocol_manual_interrupt(void);

/**
 * @brief 手动开始监听（PTT模式）
 * @return 0成功，非0失败
 */
int nertc_protocol_manual_start_listen(void);

/**
 * @brief 手动停止监听（PTT模式）
 * @return 0成功，非0失败
 */
int nertc_protocol_manual_stop_listen(void);

/**
 * @brief 回复MCP工具调用结果
 * @param payload JSON-RPC格式的结果
 * @param payload_len 结果长度
 * @return 0成功，非0失败
 */
int nertc_protocol_reply_mcp_tool(const char *payload, int payload_len);

/**
 * @brief 获取SDK版本
 * @return 版本字符串
 */
const char* nertc_protocol_get_version(void);

/**
 * @brief 获取当前房间名
 * @return 房间名，未加入返回NULL
 */
const char* nertc_protocol_get_cname(void);

/**
 * @brief 是否已加入房间
 * @return true/false
 */
bool nertc_protocol_is_joined(void);

#ifdef __cplusplus
}
#endif

#endif /* _NERTC_PROTOCOL_JIELI_H_ */
