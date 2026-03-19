#ifndef _APP_PROCESS_H
#define _APP_PROCESS_H
#include "system/includes.h"

#define SOFT_VERSION 			"V1000014"
#define HARD_VERSION			"H0000004"




#define DEVICE_MAC "f07cc7000003"
#define DEVICE_ID "n6436f32"
#define AUTH_KEY "7982f47bc2e648dcb5745279a9641dec"

#define PRODUCT_KEY "6608e82aa63f4b98a1aa3c078f461dcf"
#define DEVICE_SECRET "67134bf1edd14909bcdd23a54d6d32b3"



typedef struct{
    u8 chat_start:1;             // 聊天开始标志位，置 1 表示聊天已开始
    u8 chat_end:1;               // 聊天结束标志位，置 1 表示聊天已结束
    u8 voice_end:1;              // 语音播放结束标志位，置 1 表示语音播放已结束
    u8 mic_open_en:1;           // 开麦标记
    u8 dac_play_en:1;           // DAC 播放使能标志位，置 1 表示 DAC 播放已开启
    u8 mic_break_en:1;           // 麦克风打断
    u8 room_change:1;           // 房间变更标志位，置 1 表示房间已变更
    u8 cfg_net_rst_en:1;        //开机重新配网,置 1 表示开机需要重新配网
} device_status_t;

extern device_status_t device_status;

#define DEVICE_TYPE_MAX_LEN         64

typedef struct{
    u8 is_reset;           // 是否重置
    u8 uid[33];               // uuid 32 + 1
    u8 reserve[DEVICE_TYPE_MAX_LEN-33];
}user_param_t;

typedef struct{
    char szPK[DEVICE_TYPE_MAX_LEN];         // 厂商数据
    char device_mac[DEVICE_TYPE_MAX_LEN];   // 设备mac
    char szAuthKey[DEVICE_TYPE_MAX_LEN];    // 认证码
    char device_id[DEVICE_TYPE_MAX_LEN];    // 设备id
    char secret[DEVICE_TYPE_MAX_LEN];         // 密钥
    user_param_t user_info;
    
    // char reserve1[DEVICE_TYPE_MAX_LEN];
    // char reserve2[DEVICE_TYPE_MAX_LEN];
    // char reserve3[DEVICE_TYPE_MAX_LEN];
}device_param_t;



extern device_param_t device_param;

typedef struct{
    char address[16];         // ip
    int port;               // 端口
    u32 restart_time;    // 重启时间
    u32 tz_offset;         // 时区偏移
    char client_id[256];         // 客户端id
    char username[256];         // 设备id
    char password[256];         // 设备id
    char subscribeTopic1[256];         // 订阅主题1
    char subscribeTopic2[256];         // 订阅主题2
    char subscribeTopic3[256];         // 订阅主题3
}device_mqtt_config_t;

extern device_mqtt_config_t device_mqtt_config;

typedef struct{
    char bot_id[64];        // 机器人 ID
    char voice_id[64];      // 语音 ID
    char user_id[64];       // 用户 ID
    char conv_id[64];       // 会话 ID
    char access_token[128]; // 访问令牌
    int expires_in;         // 令牌过期时间
    int token_quota;        // 令牌配额
    int platform_type;      // 平台类型
}coze_login_t;

extern coze_login_t coze_login;


int parse_string(const char *str, device_mqtt_config_t *data) ;

void mqtt_recv_parse(char *data, u32 len);


void app_module_deinit(void);

void device_param_save(u8 type);

void app_music_play_cfg_suc(void);

void app_music_play_connect_success(void);

void change_room_handler(void);

void websocket_close_handler(u8 flag);

#define POWER_LED_PORT      IO_PORTC_01   
#endif