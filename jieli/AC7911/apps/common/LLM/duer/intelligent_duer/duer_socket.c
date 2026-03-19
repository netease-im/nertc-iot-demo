#include "websocket_api.h"
#include "my_platform_common.h"
#include "duer_common.h"
#include "fs/fs.h"
#include "duer_record.h"

#if INTELLIGENT_DUER
#define OBJ_URL 	"ws://duer-kids.baidu.com/sandbox/sota/realtime_asr?sn=%s"
static int init = 0;
static void task_kill_callback(char *buf);
static void websockets_callback(u8 *buf, u32 len, u8 type)
{
    MY_LOG_I("wbs recv msg : %s\n", buf);
    InsideRCResponse *response = duer_parse_inside_rc_json((char *)buf);
    if (response) {
        duer_free_inside_rc_response(response);
    }
}
extern duer_rec_t rec;
extern int task_kill(const char *name);
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
    MY_LOG_I("myurl %s \n", websockets_info->ip_or_url);
    return websockets_info->_handshack(websockets_info);
}

static int websockets_client_send(struct websocket_struct *websockets_info, u8 *buf, int len, char type)
{
    //SSL加密时一次发送数据不能超过16K，用户需要自己分包
    return websockets_info->_send(websockets_info, buf, len, type);
}

static void websockets_client_exit(struct websocket_struct *websockets_info)
{
    websockets_info->_exit(websockets_info);
}


/*******************************************************************************
*   Websocket Client.c
*   Just one example for test
*******************************************************************************/
static void websockets_client_main_thread(void *priv)
{
    int err;
    char mode = WEBSOCKET_MODE;
    char cuid_str[7]; // 6个字符 + 结束符
    char url[256];
    char *start_str = NULL;
    char *finish = NULL;
    TokenData *token = NULL;
    duer_generate_6char_random_string(cuid_str);//生成cuid
    MY_LOG_D("Random String : %s\n", cuid_str);
    snprintf(url, sizeof(url), OBJ_URL, cuid_str);
    MY_LOG_D(">>>>>url %s \n", url);
    const char *origin_str = "http://coolaf.com";
    /* 0 . malloc buffer */
    struct websocket_struct *websockets_info = malloc(sizeof(struct websocket_struct));
    if (!websockets_info) {
        return;
    }
    /* 1 . register */
    websockets_client_reg(websockets_info, mode);

    /* 2 . init */
    err = websockets_client_init(websockets_info, (u8 *)url, NULL, NULL);
    if (FALSE == err) {
        MY_LOG_E("  . ! Cilent websocket init error !!!\r\n");
        goto exit_ws;
    }

    /* 3 . hanshack */
    err = websockets_client_handshack(websockets_info);
    if (FALSE == err) {
        MY_LOG_E("  . ! Handshake error !!!\r\n");
        goto exit_ws;
    }
    MY_LOG_D(" . Handshake success \r\n");

    /* 4 . CreateThread */
    thread_fork("websocket_client_heart", 19, 512, 0,
                &websockets_info->ping_thread_id,
                websockets_info->_heart_thread,
                websockets_info);
    thread_fork("websocket_client_recv", 18, 512, 0,
                &websockets_info->recv_thread_id,
                websockets_info->_recv_thread,
                websockets_info);

    websockets_sleep(1000);

    char dialog_id[60];
    duer_generate_dialog_request_id(dialog_id);// 生成对话请求ID
    MY_LOG_D("Generated Dialog Request ID: %s\n", dialog_id);
    token = duer_get_access_token();
    if (!token) {
        MY_LOG_E("Failed to get access token\n");
        return;
    }
    start_str = duer_start_frame(DUER_CLIENT_AK, DUER_CLIENT_SK, DUER_CLIENT_AK, DUER_APP_PID,
                                 cuid_str, DUER_APP_FORMAT, 1, DUER_APP_SAMPLE,
                                 cuid_str, token->access_token, 1,
                                 1, dialog_id, 1,
                                 "1");
    MY_LOG_D(">>>start_str %s \n", start_str);
    err = websockets_client_send(websockets_info, (u8 *)start_str, strlen(start_str), WCT_TXTDATA);
    if (FALSE == err) {
        MY_LOG_E("  . ! send err !!!\r\n");
        goto exit_ws;
    }
    os_time_dly(40);
    u8 buffer[640];
    finish = build_finish_frame();
    duer_rec_start();
    u8 send_buf[40];
    while (1) {
        int rlen = cbuf_read(&rec.cbuf, send_buf, 40);
        if (rlen != 40) {
            cbuf_read(&rec.cbuf, send_buf, rlen);
        }
        err = websockets_client_send(websockets_info, send_buf, rlen, WCT_BINDATA);
        if (false == err) {
            MY_LOG_E("  . ! send err !!!\r\n");
            goto exit_ws;
        }
        websockets_sleep(10);
    }
exit_ws:
    /* 6 . exit */
    duer_rec_stop();
    if (websockets_info->ping_thread_id) {
        thread_kill(&websockets_info->ping_thread_id, KILL_WAIT);
    }
    if (websockets_info->recv_thread_id) {
        thread_kill(&websockets_info->recv_thread_id, KILL_WAIT);
    }
    websockets_client_exit(websockets_info);
    if (websockets_info) {
        free(websockets_info);
    }
    if (start_str) {
        free(start_str);
    }
    if (finish) {
        free(finish);
    }
    duer_free_token_data(token);
    init = 0;
}


void my_duer_websocket_client_thread_create(void)
{
    if (init) {
        printf("duer thread has been running");
        return;
    }
    my_url_list_init();
    thread_fork("websockets_client_main", 15, 512 * 3, 0, 0, websockets_client_main_thread, NULL);
    init = 1;
}

#endif
