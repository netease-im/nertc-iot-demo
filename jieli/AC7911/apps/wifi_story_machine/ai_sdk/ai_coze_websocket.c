#include "web_socket/websocket_api.h"
#include "ai_coze_websocket.h"
#include "ai_coze_server.h"
#include "ai_coze_config.h"
#include "ai_device_manager.h"
#include "ai_coze_api_message.h"
#include "nertc_protocol.h"

#ifdef CONFIG_NERTC_CONNECTION
/**
 * @brief NERTC 事件回调 - 处理状态机切换
 */
static void nertc_event_callback(const nertc_protocol_event_t *event, void *user_data)
{
    if (!event) return;
    
    switch (event->event) {
        case NERTC_PROTOCOL_EVENT_TTS_START:
            /* AI 开始说话 -> SPEAKING 状态 */
            ai_set_state(AI_STATE_SPEAKING);
            break;
            
        case NERTC_PROTOCOL_EVENT_TTS_STOP:
            /* AI 停止说话 -> LISTENING 状态 */
            ai_set_state(AI_STATE_LISTENING);
            break;
            
        case NERTC_PROTOCOL_EVENT_DISCONNECTED:
            /* 断开连接 -> IDLE 状态 */
            ai_set_state(AI_STATE_IDLE);
            break;
            
        case NERTC_PROTOCOL_EVENT_ERROR:
            log_info("[NERTC] Error: code=%d, msg=%s", 
                     event->data.error.error_code, 
                     event->data.error.error_msg ? event->data.error.error_msg : "");
            break;
            
        default:
            break;
    }
}
#endif /* CONFIG_NERTC_CONNECTION */

extern struct ai_coze_sdk_api ai_coze_sdk_api;
static coze_ws_info_t coze_ws_info;

static void websockets_recv_callback(u8 *buf, u32 len, u8 type){
    bool is_fin = type & 0x80;
    u8 data_type = type & 0x0f;
    //log_debug("websockets_recv_callback--> type:0x%0x, is_fin:0x%0x, data_type:0x%0x",type, is_fin, data_type);

    switch (data_type) {
    case WCT_TXTDATA:
        coze_ws_recv_handler(buf, len);
        break;
    case WCT_DISCONN:
        ai_send_event_to_sdk(AI_EVENT_WEBS_CLOSE, 1);
    break;
    case WCT_PING:
        put_buf(buf,len);
    break;
    case WCT_PONG:
        put_buf(buf,len);
    break;
    default:
        log_info("!!!!!!!!Unhandled message type: 0x%0x, len: %d", type, len);
        break;
    }
}

//返回回调事件
static void websockets_exit_callback(WEBSOCKET_INFO *ws_info){
    log_debug("websockets_exit_callback");
    //websocket_struct_dump(ws_info);
    printf("ws_info->websocket_valid: %d\n", ws_info->websocket_valid);
    printf("ws_info->ping_thread_id: %d\n", ws_info->ping_thread_id);
    printf("ws_info->recv_thread_id: %d\n", ws_info->recv_thread_id);

}


int ws_client_send(u8 *buf, int len, char type){
    return coze_ws_info._send(&coze_ws_info, buf, len, type);
}


static void giz_chat_start(void *parm){
    log_info("giz_chat_start");

    int err;
    
#ifdef CONFIG_NERTC_CONNECTION
    /* ========== NERTC 模式：只使用 NERTC SDK，不连接 Coze WebSocket ========== */
    {
        /* 设置初始状态：STARTING */
        ai_set_state(AI_STATE_STARTING);
        
        nertc_protocol_config_t nertc_config;
        nertc_protocol_config_init(&nertc_config);
        /* nertc_protocol_config_init 已设置默认值：
         * - app_key = NERTC_DEFAULT_APP_KEY
         * - device_id = NERTC_DEFAULT_DEVICE_ID ("jieli-test")
         * - license = NERTC_DEFAULT_TEST_LICENSE
         * - sample_rate = 16000, channels = 1, frame_duration = 60
         */
        
        log_info("[NERTC] Config: appkey=%s, device_id=%s", 
                 nertc_config.app_key, nertc_config.device_id);
        
        int nertc_ret = nertc_protocol_init_with_config(&nertc_config);
        if (nertc_ret != 0) {
            log_info("[NERTC] Protocol init failed: %d", nertc_ret);
            ai_set_state(AI_STATE_IDLE);  /* 失败回到 IDLE */
            ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL, nertc_ret);
            return;
        }
        
        log_info("[NERTC] Protocol init success, starting...");
        
        /* 注册事件回调（用于状态机切换） */
        nertc_protocol_set_event_callback(nertc_event_callback, NULL);
        
        /* 使用默认房间名和UID启动 */
        nertc_ret = nertc_protocol_start(NULL, 0);
        if (nertc_ret != 0) {
            log_info("[NERTC] Protocol start failed: %d", nertc_ret);
            ai_set_state(AI_STATE_IDLE);  /* 失败回到 IDLE */
            ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL, nertc_ret);
            return;
        }
        
        log_info("[NERTC] Protocol start success");
        /* 注册下行音频回调，将接收到的音频喂给播放设备 */
        nertc_register_audio_callback();
        
        /* 启动成功 -> IDLE 状态（等待 AI 开始说话或用户开始说话） */
        ai_set_state(AI_STATE_IDLE);
        
        /* 发送连接成功事件（触发 MP3 播报） */
        log_info("[NERTC] Connection established, sending success event");
        ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_SUCC, 0);
    }
    /* ========== NERTC 模式结束 ========== */
    return;
    
