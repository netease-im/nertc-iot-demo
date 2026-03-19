#ifndef __AI_COZE_API_H__
#define __AI_COZE_API_H__

#include "generic/typedef.h"
/*
扣子（英文名称 Coze） 是新一代一站式 AI Bot 开发平台。Coze 是由字节跳动推出的一个AI聊天机器人和应用程序编辑开发平台，
可以理解为字节跳动版的GPTs。无论你是否有编程基础，都可以在扣子平台上快速搭建基于 AI 模型的各类问答 Bot，
这个平台都可以让你快速地创建各种类型的聊天机器人，并将它们部署在其他社交平台和消息应用上。
Coze还提供了多种插件、知识、工作流、长期记忆和定时任务等功能，来增强聊天机器人的能力和交互性。
而且你可以将搭建的 Bot 发布到各类社交平台和通讯软件上，让更多的用户与你搭建的 Bot 聊天。

https://juejin.cn/post/7444510574194999333

 */
enum {
    AI_SERVER_EVENT_URL,
    AI_SERVER_EVENT_URL_TTS,
    AI_SERVER_EVENT_URL_MEDIA,
    AI_SERVER_EVENT_CONNECTED,
    AI_SERVER_EVENT_DISCONNECTED,
    AI_SERVER_EVENT_CONTINUE,
    AI_SERVER_EVENT_PAUSE,
    AI_SERVER_EVENT_STOP,
    AI_SERVER_EVENT_UPGRADE,
    AI_SERVER_EVENT_UPGRADE_SUCC,
    AI_SERVER_EVENT_UPGRADE_FAIL,
    AI_SERVER_EVENT_MIC_OPEN,
    AI_SERVER_EVENT_MIC_CLOSE,
    AI_SERVER_EVENT_REC_TOO_SHORT,
    AI_SERVER_EVENT_RECV_CHAT,
    AI_SERVER_EVENT_VOLUME_CHANGE,
    AI_SERVER_EVENT_SET_PLAY_TIME,
    AI_SERVER_EVENT_SEEK,
    AI_SERVER_EVENT_PLAY_BEEP,
    AI_SERVER_EVENT_CHILD_LOCK_CHANGE,
    AI_SERVER_EVENT_LIGHT_CHANGE,
    AI_SERVER_EVENT_REC_ERR,
    AI_SERVER_EVENT_BT_CLOSE,
    AI_SERVER_EVENT_BT_OPEN,
    AI_SERVER_EVENT_RESUME_PLAY,
    AI_SERVER_EVENT_PREV_PLAY,
    AI_SERVER_EVENT_NEXT_PLAY,
    AI_SERVER_EVENT_SHUTDOWN,
    /* NERTC 相关事件 */
    AI_SERVER_EVENT_AI_READY,       /* AI 服务就绪 */
    AI_SERVER_EVENT_ASR_RESULT,     /* ASR 识别结果 */
    AI_SERVER_EVENT_TTS_START,      /* TTS 开始 */
    AI_SERVER_EVENT_TTS_STOP,       /* TTS 停止 */
};




/**
 * @brief 定义 AI Coze 服务器事件的枚举类型。
 * 
 * 该枚举列出了 AI Coze 服务器可能触发的各种事件，每个事件都有一个唯一的整数值标识。
 */
enum ai_coze_server_event {
    AI_EVENT_SPEAK_END     = 0x01,  // 语音播放结束
    AI_EVENT_MEDIA_END     = 0x02,  // 媒体播放结束
    AI_EVENT_PLAY_PAUSE    = 0x03,  // 播放暂停
    AI_EVENT_PREVIOUS_SONG = 0x04,  // 上一首
    AI_EVENT_NEXT_SONG     = 0x05,  // 下一首
    AI_EVENT_VOLUME_CHANGE = 0X06,  // 音量改变
    AI_EVENT_VOLUME_INCR   = 0x07,  // 音量增加
    AI_EVENT_VOLUME_DECR   = 0x08,  // 音量减小
    AI_EVENT_VOLUME_MUTE   = 0x09,  // 音量静音
    AI_EVENT_RECORD_START  = 0x0a,  // 录音开始
    AI_EVENT_RECORD_BREAK  = 0x0b,  // 录音打断
    AI_EVENT_RECORD_STOP   = 0x0c,  // 录音停止
    AI_EVENT_VOICE_MODE    = 0x0d,  // 语音模式
    AI_EVENT_PLAY_TIME     = 0x0e,  // 播放时间
    AI_EVENT_MEDIA_STOP    = 0x0f,  // 媒体停止
    AI_EVENT_COLLECT_RES   = 0x10,  // 收集结果事件
    AI_EVENT_CHILD_LOCK    = 0x11,   // 儿童锁状态变化事件
    AI_EVENT_WEBS_CLOSE    = 0x12,   // WebSocket 连接关闭事件
    AI_EVENT_WEBS_CONNECTED = 0x13,  // WebSocket 连接成功事件
    AI_EVENT_MQTT_REG_SUCC = 0x14,     // MQTT 注册成功事件
    AI_EVENT_MQTT_REG_FAIL = 0x15,      // MQTT 注册失败事件
    AI_EVENT_MQTT_GET_COZE_INFO_SUCC = 0x16,    // MQTT 获取 Coze 信息成功事件
    AI_EVENT_MQTT_PUBLISH_TOPIC = 0x17,        // MQTT 发布主题事件
    AI_EVENT_MQTT_CHANGE_ROOM = 0x18,           // MQTT 房间变更事件
    AI_EVENT_MQTT_GET_COZE_INFO_FAIL = 0x19,    // MQTT 获取 Coze 信息失败事件
    AI_EVENT_MQTT_DISCONNECTED = 0x1a,    // MQTT 获取 Coze 信息失败事件
    AI_EVENT_COZE_WEBSOCKET_BUILD_SUCC = 0x1b,  // Coze WebSocket 连接建立成功事件
    AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL = 0x1c,  // Coze WebSocket 连接建立成功事件

