#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "e2e.h"

#include "websocket_api.h"
#ifdef WEBSOCKET_MEMHEAP_DEBUG
#include "mem_leak_test.h"
#endif


static struct websocket_struct *websockets_info = NULL;
/**** websoket "ws://"  websokets "wss://"  *****/

//#define OBJ_URL        "ws://82.157.123.54:9010/ajaxchattest" //远程服务器测试
//#define OBJ_URL 	"ws://121.40.165.18:8800"
//#define OBJ_URL     "ws://www.example.com/socketserver"
//#define OBJ_URL 	"wss://localhost:8888"                  //本地服务器测试
//#define OBJ_URL 	"wss://172.16.23.28:8888"                  //本地服务器测试


static void websockets_callback(u8 *buf, u32 len, u8 type)
{
    if (type == 129) {
        printf("-----------------recv msg ：\n%s\n", buf);
    }

    if (type == 130) {
        _device_write_voice_data(buf, len);
    }
    printf("-----------------wbs recv msg len: %d, type:%d---------------------\n", len, type);
}

/*******************************************************************************
*   Websocket Client api
*******************************************************************************/
static void websockets_client_reg(struct websocket_struct *websockets_info, char mode)
{
    memset(websockets_info, 0, sizeof(struct websocket_struct));
    websockets_info->_init           = websockets_client_socket_init;
    websockets_info->_exit           = websockets_client_socket_exit;
    websockets_info->_handshack      = webcockets_client_socket_handshack;
    websockets_info->_send           = websockets_client_socket_send;
    websockets_info->_recv_thread    = websockets_client_socket_recv_thread;
    websockets_info->_heart_thread   = websockets_client_socket_heart_thread;
    websockets_info->_recv_cb        = websockets_callback;
    websockets_info->_recv           = NULL;

    websockets_info->websocket_mode  = mode;

#if 0
    //应用层指定申请接收内存大小（申请即可，不用释放，内部会释放），默认不使用
    websockets_info->recv_buf_size	 = 30 * 1024;
    websockets_info->recv_buf		 = malloc(websockets_info->recv_buf_size);
#endif
}

static int websockets_client_init(struct websocket_struct *websockets_info, u8 *url, const char *origin_str, const char *user_agent_str)
{
    websockets_info->ip_or_url = url;
    websockets_info->origin_str = origin_str;
    websockets_info->user_agent_str = user_agent_str;
    websockets_info->recv_time_out = 1000;

    //应用层和库的版本检测，结构体不一样则返回出错
    int err = websockets_struct_check(sizeof(struct websocket_struct));
    if (err == FALSE) {
        return err;
    }
    return websockets_info->_init(websockets_info);
}

static int websockets_client_handshack(struct websocket_struct *websockets_info)
{
    return websockets_info->_handshack(websockets_info);
}

static int websockets_client_send(struct websocket_struct *websockets_info, u8 *buf, int len, char type)
{
//    printf("len:%d", len);
    //SSL加密时一次发送数据不能超过16K，用户需要自己分包
    return websockets_info->_send(websockets_info, buf, len, type);
}

static void websockets_client_exit(struct websocket_struct *websockets_info)
{
    websockets_info->_exit(websockets_info);
}

static int web_soc_connected = 0;
static int web_recved = 0;

static int free_cancel()
{
    if (websockets_info) {
        if (websockets_info->ping_thread_id) {
            thread_kill(&websockets_info->ping_thread_id, KILL_REQ);
        }
        if (websockets_info->recv_thread_id) {
            thread_kill(&websockets_info->recv_thread_id, KILL_REQ);
        }
        printf("sound_free rws_socket_disconnect_and_release in \n");
        websockets_client_exit(websockets_info);
        printf("sound_free rws_socket_disconnect_and_release out\n");
        free(websockets_info);
        websockets_info = NULL;
    }
    return 0;
}