#else
    /* ========== WebSocket 模式：连接 Coze WebSocket 服务器 ========== */
    
    // 1. 检查当前的连接状态
    // if (coze_ws_info.websocket_valid == ESTABLISHED) {
    //     log_info("giz_chat_start: already connected");
    //     return;
    // }
    // if (coze_ws_info.websocket_valid == INVALID_ESTABLISHED) {
    //     log_info("giz_chat_start: invalid connection");
    //     goto exit_ws;
    // }


    // 2. 注册 WebSocket 客户端
    memset(&coze_ws_info, 0, sizeof(WEBSOCKET_INFO));
    coze_ws_info._init = websockets_client_socket_init;
    coze_ws_info._exit = websockets_client_socket_exit;
    coze_ws_info._handshack = webcockets_client_socket_handshack;
    coze_ws_info._send = websockets_client_socket_send;
    coze_ws_info._recv_thread = websockets_client_socket_recv_thread;
    coze_ws_info._heart_thread = websockets_client_socket_heart_thread;
    coze_ws_info._recv_cb = websockets_recv_callback;
    coze_ws_info._recv = NULL;
    coze_ws_info._exit_notify = websockets_exit_callback; // @NOTICE: 可以用来通知相关线程
    coze_ws_info.websocket_mode = WEBSOCKETS_MODE;

    // 3. 初始化 WebSocket 客户端
    char temp_coze_url[128] = {0};
    char temp_user_agent[128] = {0};
    memset(temp_coze_url, 0, sizeof(temp_coze_url));
    memset(temp_user_agent, 0, sizeof(temp_user_agent));

#if CHAT_BY_MY_COZE
    snprintf(temp_coze_url, sizeof(temp_coze_url), "%s?bot_id=%s", COZE_HURL, COZE_BOT_ID);
    snprintf(temp_user_agent, sizeof(temp_user_agent), "Authorization: Bearer %s", COZE_TOKEN);
#else
    snprintf(temp_coze_url, sizeof(temp_coze_url), "%s?bot_id=%s", GIZ_CHAT_WS_SERVICE_URL, coze_login.bot_id);
    snprintf(temp_user_agent, sizeof(temp_user_agent), "Authorization: Bearer %s", coze_login.access_token);
#endif
    
    printf("temp_coze_url: %s\n", temp_coze_url);
    printf("temp_user_agent: %s\n", temp_user_agent);

    coze_ws_info.ip_or_url = temp_coze_url;
    coze_ws_info.origin_str = GIZ_CHAT_WS_ORIGIN_URL;
    coze_ws_info.user_agent_str = temp_user_agent;
    coze_ws_info.recv_time_out = GIZ_CHAT_WS_RECV_TIMEOUT_MS;

    err = websockets_struct_check(sizeof(WEBSOCKET_INFO));
    if (err == 0) goto exit_ws;
    printf("websockets_struct_check suss!!!");

    err =  coze_ws_info._init(&coze_ws_info);
    if (err == 0)goto exit_ws;
    printf("coze_ws_info._init suss!!!");

    // 4. 进行 WebSocket 连接握手
    err =  coze_ws_info._handshack(&coze_ws_info);
    if (err == 0)goto exit_ws;
    printf("coze_ws_info._handshack suss!!!");

    // 5. 启动 WebSocket 所需线程（心跳、接收）
    err = thread_fork("giz_chat_ws_heart_thread", 19, 4*1024, 0,&coze_ws_info.ping_thread_id,coze_ws_info._heart_thread,&coze_ws_info);
    if (err != OS_NO_ERR)goto exit_ws;
    printf("giz_chat_ws_heart_thread suss!!!");

    err = thread_fork("giz_chat_ws_recv_thread", 18, 4*1024, 0,&coze_ws_info.recv_thread_id,coze_ws_info._recv_thread,&coze_ws_info);
    if (err != OS_NO_ERR) goto exit_ws;
    printf("giz_chat_ws_recv_thread suss!!!");

    printf("\n\n***************************************giz_chat_init end***************************************************\n");
    u32 timeout=0;
    ws_set_chat_state(WS_CHAT_STATE_INIT);
    while(1){
        timeout++;

        if(ws_chat_state ==WS_CHAT_STATE_UPDATED)break;
        switch(timeout){
            case 6:
            send_chat_update_request();
            break;
            case 12:
            if(ws_chat_state !=WS_CHAT_STATE_UPDATED){
                goto exit_ws;
            }
            break;
        }

        printf("giz init:%d",timeout);
        os_time_dly(100);
    }
    return;