    AI_EVENT_RECORD_RESET_CLOSE = 0x1a,        // 录音重置关闭事件
    AI_EVENT_RECORD_RESET_OPEN = 0x1b,         // 录音重置打开事件


    AI_EVENT_CUSTOM_FUN    = 0x30,               // 自定义功能事件
    AI_EVENT_RUN_START     = 0x80,              // 运行开始事件
    AI_EVENT_RUN_STOP      = 0x81,              // 运行停止事件
    AI_EVENT_SPEAK_START   = 0x82,              // 语音播报开始事件
    AI_EVENT_MEDIA_START   = 0x83,              // 媒体播放开始事件
    AI_EVENT_MEDIA_PLAY_TIME   = 0x84,          // 媒体播放时间更新事件

    AI_EVENT_RTC_ONOFF=0x85,

    AI_EVENT_QUIT= 0xff,
};

enum {
    AI_REQ_CONNECT,
    AI_REQ_DISCONNECT,
    AI_REQ_EVENT,
};

struct ai_coze_event {
    int arg;
    int event;
    const char *ai_name;
};
union ai_coze_req {
    struct ai_coze_event evt;
};


struct ai_coze_sdk_api {
    void (*handler)(void *, int argc, int *argv);
    u8 ai_connected;
    u8 ai_chat_created;
};

extern struct ai_coze_sdk_api ai_coze;

/* ============================================================================
 * AI 状态机定义
 * ============================================================================ */
typedef enum {
    AI_STATE_STARTING = 0,   /* 启动中：从硬件启动到 nertc_protocol_start 完成之前 */
    AI_STATE_IDLE,           /* 空闲：已连接但未开启 AI 对话 */
    AI_STATE_LISTENING,      /* 监听中：AI 未说话，用户可以说话 */
    AI_STATE_SPEAKING,       /* AI说话中：AI 正在输出语音 */
} ai_state_t;

/**
 * @brief 获取当前 AI 状态
 * @return 当前状态
 */
ai_state_t ai_get_state(void);

/**
 * @brief 设置 AI 状态
 * @param state 新状态
 */
void ai_set_state(ai_state_t state);

/**
 * @brief 获取状态名称字符串（调试用）
 * @param state 状态值
 * @return 状态名称
 */
const char* ai_state_to_string(ai_state_t state);

void ai_send_event_to_sdk(int event, int arg);
void ai_server_event_callbck(int event, int arg);
void *ai_coze_server_open(void *name, void *priv);
void ai_coze_server_close(void *name);
void ai_coze_server_register_event_handler(struct ai_coze_sdk_api *server, void *priv,
                                   void (*handler)(void *, int argc, int *argv));

#define AI_COZE_SDK_NAME   "ai_coze"

/* ============================================================================
 * NERTC 扩展接口 (当使用 ai_nertc_server.c 替换 ai_coze_server.c 时可用)
 * ============================================================================ */
#ifdef USE_NERTC_AI_SERVER

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 推送 PCM 音频数据到 NERTC
 * @param data PCM 数据
 * @param length 数据长度 (int16_t 个数)
 * @return 0成功，非0失败
 */
int ai_nertc_push_audio_pcm(const int16_t *data, int length);

/**
 * @brief 推送编码后的音频数据到 NERTC
 * @param data 编码数据
 * @param length 数据长度
 * @param timestamp 时间戳
 * @return 0成功，非0失败
 */
int ai_nertc_push_audio_encoded(const uint8_t *data, int length, uint32_t timestamp);

/**
 * @brief 发送 TTS 文本
 * @param text 文本内容
 * @return 0成功，非0失败
 */
int ai_nertc_send_tts(const char *text);

/**
 * @brief 发送 LLM 文本
 * @param text 文本内容
 * @return 0成功，非0失败
 */
int ai_nertc_send_llm_text(const char *text);

/**
 * @brief 手动打断 AI
 * @return 0成功，非0失败
 */
int ai_nertc_manual_interrupt(void);

/**
 * @brief 手动开始监听
 * @return 0成功，非0失败
 */
int ai_nertc_manual_start_listen(void);

/**
 * @brief 手动停止监听
 * @return 0成功，非0失败
 */
int ai_nertc_manual_stop_listen(void);

/**
 * @brief 检查是否已连接
 * @return true/false
 */
bool ai_nertc_is_connected(void);

/**
 * @brief 检查音频通道是否已打开
 * @return true/false
 */
bool ai_nertc_is_audio_channel_opened(void);

#endif /* USE_NERTC_AI_SERVER */

#endif /* __AI_COZE_API_H__ */