int iat_start()
{
    int err;
    char mode = WEBSOCKET_MODE;
    char *url = e2e_voice_start();
    const char *origin_str = "biz-llmvoicechat.xfyun.cn";

    printf("sound_free rws_socket_set_url %s\n", url);

    websockets_info = malloc(sizeof(struct websocket_struct));
    memset(websockets_info, 0, sizeof(struct websocket_struct));

    /* 1 . register */
    websockets_client_reg(websockets_info, mode);

    /* 2 . init */
    err = websockets_client_init(websockets_info, url, NULL, NULL);
    if (FALSE == err) {
        printf("  . ! Cilent websocket init error !!!\r\n");
        goto exit_ws;
    }

    /* 3 . hanshack */
    err = websockets_client_handshack(websockets_info);
    if (FALSE == err) {
        printf("  . ! Handshake error !!!\r\n");
        goto exit_ws;
    }
    printf(" . Handshake success \r\n");

    thread_fork("websocket_client_heart", 19, 512, 0,
                &websockets_info->ping_thread_id,
                websockets_info->_heart_thread,
                websockets_info);
    thread_fork("websocket_client_recv", 18, 512, 0,
                &websockets_info->recv_thread_id,
                websockets_info->_recv_thread,
                websockets_info);

    websockets_sleep(1000);

    free(url);
    return 0;
exit_ws:
    /* 6 . exit */

    if (websockets_info->ping_thread_id) {
        thread_kill(&websockets_info->ping_thread_id, KILL_REQ);
    }
    if (websockets_info->recv_thread_id) {
        thread_kill(&websockets_info->recv_thread_id, KILL_REQ);
    }
    websockets_client_exit(websockets_info);
    free(websockets_info);
    e2e_voice_destroy();
    return 0;
}

int iat_write(const char *audioData, unsigned int dataLen)
{
    char *data_up = e2e_voice_write(audioData, dataLen);
    if (data_up) {
        printf(">>>>>>> write %s", data_up);
        websockets_client_send(websockets_info, (u8 *)data_up, strlen(data_up), WCT_TXTDATA);
        free(data_up);
    }
    return 0;
}

int iat_commit()
{
    char *data_up = e2e_voice_commit();
    if (data_up) {
        printf(">>>>>>> commit %s", data_up);
        websockets_client_send(websockets_info, (u8 *)data_up, strlen(data_up), WCT_TXTDATA);
        free(data_up);
    }
    return 0;
}

int iat_stop()
{

    if (websockets_info->ping_thread_id) {
        thread_kill(&websockets_info->ping_thread_id, KILL_REQ);
    }
    if (websockets_info->recv_thread_id) {
        thread_kill(&websockets_info->recv_thread_id, KILL_REQ);
    }
    if (websockets_info) {
        printf("sound_free rws_socket_disconnect_and_release in \n");
        websockets_client_exit(websockets_info);
        printf("sound_free rws_socket_disconnect_and_release out\n");
        free(websockets_info);
        websockets_info = NULL;
    }
    e2e_voice_destroy();
    return 0;
}

#define DEFAULT_ONECE_SIZE 1280
static int init_e2e = 0;
static int flag = 0;
void audio_timeout()
{
    flag = 0;
    return;
}

void process_audio()
{
    unsigned char audio_buffer[DEFAULT_ONECE_SIZE]; // 存储原始音频数据的缓冲区
    int fetch_result;
    static int audio_state = 0;
    if (get_recoder_state() && !flag) {
        flag = 1;
        sys_timeout_add_to_task("timeout_", 10, audio_timeout, 5 * 1000);
    }

    while (flag) {
        if (!init_e2e) {
            free_cancel();
            iat_start();
            init_e2e = 1;
        }
        // 获取音频数据
        fetch_result = _device_get_voice_data(audio_buffer, DEFAULT_ONECE_SIZE);
        if (fetch_result != DEFAULT_ONECE_SIZE) {
            continue;
        }
        iat_write(audio_buffer, DEFAULT_ONECE_SIZE);
        audio_state = 1;
    }
    init_e2e = 0;
    if (flag == 0) {
        if (audio_state) {
            printf("\n----------------SEND AUDIO END----------------\n");
            iat_commit();
            audio_state = 0;
        }
    }
}

void xf_demo()
{
    /* wlsx_audio_init(16000, 16, 1); */
    /* wlsx_init_audio_stream(); */

    audio_stream_init(16000, 16, 1); //初始化音频流收发
    start_audio_stream(); //开启音频流收发

    e2e_voice_create("98e573c240a74499835b25a95b367100", "dd5e8b5a54bf4b6ea98adebe5b14a452", "adc10010301101");
    e2e_voice_set_tts_param("aue", "raw");
    while (1) {
        process_audio();
    }
}
