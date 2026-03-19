#include "app_config.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "ai_coze_server.h"
#include "ai_coze_config.h"
#include "ai_device_manager.h"
#ifdef CONFIG_NERTC_CONNECTION
#include "nertc_protocol.h"
#endif

struct ai_coze_sdk_api ai_coze;

/* ============================================================================
 * AI 状态机实现
 * ============================================================================ */
static ai_state_t g_ai_state = AI_STATE_STARTING;

static const char* ai_state_names[] = {
    "STARTING",
    "IDLE", 
    "LISTENING",
    "SPEAKING"
};

ai_state_t ai_get_state(void)
{
    return g_ai_state;
}

void ai_set_state(ai_state_t state)
{
    if (state == g_ai_state) {
        return;
    }
    
    printf("[AI_STATE] %s -> %s\n", 
           ai_state_to_string(g_ai_state), 
           ai_state_to_string(state));
    
    g_ai_state = state;
}

const char* ai_state_to_string(ai_state_t state)
{
    if (state < sizeof(ai_state_names) / sizeof(ai_state_names[0])) {
        return ai_state_names[state];
    }
    return "UNKNOWN";
}


static int ai_coze_sdk_do_event(int event, int arg){
    printf("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee ai_coze_sdk_do_event:%d--%d",event,arg);
    switch (event) {
        case AI_EVENT_MQTT_REG_SUCC:   // MQTT 注册成功事件
            if(is_in_config_network_state()){
                config_network_stop();
            }
            device_status.cfg_net_rst_en = 0; // 网络重置
            gizwits_mqtt_main(); //启动mqtt
            break;
        case AI_EVENT_MQTT_GET_COZE_INFO_SUCC: // MQTT 获取 Coze 信息成功事件
            memset(&device_status, 0, sizeof(device_status_t));
            giz_chat_init();
            break;
        case AI_EVENT_COZE_WEBSOCKET_BUILD_SUCC:  // Coze WebSocket 连接建立成功事件
            printf("oooooooooooooooooooooooooooooooooooAI_EVENT_COZE_WEBSOCKET_BUILD_SUCC\n");
            ai_coze.ai_connected=1;
            ai_server_event_callbck(AI_SERVER_EVENT_CONNECTED,0);
            //dec_enc_mode_init();      
            break;
        case AI_EVENT_MQTT_REG_FAIL: // MQTT 注册失败事件
            ai_coze.ai_connected=0;
            //gizwits_login();
            ai_server_event_callbck(AI_SERVER_EVENT_DISCONNECTED,0);
            break;
        case AI_EVENT_MQTT_GET_COZE_INFO_FAIL:
        case AI_EVENT_COZE_WEBSOCKET_BUILD_FAIL:
             printf("-------------------------------------AI_EVENT_MQTT_REG_FAIL\n");
             ai_coze.ai_connected=0;
             ai_server_event_callbck(AI_SERVER_EVENT_DISCONNECTED,0);
            break;
        case AI_EVENT_MQTT_DISCONNECTED:
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!AI_EVENT_MQTT_DISCONNECTED\n");
            break;
        case AI_EVENT_RECORD_START:
            //enc_start();
            break;
        case AI_EVENT_RECORD_BREAK:
            //enc_stop(0);
            break;
        case AI_EVENT_RECORD_STOP:
            //enc_stop(1);
            break;
        case AI_EVENT_RTC_ONOFF:
            printf("--------------------AI_EVENT_RTC_ONOFF:%d",ai_coze.ai_connected);
            #if TEST_SAVE_PCM
                if(ai_coze.ai_chat_created){
                    ai_coze.ai_chat_created=0;
                    enc_stop(0);
                }else{
                    ai_coze.ai_chat_created=1;
                    enc_start();
                }
            #elif defined(CONFIG_NERTC_CONNECTION)
                /* NERTC 模式：根据当前 AI 状态执行不同操作
                 * 按钮只控制 audio channel 的开关，不控制 RTC 线程 */
                {
                    ai_state_t current_state = ai_get_state();
                    printf("[NERTC] RTC_ONOFF: state=%s, chat_created=%d\n", 
                           ai_state_to_string(current_state), ai_coze.ai_chat_created);
                    
                    switch (current_state) {
                        case AI_STATE_STARTING:
                            /* STARTING 状态：首次启动 RTC 线程 */
                            if (ai_coze.ai_connected && !ai_coze.ai_chat_created) {
                                ai_coze.ai_chat_created = 1;
                                coze_rtc_start();  /* 只调用一次，启动音频发送线程 */
                                printf("[NERTC] Start RTC thread (first time)\n");
                            }
                            break;
                            
                        case AI_STATE_IDLE:
                            /* IDLE 状态：打开音频通道 */
                            if (ai_coze.ai_connected && ai_coze.ai_chat_created) {
                                nertc_protocol_open_audio_channel();
                                printf("[NERTC] Open audio channel\n");
                            } else if (ai_coze.ai_connected && !ai_coze.ai_chat_created) {
                                /* 首次：启动线程并打开通道 */
                                ai_coze.ai_chat_created = 1;
                                coze_rtc_start();
                                nertc_protocol_open_audio_channel();
                                printf("[NERTC] Start RTC thread (first time from IDLE)\n");
                            } else {
                                printf("[NERTC] Cannot open audio channel: not connected or chat not created\n");
                            }
                            break;
                            
                        case AI_STATE_SPEAKING:
                            /* SPEAKING 状态：打断 AI */
                            printf("[NERTC] Interrupt AI (manual)\n");
                            nertc_protocol_manual_interrupt();
                            break;
                            
                        case AI_STATE_LISTENING:
                            /* LISTENING 状态：关闭音频通道（不停止线程） */
                            printf("[NERTC] Close audio channel\n");
                            nertc_protocol_close_audio_channel();
                            ai_set_state(AI_STATE_IDLE);
                            break;
                            
                        default:
                            printf("[NERTC] Unknown state: %d\n", current_state);
                            break;
                    }
                }
            #else
                /* WebSocket 模式：原有逻辑 */
                if(ai_coze.ai_connected){
                    if(ai_coze.ai_chat_created){
                        ai_coze.ai_chat_created=0;
                        //enc_stop(0);
                        coze_rtc_stop();
                    }else{
                        ai_coze.ai_chat_created=1;
                        //enc_start();
                        coze_rtc_start();
                    }
                }
            #endif
            break;
        case AI_EVENT_MEDIA_START:


            break;
        case AI_EVENT_MEDIA_STOP:


            break;
        // case AI_EVENT_MEDIA_PLAY:
        //     device_status.chat_end = 0;
        //     break;
        case AI_EVENT_WEBS_CONNECTED:
            printf("------------------------------------------------------------AI_EVENT_WEBS_CONNECTED\n");
            
            break;
        case AI_EVENT_WEBS_CLOSE:
             printf("------------------------------------------------------------AI_EVENT_WEBS_CLOSE\n");
            //  if(device_status.mic_open_en){
            //     device_status.mic_open_en = 0;
            //     void enc_stop(u8 is_stop_send);
            //     enc_stop(1);
            // }
            break;
        case AI_EVENT_MQTT_CHANGE_ROOM:
            printf("AI_EVENT_MQTT_CHANGE_ROOM\n");

            break;
        default:
            break;
    }
    return 0;
}

