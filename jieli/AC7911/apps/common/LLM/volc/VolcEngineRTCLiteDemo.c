#include "VolcEngineRTCLite.h"
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "volc_type.h"
#include "fs/fs.h"
#include "RtcBotUtils.h"
#include "cJSON.h"
#include "Config.h"
#include "g711.h"
#include "audio_input.h"
#include "app_config.h"

static bool joined = false;

static bool received = 0;

#define DEFAULT_FPS_VALUE                   25
#define NUMBER_OF_OPUS_FRAME_FILES          618
#define SAMPLE_AUDIO_FRAME_DURATION         20


#define MAX_APPID_LEN       64
#define MAX_ROOMID_LEN      64
#define MAX_USERID_LEN      64
#define MAX_video_file_LEN   256
#define MAX_audio_file_LEN   256
#define MAX_TOKEN_LEN       256
#define MAX_PATH_LEN        1024

typedef struct {
    char app_id[MAX_APPID_LEN];
    char room_id[MAX_ROOMID_LEN];
    char user_id[MAX_USERID_LEN];
    char video_file[MAX_video_file_LEN];
    char audio_file[MAX_audio_file_LEN];
    char token[MAX_TOKEN_LEN];

    bool channel_joined;
    bool is_send_video;
    bool is_send_audio;
    pthread_t  send_video_thread_id;
    pthread_t  send_audio_thread_id;
    int audio_codec;
    int fps;
    byte_rtc_engine_t engine;
    bool fini_notifyed;
} byte_rtc_data;


byte_rtc_data g_byte_rtc_data;


static void byte_rtc_on_join_channel_success(byte_rtc_engine_t engine, const char *channel, int elapsed_ms, bool rejoin)
{
    g_byte_rtc_data.channel_joined = true;
    printf("\n join channel success %s elapsed %d ms \n", channel, elapsed_ms);
    joined = true;
};

static void byte_rtc_on_user_joined(byte_rtc_engine_t engine, const char *channel, const char *user_name, int elapsed_ms)
{
    printf("\n remote user joined  %s:%s \n", channel, user_name);
};

static void byte_rtc_on_user_offline(byte_rtc_engine_t engine, const char *channel, const char *user_name, int reason)
{
    printf("\n remote user offline  %s:%s \n", channel, user_name);
};

static void byte_rtc_on_user_mute_audio(byte_rtc_engine_t engine, const char *channel, const char *user_name, int muted)
{
    printf("\n remote user mute audio  %s:%s %d \n", channel, user_name, muted);

};

static void byte_rtc_on_user_mute_video(byte_rtc_engine_t engine, const char *channel, const char *user_name, int muted)
{
    printf("\n remote user mute video  %s:%s %d \n", channel, user_name, muted);
};

static void byte_rtc_on_connection_lost(byte_rtc_engine_t engine, const char *channel)
{
    printf("\n connection Lost  %s \n", channel);
};

bool is_pcma_silence(const uint8_t *data, size_t length)
{
    const uint8_t SILENT_BYTE = 0xD5; // A-law静音编码值
    for (size_t i = 0; i < length; i++) {
        if (data[i] != SILENT_BYTE) {
            return false;
        }
    }
    return true;
}

static void byte_rtc_on_audio_data(byte_rtc_engine_t engine, const char *channel, const char *user_name, uint16_t sent_ts,
                                   audio_data_type_e codec, const void *data_ptr, size_t data_len)
{
    /* printf("\n byte_rtc_on_audio_data, len:%d\n", data_len); */
// #ifdef AUDIO_TYPE_G711A
#if 1
    size_t sample_count = data_len;  // 样本数 = 数据字节数（1字节/样本）
    short *pcm = malloc(sample_count * sizeof(short));

    /* received = 1; */
    if (!pcm) {
        printf("Memory allocation failed\n");
        return;
    }

    if (!is_pcma_silence(data_ptr, data_len)) {
        unsigned char *pcma = (unsigned char *)data_ptr;

        for (size_t i = 0; i < sample_count; i++) {
            int pcm_val = alaw2linear(pcma[i]);    // 转换为int（假设结果在short范围内）
            pcm[i] = (short)pcm_val;               // 安全截断为16位
        }
        // 网络音频数据写入播放
        _device_write_voice_data(pcm, sample_count * sizeof(short));
    } else {

    }
    free(pcm);
#endif
// #else
//     _device_write_voice_data(data_ptr, data_len);
// #endif
};

