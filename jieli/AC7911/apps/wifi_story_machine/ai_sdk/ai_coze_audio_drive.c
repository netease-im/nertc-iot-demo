#include "app_config.h"
#include "system/includes.h"
#include "ai_coze_server.h"
#include "ai_coze_api_message.h"
#ifdef CONFIG_NERTC_CONNECTION
#include "nertc_protocol.h"
#endif

static int giz_encoder_pid = 0;

#define MIN_VOLUME_VALUE	5
#define MAX_VOLUME_VALUE	100
#define INIT_VOLUME_VALUE   50

struct recorder_hdl {
    volatile u8 is_enc_run;
};
static struct recorder_hdl recorder_handler;
#define __this (&recorder_handler)

static volatile u8 is_manual_stop = 0;  /* 标记是否为手动停止 */

void coze_rtc_stop(void){
    is_manual_stop = 1;  /* 标记为手动停止，不触发断开提示 */
    __this->is_enc_run=0;
    mdelay(100);
    stop_audio_stream();
    /* 注意: 不再在这里调用 nertc_protocol_close_audio_channel()
     * 音频通道的开关由按钮事件 AI_EVENT_RTC_ONOFF 控制 */
}

int rtc_thread(){
    #define DEFAULT_READ_SIZE  120
    static u8 audio_buffer[DEFAULT_READ_SIZE];
    int read_len,send_len,re,times=0;
    audio_stream_init(16000, 16, 1); //初始化音频流收发
    start_audio_stream(); //开启音频流收发

    while (__this->is_enc_run) {    // 发送音频数据，根据需要设置打断循环条件
        //if (get_recoder_state()) {
            read_len = _device_get_voice_data(audio_buffer, DEFAULT_READ_SIZE);
            // printf("read_len:%d",read_len);
            if(__this->is_enc_run && read_len){
#ifdef CONFIG_NERTC_CONNECTION
                /* 检查音频通道是否打开，未打开则跳过发送（不计入失败次数） */
                if (!nertc_protocol_is_audio_channel_opened()) {
                    continue;
                }
                // 发送到 NERTC 服务器
                send_len = send_voice_data_to_nertc_server(audio_buffer, read_len);
#else
                // 发送到 Coze WebSocket 服务器
                send_len = send_voice_data_to_server(audio_buffer, read_len);
#endif
                if(!send_len){
                    printf("read_len:%d re:%d",read_len,send_len);
                    if(++times == 10){
                        printf("chat stop......................");
                        coze_rtc_stop();
                        break;
                    }
                }
            }
        //}
    }
    mdelay(100);
    
    /* 只有异常退出才触发断开提示，手动停止不触发 */
    if (!is_manual_stop) {
        ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL, 0);
    }
    is_manual_stop = 0;  /* 重置标记 */
}


void coze_rtc_start(void){
    if(__this->is_enc_run)return;
    __this->is_enc_run=1;

    thread_fork("Volc_demo", 4, 3 * 1024, 0, giz_encoder_pid, rtc_thread, NULL);
}



int dec_save_cbuf_put(void *data, u32 len){
    static u8 put_len=0;
    if(!put_len){
        put_len=1;
        printf("-------------------------------------->put len:%d",len);
    }
    return _device_write_voice_data(data,len);
}