void ai_send_event_to_sdk(int event, int arg){
    // union ai_coze_req req = {0};
    // req.evt.event   = event;
    // req.evt.ai_name = AI_COZE_SDK_NAME;
    // req.evt.arg = arg;
    // ai_server_request(__this->ai_server, AI_REQ_EVENT, &req);
    ai_coze_sdk_do_event(event, arg);
}

void ai_server_event_callbck(int event, int arg){
    int msg[2];
    msg[0]=event;
    msg[1]=arg;
    ai_coze.handler(&ai_coze,2,msg);
}

//接收到消息管理
int ai_server_request(void *server, int req_type, union ai_coze_req *req){
    int err = 0;
    switch (req_type) {
    case AI_REQ_CONNECT:
#if CHAT_BY_MY_COZE
        ai_send_event_to_sdk(AI_EVENT_MQTT_GET_COZE_INFO_SUCC, 0);
#else
        err=gizwits_login();
#endif
        break;
    case AI_REQ_DISCONNECT:

        break;
    case AI_REQ_EVENT:
        err = ai_coze_sdk_do_event(req->evt.event, req->evt.arg);
        break;
    default:
        break;
    }

    return err;
}

void app_module_deinit(void){
    printf("-----------------------------------app_module_deinit\n");
    //关闭解码器
    //void dec_enc_mode_exit(void);
    //dec_enc_mode_exit();
    printf("-----------------------------------app_module_deinit1\n");
    // // 退出coze
    giz_chat_deinit(0);
    printf("-----------------------------------app_module_deinit3\n");
    // 退出mqtt
    mqtt_task_kill();
    printf("-----------------------------------app_module_deinit4\n");
    //device_status.cfg_net_rst_en = 1;
    //device_status.room_change = 1;
}



void *ai_coze_server_open(void *name, void *priv){
    return &ai_coze;
}


void ai_coze_server_close(void *name){
    printf("---->%s:%d",__func__,__LINE__);
    ai_coze.ai_connected=0;
}

void ai_coze_server_register_event_handler(struct ai_coze_sdk_api *server, void *priv,
                                   void (*handler)(void *, int argc, int *argv)){
    server->handler=handler;
}


