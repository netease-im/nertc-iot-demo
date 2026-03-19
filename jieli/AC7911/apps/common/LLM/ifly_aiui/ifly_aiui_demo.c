#include "cJSON.h"
#include "system/includes.h"
#include "ifly_aiui_net.h"
#include "ifly_aiui_sdk_common.h"
#include "aiui_get_firmware_info.h"
#include "wifi/wifi_connect.h"
#include "server/ai_server.h"


#define IFLY_RECOGNIZE_TASK_NAME "app_recognize_task"

char *ifly_aiui_demo_version(void);

static bool s_start_send = false;
void user_aiui_init(void)
{
    unsigned char vl_reboot_type;
    t_ifly_aiui_sdk_common_data param;
    param.appid = "f5ca7850";
    param.apikey = "b89f99da18d017ea0c3dfb5c182b7814";
    param.apisecret = "MWYyY2ExZjY3ZWIwODgwMmY4NzQxM2Jk";

    u8 flash_uid[16];
    memcpy(flash_uid, get_norflash_uuid(), 16);
    for (int i = 0; i < 16; i++) {
        sprintf(&param.device_id[2 * i], "%02X", flash_uid[i]);
    }
    printf("device id:%s", param.device_id);
    ifly_aiui_sdk_common_data_init(&param);
    printf("aiui_sdk_version:%s\n", ifly_aiui_sdk_version());
}
void ifly_aiui_custom_websockets_cb(u8 *buf, u32 vp_len, u8 type);
void aiui_demo_task()
{
    user_aiui_init();
    audio_stream_init(16000, 16, 1); //初始化音频流收发
    start_audio_stream(); //开启音频流收发
    enum wifi_sta_connect_state state;
    while (1) {
        printf("Connecting to the network...\n");
        state = wifi_get_sta_connect_state();
        if (WIFI_STA_NETWORK_STACK_DHCP_SUCC == state) {
            printf("Network connection is successful!\n");
            break;
        }
        os_time_dly(500);
    }

    ifly_aiui_process_init(ifly_aiui_custom_websockets_cb);
    mdelay(5 * 1000);
    int err = ifly_aiui_start_voice_req();
    const int DEFAULT_READ_SIZE = IFLY_AIUI_VOICE_ONCE_SEND_BYTES;
    uint8_t *audio_buffer = malloc(DEFAULT_READ_SIZE);
    if (!audio_buffer) {
        printf("Failed to alloc audio buffer!");
        return;
    }
    unsigned char vl_voice_begin_send = 0;
    while (1) {
        int len = _device_get_voice_data(audio_buffer, DEFAULT_READ_SIZE);
        if (len > 0) {
            if (ifly_aiui_app_get_connect_status() != 0) {
                if (vl_voice_begin_send == 0) {
                    if (OAL_SUCCESS == ifly_aiui_send_voice_start(audio_buffer, len)) {
                        vl_voice_begin_send = 1;
                        printf("aiui encoded voice begin send\n");
                    } else {
                        s_start_send = false;
                        printf("aiui encoded voice begin false\n");
                    }
                } else if (OAL_FAILED == ifly_aiui_send_voice_data((const char *)audio_buffer, len)) {
                    if (s_start_send) {
                        s_start_send = false;
                    }
                    printf("aiui encoded send failed.......\n");
                }
            }
        }
    }
    ifly_aiui_stop_voice_req();
    ifly_aiui_app_uninit();

    stop_audio_stream();
    free(audio_buffer);
    audio_buffer = NULL;
}