static void byte_rtc_on_video_data(byte_rtc_engine_t engine, const char *channel, const char *user_name, uint16_t sent_ts,
                                   video_data_type_e codec,  int is_key_frame,
                                   const void *data_ptr, size_t data_len)
{
    // if (is_key_frame) {
    printf("\n received key frame of user: %s \n", user_name);
    // }
};

static void byte_rtc_on_channel_error(byte_rtc_engine_t engine, const char *channel, int code, const char *msg)
{
    printf("\n error occur %s %d %s\n", channel, code, msg ? msg : "");
};

static void byte_rtc_on_gloable_error(byte_rtc_engine_t engine, int code, const char *msg)
{
    printf("\n global error occur %d %s \n", code, msg ? msg : "");
};

static void byte_rtc_on_keyframe_gen_req(byte_rtc_engine_t engine, const char *channel, const char *user_name)
{
    printf("\n remote req key frame %s:%s \n", channel, user_name);
};

static void byte_rtc_on_target_bitrate_changed(byte_rtc_engine_t engine, const char *channel, uint32_t target_bps)
{
    // printf("byte_rtc_on_target_bitrate_changed, channel=%s, target_bps=%d\n", channel, target_bps);
};

static void byte_rtc_on_token_privilege_will_expire(byte_rtc_engine_t engine, const char *token)
{
    printf("\n token privilege will expire %s", token);
};

static void byte_rtc_on_fini_notify(byte_rtc_engine_t engine)
{
    printf("byte_rtc_on_fini_notify");
    g_byte_rtc_data.fini_notifyed = true;
}

// remote message
// 字幕消息 参考https://www.volcengine.com/docs/6348/1337284
static void on_subtitle_message_received(byte_rtc_engine_t engine, const cJSON *root)
{
    cJSON *type_obj = cJSON_GetObjectItem(root, "type");
    if (type_obj != NULL && strcmp("subtitle", cJSON_GetStringValue(type_obj)) == 0) {
        cJSON *data_obj_arr = cJSON_GetObjectItem(root, "data");
        cJSON *obji = NULL;
        cJSON_ArrayForEach(obji, data_obj_arr) {
            cJSON *user_id_obj = cJSON_GetObjectItem(obji, "userId");
            cJSON *text_obj = cJSON_GetObjectItem(obji, "text");
            if (user_id_obj && text_obj) {
                printf("subtitle:%s:%s", cJSON_GetStringValue(user_id_obj), cJSON_GetStringValue(text_obj));
            }
        }
    }
}

// function calling 消息 参考 https://www.volcengine.com/docs/6348/1359441
static void on_function_calling_message_received(byte_rtc_engine_t engine, const cJSON *root, const char *json_str)
{
    printf("on_function_calling_message_received");
}

static void byte_rtc_on_message_received(byte_rtc_engine_t engine, const char *room, const char *src, const uint8_t *message, int size, bool binary)
{
    /* printf("byte_rtc_on_message_received, len=%d, strlen=%d\n", size, strlen((char *)message)); */
    static char message_buffer[4096];
    if (size > 8) {

        memcpy(message_buffer, message, size);
        message_buffer[size] = 0;
        message_buffer[size + 1] = 0;
        cJSON *root = cJSON_Parse(message_buffer + 8);

        char *json_string = cJSON_Print(root);
        /* printf("recv message: %s", json_string); */
        cJSON_free(json_string);
        if (root != NULL) {
            if (message[0] == 's' && message[1] == 'u' && message[2] == 'b' && message[3] == 'v') {
                // 字幕消息
                on_subtitle_message_received(engine, root);
            } else if (message[0] == 't' && message[1] == 'o' && message[2] == 'o' && message[3] == 'l') {
                // function calling 消息
                on_function_calling_message_received(engine, root, message_buffer + 8);
            } else {
                printf("unknown json message: %s", message_buffer + 8);
            }
            cJSON_Delete(root);
        } else {
            printf("unknown message.");
        }
    } else {
        printf("unknown message.");
    }
};

