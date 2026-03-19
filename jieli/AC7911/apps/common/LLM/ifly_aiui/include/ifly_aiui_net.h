#ifndef _IFLY_AIUI_NET_H_
#define _IFLY_AIUI_NET_H_

#include "oal_type.h"
#include "web_socket/websocket_api.h"

#define IFLY_AIUI_MAX_IN_TEXT_LENGTH (20 * 1024)

#define IFLY_AIUI_DEVICE_CODE_LEN 128

#define IFLY_AIUI_VOICE_ONCE_SEND_POINTS 320

#define IFLY_AIUI_VOICE_ONCE_SEND_BYTES (IFLY_AIUI_VOICE_ONCE_SEND_POINTS * 2)

#define IFLY_AIUI_NET_TASK_NAME "aiui_net_task"

enum IFLY_AIUI_NET_MSG {
    IFLY_AIUI_RECORD_START_MSG = 0x01,
    IFLY_AIUI_RECORD_SEND_MSG = 0x02,
    IFLY_AIUI_RECORD_STOP_MSG = 0x03,
    IFLY_AIUI_PREVIOUS_SONG_MSG = 0x04,
    IFLY_AIUI_NEXT_SONG_MSG = 0x05,
    IFLY_AIUI_ALARM_SYNC = 0x06,
    IFLY_AIUI_ALARM_RING = 0x07,
    IFLY_AIUI_ALARM_OPERATE = 0x08,
    IFLY_AIUI_INFO_SEARCH = 0X09,
    IFLY_AIUI_CONNECT_SERVER_MSG = 0x0a,
    IFLY_AIUI_SESSION_FINISH_MSG = 0x0b,
    IFLY_AIUI_SEARCH_MSG = 0x0c,
    IFLY_AIUI_CITY_MSG = 0x0e,   //位置信息
    IFLY_AIUI_SEND_PING_MSG = 0x0f,
    IFLY_AIUI_QUERY_CITY_WEATHER_MSG = 0x10,
    IFLY_AIUI_NTP_TIME = 0x11,
    IFLY_AIUI_CITY_WEATHER_MSG = 0x12,
    IFLY_AIUI_UPGRADE_FIRMWARE_CHECK = 0x13,
    IFLY_AIUI_MULTI_SESSION_MSG = 0x14,
    IFLY_AIUI_QUIT_MSG = 0xFF,
};

typedef enum {
    TEXT_REQUEST,       //文本请求
    AUDIO_REQUEST,      //语音请求
    SPEECH_RECOGNITION, //语音识别
    INTENT_REQUEST,     //意图请求
    SKILL_SETTING,      //技能配置
    SYSTEM_SETTING,     //系统配置
    CUSTOM_REQUEST,     //自定义信息查询
} MUTUAL_TYPE;          //交互类型

typedef enum {
    ALARM_SYNC,    //同步提醒
    ALARM_RING,    //播放响铃
    ALARM_OPERATE, //闹钟操作
} MUTUAL_INTENT;

typedef struct {
    //文本请求
    char *text;
    //意图请求
    char *intent;
    char *slots;
    char *extra;
} MUTUAL_PARAM;

typedef struct {
    struct websocket_struct websockets_info;
    u8 connect_status;
    OS_MUTEX websockets_op_mutex;
    unsigned int v_heat_beat_escape;
} t_MUTUAL_HDL;

typedef struct {
    char task[20];
    char sessionId[40]; //会话ID
    char deviceSecret[40];
} MUTUAL_REPLY;

struct ifly_aiui_para {
    MUTUAL_TYPE type;
    MUTUAL_INTENT intent;
    MUTUAL_PARAM param;
    MUTUAL_REPLY reply;
    t_MUTUAL_HDL hdl;
    t_MUTUAL_HDL hdl_tts;
    t_MUTUAL_HDL hdl_text;
};

typedef t_MUTUAL_HDL t_ifly_websockt_handle;
typedef void (*t_p_fun_ws_rcv_cb)(u8 *buf, u32 len, u8 type);
/**
 * @brief ifly_aiui_process_init函数用于初始化内部线程和一些必要操作，需要在联网后调用。
 * @param t_p_fun_ws_rcv_cb *p_cb 注册websocket 回调函数，用于接收处理AIUI下发消息。
 * @return 返回线程创建结果。
 */
int ifly_aiui_process_init(t_p_fun_ws_rcv_cb *p_cb);
/**
 * @brief 内部线程销毁。
 * @param NULL
 * @return void。
 */
void ifly_aiui_process_uninit(void);
/**
 * @brief 获取aiui是否已连接。
 * @param NULL
 * @return 已连接：1 未连接：0。
 */
u8 ifly_aiui_app_get_connect_status(void);
/**
 * @brief 合成TTS。
 * @param *pp_buf 合成文本的内存指针
 * @return void。
 */
void ifly_aiui_tts(unsigned char *pp_buf);
/**
 * @brief 文本交互。
 * @param *pp_buf 交互文本的内存指针
 * @return void。
 */
void ifly_aiui_nlp(unsigned char *pp_buf);
/**
 * @brief 发起语音交互请求。
 * @param void。
 * @return void。
 */
int ifly_aiui_start_voice_req(void);
/**
 * @brief 开始发送识别音频。
 * @param void。
 * @return void。
 */
OAL_RESULT ifly_aiui_send_voice_start(char *data, int data_len);
/**
 * @brief 发送识别音频数据。
 * @param *pp_voice_buf 音频数据的内存指针
 * @param vp_len 音频数据的长度
 * @return OAL_RESULT。
 */
OAL_RESULT ifly_aiui_send_voice_data(char *data, int data_len);
/**
 * @brief 发送识别结束/取消指令。
 * @param vp_end_type 预留
 * @return OAL_RESULT。
 */
OAL_RESULT ifly_aiui_send_voice_stop(char *data, int data_len);
/**
 * @brief 关闭aiui websocket连接。
 * @param void。
 * @return void。
 */
void ifly_aiui_stop_voice_req();
/**
 * @brief 获取AIUI SDK版本。
 * @param NULL。
 * @return char*。
 */
char *ifly_aiui_sdk_version(void);

#endif
