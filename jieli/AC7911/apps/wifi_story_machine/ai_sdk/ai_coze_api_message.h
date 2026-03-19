//上行事件
// Event type sent by the client
//更新会话配置
#define COZE_WS_EVENT_CHAT_UPDATE                       "chat.update"
//流式上传音频片段
#define COZE_WS_EVENT_INPUT_AUDIO_BUFFER_APPEND         "input_audio_buffer.append"
//提交音频片段
#define COZE_WS_EVENT_INPUT_AUDIO_BUFFER_COMPLETE       "input_audio_buffer.complete"
//清除缓冲区音频
#define COZE_WS_EVENT_INPUT_AUDIO_BUFFER_CLEAR          "input_audio_buffer.clear"
//手动提交对话内容
#define COZE_WS_EVENT_CONVERSATION_MESSAGE_CREATE       "conversation.message.create"
//清除上下文
#define COZE_WS_EVENT_CONVERSATION_CLEAR                "conversation.clear"
//提交端插执行结果
#define COZE_WS_EVENT_CONVERSATION_CHAT_SUBMIT_TOOL_OUTPUTS "conversation.chat.submit_tool_outputs"

// Event type received by the client
//会话连接成功
#define COZE_WS_EVENT_CHAT_CREATED                      "chat.created"
//会话配置成功
#define COZE_WS_EVENT_CHAT_UPDATED                      "chat.updated"
//对话开始
#define COZE_WS_EVENT_CONVERSATION_CREATED              "conversation.chat.created"
//对话正在处理
#define COZE_WS_EVENT_CONVERSATION_IN_PROGRESS          "conversation.chat.in_progress"
//增量消息
#define COZE_WS_EVENT_CONVERSATION_MESSAGE_DELTA        "conversation.message.delta"
//增量语音
#define COZE_WS_EVENT_CONVERSATION_AUDIO_DELTA          "conversation.audio.delta"
//消息完成
#define COZE_WS_EVENT_CONVERSATION_MESSAGE_COMPLETED    "conversation.message.completed"
//语音回复完成
#define COZE_WS_EVENT_CONVERSATION_AUDIO_COMPLETED      "conversation.audio.completed"
//对话完成
#define COZE_WS_EVENT_CONVERSATION_CHAT_COMPLETED       "conversation.chat.completed"
//对话失败
#define COZE_WS_EVENT_CONVERSATION_CHAT_FAILED          "conversation.chat.failed"
//发生错误
#define COZE_WS_EVENT_ERROR                             "error"
//input_audio_buffer提交成功
#define COZE_WS_EVENT_INPUT_AUDIO_BUFFER_COMPLETED      "input_audio_buffer.completed"
//input_audio_buffer清除成功
#define COZE_WS_EVENT_INPUT_AUDIO_BUFFER_CLEARED        "input_audio_buffer.cleared"
//上下文清除完成
#define COZE_WS_EVENT_CONVERSATION_CLEARED              "conversation.cleared"
//智能体输出中断
#define COZE_WS_EVENT_AUDIO_TRANSCRIPT_UPDATE           "conversation.audio_transcript.update"
//用户语音识别智库
#define COZE_WS_EVENT_AUDIO_TRANSCRIPT_COMPLETED        "conversation.audio_transcript.completed"
// RTC
//流式语音合成
#define COZE_WS_EVENT_INPUT_AUDIO_SPEECH_STARTED        "input_audio_buffer.speech_started"
#define COZE_WS_EVENT_INPUT_AUDIO_SPEECH_STOPPED        "input_audio_buffer.speech_stopped"


// 定义 WS 协议状态
//聊天状态定义与管理
typedef enum {
    WS_CHAT_STATE_INIT,
    WS_CHAT_STATE_CREATED,
    WS_CHAT_STATE_UPDATE_SENT,
    WS_CHAT_STATE_UPDATED,
    WS_CHAT_STATE_CONVERSATION_MESSAGE_COMPLETED,
    WS_CHAT_STATE_MESSAGE_DELTA,
    WS_CHAT_STATE_AUDIO_DELTA,
    WS_CHAT_STATE_AUDIO_COMPLETED,
    WS_CHAT_STATE_CHAT_COMPLETED,
    WS_CHAT_STATE_INPUT_AUDIO_COMPLETED,
    WS_CHAT_STATE_AUDIO_TRANSCRIPT_UPDATE,
    WS_CHAT_STATE_AUDIO_TRANSCRIPT_COMPLETED,
    WS_CHAT_STATE_AUDIO_SPEECH_STARTED,
    WS_CHAT_STATE_AUDIO_SPEECH_STOPED,
    WS_CHAT_STATE_FAILED,
    WS_CHAT_STATE_ERROR,
    WS_CHAT_STATE_MAX,
} ws_chat_state_t;

extern ws_chat_state_t ws_chat_state;

/* ============================================================================
 * 函数声明
 * ============================================================================ */

/**
 * @brief 发送音频到 Coze WebSocket 服务器
 * @param voice_data 音频数据
 * @param len 数据长度
 * @return 0:成功, <0:失败
 */
int send_voice_data_to_server(u8 *voice_data, int len);

/* 发送音频数据到 NERTC 服务器 */
int send_voice_data_to_nertc_server(u8 *voice_data, int len);

/* 注册 NERTC 下行音频回调 */
void nertc_register_audio_callback(void);

/**
 * @brief 发送聊天更新请求
 */
void send_chat_update_request(void);

/**
 * @brief 发送 Hello 消息
 */
void send_hello_message(void);

/**
 * @brief Coze WebSocket 接收处理
 */
void coze_ws_recv_handler(u8 *buf, u32 len);

/**
 * @brief 结束录音
 */
void end_recorder(void);

/**
 * @brief 取消 AI 智能体任务
 */
void cancel_ai_agent_task(void);

/**
 * @brief 设置聊天状态
 */
void ws_set_chat_state(ws_chat_state_t state);