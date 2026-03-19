#ifndef __COZE_CONFIG_H__
#define __COZE_CONFIG_H__
#include "app_config.h"

#define LOG_TAG_CONST   GIZ_CHAT
#define LOG_TAG_VAR     "[GIZ_CHAT]"
#define log_info          printf
#define log_debug          printf
#define log_error          printf

#define CHAT_BY_MY_COZE     0
#define COZE_HURL           "ws://ws.coze.cn/v1/chat"
#define COZE_BOT_ID         "7590427972948721698"
#define COZE_TOKEN          "pat_QKWuPelL5DMPD9b1fS73dmfGSxqTZ0oJFNqzVGl7gwT0qms7qGzVjrYGMImc2bT1"


#define GIZ_CHAT_WS_SERVICE_URL             "ws://ws.coze.cn/v1/chat"   // ws.cdn-qa.cn  ws.coze.cn
#define GIZ_CHAT_WS_ORIGIN_URL              "http://coolaf.com"
#define GIZ_CHAT_WS_RECV_TIMEOUT_MS         30000
#define GIZ_CHAT_WS_SEND_INTERVAL_MS        30000


#define GIZWITS_RTC_ENABLE	            1
#define TEST_SAVE_PCM                   0
#define COZE_INPUT_ENC_FORMAT           1 //pcm:0,opus:1
#if COZE_INPUT_ENC_FORMAT
#define COZE_INPUT_ENC_FORMAT_STRING     "pcm"
#define COZE_INPUT_SERVER_FORMAT_STRING  "opus"
#define COZE_INPUT_ENC_SAMPLE_RATE       16000
#define COZE_INPUT_ENC_CHANNEL            1
#define COZE_INPUT_OUTPUT_LEN            8000
#else
#define COZE_INPUT_ENC_FORMAT_STRING     "pcm"
#define COZE_INPUT_SERVER_FORMAT_STRING  "pcm"
#define COZE_INPUT_ENC_SAMPLE_RATE      8000
#define COZE_INPUT_ENC_CHANNEL          1
#define COZE_INPUT_OUTPUT_LEN           (COZE_INPUT_ENC_SAMPLE_RATE*3)
#endif


#define COZE_OUTPUT_DEC_FORMAT           1 //pcm:0,opus:1
#if (COZE_OUTPUT_DEC_FORMAT==1)
#define COZE_OUTPUT_DEC_FORMAT_STRING    "opus"
#define COZE_OUTPUT_DEC_SAMPLE_RATE      16000
#define COZE_OUTPUT_DEC_CHANNEL          1
#define COZE_OUTPUT_DEC_SAMPLE_MS        20
#define COZE_OUTPUT_DEC_USE_CBR          true
#define COZE_OUTPUT_DEC_FORMAT_BUFFER    5000
#define COZE_OUTPUT_DEC_FORMAT_SIZE      4000
#else
#define COZE_OUTPUT_DEC_FORMAT_STRING    "pcm"
#define COZE_OUTPUT_DEC_SAMPLE_RATE      8000
#define COZE_OUTPUT_DEC_CHANNEL          1
#define COZE_OUTPUT_DEC_SAMPLE_MS        125
#define COZE_OUTPUT_DEC_FORMAT_BUFFER    8000
#endif


typedef enum {
    WS_AUDIO_FORMAT_WAV = 0,
    WS_AUDIO_FORMAT_PCM,
    WS_AUDIO_FORMAT_OGG,
    WS_AUDIO_FORMAT_OPUS,
    WS_AUDIO_FORMAT_MAX,
} ws_audio_format_t;
#endif /* __COZE_CONFIG_H__ */