int VolcEngineRTCDemo()
{
    static u8 pcma[160];
    memset(&g_byte_rtc_data, 0, sizeof(g_byte_rtc_data));
    rtc_room_info_t *room_info = malloc(sizeof(rtc_room_info_t));
    int start_ret = start_voice_chat(room_info);
    if (start_ret != 0) {
        printf("Bot start Failed, ret = %d", start_ret);
        return -1;
    }
    printf("roomid = %s, uid = %s, token = %s, appid = %s, task_id = %s", room_info->room_id, room_info->uid, room_info->token, room_info->app_id, room_info->task_id);

    audio_stream_init(8000, 16, 1); //初始化音频流收发
    start_audio_stream(); //开启音频流收发

// #ifdef AUDIO_TYPE_G711A
    /* audio_stream_init(8000, 16, 1); //初始化音频流收发 */
// #elif defined(AUDIO_TYPE_AACLC)
    /* audio_stream_init(48000, 16, 1); //初始化音频流收发 */
// #elif defined(AUDIO_TYPE_OPUS)
//     audio_stream_init(16000, 16, 1); //初始化音频流收发
// #endif
    /* start_audio_stream(); //开启音频流收发 */
    byte_rtc_event_handler_t eventHandle = {
        .on_global_error            =   byte_rtc_on_gloable_error,
        .on_join_room_success       =   byte_rtc_on_join_channel_success,
        .on_room_error              =   byte_rtc_on_channel_error,
        .on_user_joined             =   byte_rtc_on_user_joined,
        .on_user_offline            =   byte_rtc_on_user_offline,
        .on_user_mute_audio         =   byte_rtc_on_user_mute_audio,
        .on_user_mute_video         =   byte_rtc_on_user_mute_video,

        .on_audio_data              =   byte_rtc_on_audio_data,
        .on_video_data              =   byte_rtc_on_video_data,

        .on_message_received        =   byte_rtc_on_message_received,

        .on_key_frame_gen_req       =   byte_rtc_on_keyframe_gen_req,
        .on_target_bitrate_changed  =   byte_rtc_on_target_bitrate_changed,
        .on_token_privilege_will_expire =   byte_rtc_on_token_privilege_will_expire,
        .on_fini_notify             =   byte_rtc_on_fini_notify
    };

    byte_rtc_engine_t engine = byte_rtc_create(room_info->app_id, &eventHandle);

    byte_rtc_set_log_level(engine, BYTE_RTC_LOG_LEVEL_ERROR); // 日志级别
    byte_rtc_set_params(engine, "{\"debug\":{\"log_to_console\":1}}");

    byte_rtc_init(engine);  //初始化平台

// #ifdef AUDIO_TYPE_G711A
    byte_rtc_set_audio_codec(engine, AUDIO_CODEC_TYPE_G711A);
// #elif defined(AUDIO_TYPE_AACLC)
    /* byte_rtc_set_audio_codec(engine, AUDIO_CODEC_TYPE_AACLC); */
// #elif defined(AUDIO_TYPE_OPUS)
//     byte_rtc_set_audio_codec(engine, AUDIO_CODEC_TYPE_OPUS);
// #endif

    byte_rtc_room_options_t options;
    options.auto_subscribe_audio = 1; // 接收远端音频
    options.auto_subscribe_video = 0; // 不接收远端视频
    options.auto_publish_audio = 1; // 是否自动发布音视频
    options.auto_publish_video = 0; //

    byte_rtc_join_room(engine, room_info->room_id, room_info->uid, room_info->token, &options);  //加入房间
    while (!g_byte_rtc_data.channel_joined) {
        mdelay(100);
    };
    /* mdelay(5 * 1000); //等待房间连接成功 */
// #ifdef AUDIO_TYPE_G711A
    const int DEFAULT_READ_SIZE = 320;
// #else
//     const int DEFAULT_READ_SIZE = 40;
    /* const int DEFAULT_READ_SIZE = 160; */
// #endif
    uint8_t *audio_buffer = malloc(DEFAULT_READ_SIZE);
    if (!audio_buffer) {
        printf("Failed to alloc audio buffer!");
        return -1;
    }

#if 1
    while (true && joined) {    // 发送音频数据，根据需要设置打断循环条件
        if (get_recoder_state()) {
            int ret = _device_get_voice_data(audio_buffer, DEFAULT_READ_SIZE);
            if (ret == DEFAULT_READ_SIZE && joined) {
                // 发送音频数据
                int i, j = 0, ret = 0;
                short *pcm = (short *)audio_buffer;
                int len = DEFAULT_READ_SIZE / 2;
                for (i = 0; i < len; i++, j++) {
                    pcma[j] = linear2alaw(pcm[i]);
                }
                audio_frame_info_t audio_frame_info = {.data_type = AUDIO_DATA_TYPE_PCMA};
                byte_rtc_send_audio_data(engine, room_info->room_id, pcma, len, &audio_frame_info);
            }
        }
    }
#endif
err:
    stop_audio_stream();
    free(audio_buffer);

    byte_rtc_leave_room(engine, room_info->room_id);
    byte_rtc_fini(engine);
    while (!g_byte_rtc_data.fini_notifyed) {
        mdelay(1000);
    };
    byte_rtc_destroy(engine);
    stop_voice_chat(room_info);
    free(room_info);
}