exit_ws:
        printf("giz_chat_init error: %d\n", err);
        coze_ws_info._exit(&coze_ws_info);
        ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL, err);
        //ai_send_event_to_sdk(AI_EVENT_WEBS_CLOSE, 1); // @NOTICE: 通知 AI 线程退出
#endif /* CONFIG_NERTC_CONNECTION */
}


static int giz_chat_pid = NULL;
void giz_chat_init(void){
    printf("\n\n***************************************giz_chat_init***************************************************\n");
    //giz_chat_start(0);

    int ret;
    ret = thread_fork("giz_chat", 15, 20 * 1024, 0, &giz_chat_pid, giz_chat_start, NULL);
    if (ret != OS_NO_ERR) {
        ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL, 0);
    }
}



void giz_chat_deinit(void){
    printf("\n\n********************************************giz_chat_deinit*****************************************\n");
    
#ifdef CONFIG_NERTC_CONNECTION
    /* NERTC 模式：停止并清理 NERTC SDK */
    log_info("[NERTC] Stopping protocol...");
    nertc_protocol_stop();
    nertc_protocol_deinit();
    ai_set_state(AI_STATE_IDLE);  /* 断开后回到 IDLE */
    log_info("[NERTC] Protocol stopped");
#else
    /* WebSocket 模式：清理 WebSocket 连接 */
    if(coze_ws_info.recv_thread_id){
        thread_kill(coze_ws_info.recv_thread_id, KILL_WAIT);
        coze_ws_info.recv_thread_id=0;
    }

    if(coze_ws_info.ping_thread_id){
        thread_kill(coze_ws_info.ping_thread_id, KILL_WAIT);
        coze_ws_info.ping_thread_id=0;
    }

    if(coze_ws_info._exit)
        coze_ws_info._exit(&coze_ws_info);
#endif /* CONFIG_NERTC_CONNECTION */
    
    /* 公共部分：清理 giz_chat 线程 */
    if(giz_chat_pid){
        thread_kill(&giz_chat_pid, KILL_FORCE);
        giz_chat_pid=NULL;
    }
}


// GENERATED-BY-CURSOR
void websocket_struct_dump(const WEBSOCKET_INFO *info){
    if (!info) {
        log_error("Null pointer in websocket_struct_dump");
        return;
    }

    log_info("\n\n===== WEBSOCKET_INFO Structure Dump =====");
    
    // 基本字段
    log_info("[Basic] sk_fd:%p lst_fd:%p mode:%d recvsub:%d", 
            info->sk_fd, info->lst_fd, info->websocket_mode, info->websocket_recvsub);
    
    // 网络配置
    log_info("[Network] Port:%u IP/URL:%s", info->port, info->ip_or_url ? (const char*)info->ip_or_url : "NULL");
    log_info("[Addr] Client:%s:%d Server:%s:%d", 
            inet_ntoa(info->clientaddr.sin_addr), ntohs(info->clientaddr.sin_port),
            inet_ntoa(info->servaddr.sin_addr), ntohs(info->servaddr.sin_port));
    
    log_info("[User-Agent] %s", info->user_agent_str);
    log_info("[Origin] %s", info->origin_str);

    // 缓冲区信息
    log_info("[Buffer] RecvBuf:%p Size:%u RecvLen:%llu Timeout:%ums",
            info->recv_buf, info->recv_buf_size, info->recv_len, info->recv_time_out);

    // 请求头信息
    const struct websocket_req_head *req = &info->req_head;
    log_info("[ReqHead] Method:%.4s File:%.256s... Host:%.32s Version:%.8s",
            req->medthod, req->file, req->host, req->version);

    // 安全相关字段（根据规则7.1部分隐藏敏感信息）
    log_info("[Security] Key:[REDACTED] SSL:%d", info->websockets_mbtls_info.ssl_fd);

    // 函数指针（根据规则4.1不打印实际地址）
    log_info("[Callbacks] Init:%p Send:%p RecvCB:%p",
            info->_init, info->_send, info->_recv_cb);

    // 状态标志
    log_info("[Status] Valid:%d Established:%d DataType:%d",
            info->websocket_valid, info->websocket_valid, info->websocket_data_type);

    log_info("===== End of Dump =====\n\n");
}

