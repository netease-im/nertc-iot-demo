#include "app_config.h"
#include "server/audio_server.h"
#include "server/server_core.h"
#include "fs/fs.h"
#include "character_coding.h"

#ifdef CONFIG_HTTP_SERVER_ENABLE

#ifndef CONFIG_AUDIO_ADC_CHANNEL_L
#define CONFIG_AUDIO_ADC_CHANNEL_L          1
#endif
#ifndef CONFIG_AUDIO_ADC_CHANNEL_R
#define CONFIG_AUDIO_ADC_CHANNEL_R          3
#endif

#define HTTP_RECORDER_CMD_START  "start?file="
#define HTTP_RECORDER_CMD_STOP   "stop"
#define HTTP_RECORDER_CMD_STATUS "status"

static void *recorder_hdl;
static FILE *fp;

//录音文件到SD卡
static int http_server_recorder_open(const char *filename, int sample_rate, u8 channel, const char *format)
{
    union audio_req req = {0};
    char __path[256];
    char path[512];

    if (recorder_hdl) {
        return -1;
    }

    recorder_hdl = server_open("audio_server", "enc");
    if (!recorder_hdl) {
        return -1;
    }

    snprintf(__path, sizeof(__path), CONFIG_ROOT_PATH"%s", filename);
    long_file_name_encode(__path, (u8 *)path, sizeof(path));
    printf("http_server_recorder_open file : %s\n", path);

    req.enc.file = fp = fopen(path, "w+");
    if (!fp) {
        server_close(recorder_hdl);
        recorder_hdl = NULL;
        return -1;
    }
    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = channel;
    req.enc.volume = 100;
    req.enc.frame_size = 8192;
    req.enc.output_buf_len = req.enc.frame_size * 10;
    req.enc.sample_rate = sample_rate;
    req.enc.sample_source = "mic";
    req.enc.format = format;
    if (!strcmp(req.enc.format, "aac")) {
        req.enc.bitrate = sample_rate * 4;
        req.enc.no_header = 1;
    }
    /* req.enc.sample_depth = 3; //IIS支持采集24bit深度 */
    if (channel == 4) {
        req.enc.channel_bit_map = 0x0f;
    } else if (channel == 2) {
        req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L) | BIT(CONFIG_AUDIO_ADC_CHANNEL_R);
    } else {
        req.enc.channel_bit_map = BIT(CONFIG_AUDIO_ADC_CHANNEL_L);
    }

    if (0 != server_request(recorder_hdl, AUDIO_REQ_ENC, &req)) {
        fdelete(fp);
        fp = NULL;
        server_close(recorder_hdl);
        return -1;
    }

    return 0;
}

static void http_server_recorder_close(void)
{
    union audio_req req = {0};

    if (recorder_hdl) {
        req.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(recorder_hdl, AUDIO_REQ_ENC, &req);
        if (fp) {
            fclose(fp);
            fp = NULL;
        }
        server_close(recorder_hdl);
        recorder_hdl = NULL;
    }
}

int http_server_get_header_callback(char *buf)
{
    const char *find = NULL;

    do {
        find = strstr(buf, HTTP_RECORDER_CMD_START);
        if (find) {
            if (0 == http_server_recorder_open(find + strlen(HTTP_RECORDER_CMD_START), 16000, 2, "wav")) {
                strcpy(buf, "START OK");
            } else {
                strcpy(buf, "START FAIL");
            }
            break;
        }

        find = strstr(buf, HTTP_RECORDER_CMD_STOP);
        if (find) {
            http_server_recorder_close();
            strcpy(buf, "STOP OK");
            break;
        }

        find = strstr(buf, HTTP_RECORDER_CMD_STATUS);
        if (find) {
            if (recorder_hdl) {
                strcpy(buf, "START");
            } else {
                strcpy(buf, "STOP");
            }
            break;
        }

        buf[0] = 0;

        return -1;
    } while (0);

    return 1;
}

#endif
