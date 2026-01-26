/**
  ******************************************************************************
  * @file   wheather.c
  * @author Sifli software development team
  ******************************************************************************
*/
/**
 * @attention
 * Copyright (c) 2024 - 2025,  Sifli Technology
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit
 *    in a product or a software update for such product, must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Sifli integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <rtthread.h>
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/dns.h"
#include "lwip/apps/websocket_client.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/apps/mqtt.h"
#include "xiaozhi.h"
#include "xiaozhi2.h"
#include "bf0_hal.h"
#include "bts2_global.h"
#include "bts2_app_pan.h"
#include <cJSON.h>
#include "button.h"
#include "audio_server.h"
#include <webclient.h>
#include "bt_env.h"
#include "./iot/iot_c_api.h"
#ifdef BSP_USING_PM
    #include "gui_app_pm.h"
#endif // BSP_USING_PM
#include "nertc_sdk.h"


nertc_sdk_engine_t engine_ = NULL;

#define MAX_WSOCK_HDR_LEN 4096
extern void xiaozhi_ui_update_ble(char *string);
extern void xiaozhi_ui_chat_status(char *string);
extern void xiaozhi_ui_chat_output(char *string);
extern void xiaozhi_ui_update_emoji(char *string);
extern void xz_audio_downlink(uint8_t *data, uint32_t size, uint32_t *aes_value, uint8_t need_aes);

// IoT 模块相关
extern void iot_initialize();                      // 初始化 IoT 模块
extern void iot_invoke(const uint8_t* data, uint16_t len);  // 执行远程命令
extern const char* iot_get_descriptors_json();     // 获取设备描述
extern const char* iot_get_states_json();          // 获取设备状态

extern void xz_button1_pressed();

xiaozhi_context_t g_xz_context;
xiaozhi_ws_t g_xz_ws;
char mac_address_string[20];
char client_id_string[40];
enum DeviceState g_state;
static char message[256];
char client_id_string[40];
ALIGN(4) uint8_t g_sha256_result[32] = {0};
extern BOOL g_pan_connected;
const char *mode_str[] =
{
    "auto",
    "manual",
    "realtime"
};

static const char *hello_message = "{"
                                   "\"type\":\"hello\","
                                   "\"version\": 3,"
                                   "\"transport\":\"websocket\","
                                   "\"audio_params\":{"
                                   "\"format\":\"opus\", \"sample_rate\":16000, \"channels\":1, \"frame_duration\":60"
                                   "}}";

static const char *ota_version = "{\r\n "
                                 "\"version\": 2,\r\n"
                                 "\"flash_size\": 4194304,\r\n"
                                 "\"psram_size\": 0,\r\n"
                                 "\"minimum_free_heap_size\": 123456,\r\n"
                                 "\"mac_address\": \"%s\",\r\n"
                                 "\"uuid\": \"%s\",\r\n"
                                 "\"chip_model_name\": \"sf32lb563\",\r\n"
                                 "\"chip_info\": {\r\n"
                                 "    \"model\": 1,\r\n"
                                 "    \"cores\": 2,\r\n"
                                 "    \"revision\": 0,\r\n"
                                 "    \"features\": 0\r\n"
                                 "},\r\n"
                                 "\"application\": {\r\n"
                                 "    \"name\": \"my-app\",\r\n"
                                 "    \"version\": \"1.0.0\",\r\n"
                                 "    \"compile_time\": \"2021-01-01T00:00:00Z\",\r\n"
                                 "    \"idf_version\": \"4.2-dev\",\r\n"
                                 "    \"elf_sha256\": \"\"\r\n"
                                 "},\r\n"
                                 "\"partition_table\": [\r\n"
                                 "    {\r\n"
                                 "        \"label\": \"app\",\r\n"
                                 "        \"type\": 1,\r\n"
                                 "        \"subtype\": 2,\r\n"
                                 "        \"address\": 10000,\r\n"
                                 "        \"size\": 100000\r\n"
                                 "    }\r\n"
                                 "],\r\n"
                                 "\"ota\": {\r\n"
                                 "    \"label\": \"ota_0\"\r\n"
                                 "},\r\n"
                                 "\"board\": {\r\n"
                                 "    \"type\":\"hdk563\",\r\n"
                                 "    \"mac\": \"%s\"\r\n"
                                 "}\r\n"
                                 "}\r\n"
                                 ;

/**
 * @brief Do hash , Single calculation, polling mode.
 * @param algo HASH Algorithm type.
 * @param raw_data Input data.
 * @param raw_data_len Input data len.
 * @param result Output data.
 * @param result_len Output data len.
 *
 * @retval none
 */
void hash_run(uint8_t algo, uint8_t *raw_data, uint32_t raw_data_len,
    uint8_t *result, uint32_t result_len)
{
/* Rest hash block. */
HAL_HASH_reset();
/* Initialize AES Hash hardware block. */
HAL_HASH_init(NULL, algo, 0);
/* Do hash. HAL_HASH_run will block until hash finish. */
HAL_HASH_run(raw_data, raw_data_len, 1);
/* Get hash result. */
HAL_HASH_result(result);
}
void hex_2_asc(uint8_t n, char *str)
{
    uint8_t i=(n>>4);
    if (i>=10)
        *str= i+'a'-10;
    else
        *str= i+'0';
    str++, i=n&0xf;
    if (i>=10)
        *str= i+'a'-10;
    else
        *str= i+'0';
}

char *get_client_id()
{
    if (client_id_string[0] == '\0') {
        int i,j=0;
        BTS2S_ETHER_ADDR   addr = bt_pan_get_mac_address(NULL);
        hash_run(HASH_ALGO_SHA256, (uint8_t*)&addr, sizeof(addr), g_sha256_result, sizeof(g_sha256_result));
        for (i=0;i<16;i++,j+=2) {
            //12345678-1234-1234-1234-123456789012
            if (i==4||i==6||i==8||i==10) {
                client_id_string[j++]='-';
            }
            hex_2_asc(g_sha256_result[i],&client_id_string[j]);
        }
        rt_kprintf(client_id_string);
    }
    return (&(client_id_string[0]));
}

char *get_mac_address()
{
    if (mac_address_string[0] == '\0')
    {
        BTS2S_ETHER_ADDR   addr = bt_pan_get_mac_address(NULL);
        uint8_t *p = (uint8_t *) & (addr);

        rt_snprintf((char *)mac_address_string, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
                    *p, *(p + 1), *(p + 2), *(p + 3), *(p + 4), *(p + 5));
    }
    return (&(mac_address_string[0]));
}

void parse_helLo(const u8_t *data, u16_t len);

void send_iot_descriptors(void) {
    const char *desc = iot_get_descriptors_json();
    if (desc == NULL) {
        rt_kprintf("Failed to get IoT descriptors\n");
        return;
    }

    char msg[512];
    snprintf(msg, sizeof(msg),
             "{\"session_id\":\"%s\",\"type\":\"iot\",\"update\":true,\"descriptors\":%s}",
             g_xz_ws.session_id, desc);

    rt_kprintf("Sending IoT descriptors:\n");
    rt_kputs(msg);
    rt_kprintf("\n");
    if (g_xz_ws.is_connected == 1) 
    {
        wsock_write(&g_xz_ws.clnt, msg, strlen(msg), OPCODE_TEXT);
    }
    else
    {
        rt_kprintf("websocket is not connected\n");
    }
}

void send_iot_states(void) {
    const char *state = iot_get_states_json();
    if (state == NULL) {
        rt_kprintf("Failed to get IoT states\n");
        return;
    }

    char msg[512];
    snprintf(msg, sizeof(msg),
             "{\"session_id\":\"%s\",\"type\":\"iot\",\"update\":true,\"states\":%s}",
             g_xz_ws.session_id, state);

    rt_kprintf("Sending IoT states:\n");
    rt_kputs(msg);
    rt_kprintf("\n");
    if (g_xz_ws.is_connected == 1) 
    {
        wsock_write(&g_xz_ws.clnt, msg, strlen(msg), OPCODE_TEXT);
    }
    else
    {
        rt_kprintf("websocket is not connected\n");
    }
}



void ws_send_speak_abort(void *ws, char *session_id, int reason)
{
    rt_kprintf("speak abort\n");
    rt_snprintf(message, 256, "{\"session_id\":\"%s\",\"type\":\"abort\"",
                session_id);
    if(reason)
        strcat(message, ",\"reason\":\"wake_word_detected\"}");
    else
        strcat(message, "}");

    if (g_xz_ws.is_connected == 1)
    {
        wsock_write((wsock_state_t *)ws, message, strlen(message), OPCODE_TEXT);
    }
    else
    {
        rt_kprintf("websocket is not connected\n");
    }
}



void ws_send_listen_start(void *ws, char *session_id, enum ListeningMode mode)
{
    rt_kprintf("listen start\n");
    rt_snprintf(message, 256, "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"start\",\"mode\":\"%s\"}",
                session_id, mode_str[mode]);
    //rt_kputs("\r\n");
    //rt_kputs(message);
    //rt_kputs("\r\n");
    if (g_xz_ws.is_connected == 1)
    {
        wsock_write((wsock_state_t *)ws, message, strlen(message), OPCODE_TEXT);
    }
    else
    {
        rt_kprintf("websocket is not connected\n");
    }

}

void ws_send_listen_stop(void *ws, char *session_id)
{
    rt_kprintf("listen stop\n");
    rt_snprintf(message, 256, "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"stop\"}",
                session_id);
    if (g_xz_ws.is_connected == 1)
    {
        wsock_write((wsock_state_t *)ws, message, strlen(message), OPCODE_TEXT);
    }
    else
    {
        rt_kprintf("websocket is not connected\n");
    }
}
void ws_send_hello(void *ws)
{
    if (g_xz_ws.is_connected == 1)
    {
        wsock_write((wsock_state_t *)ws, hello_message, strlen(hello_message), OPCODE_TEXT);
    }
    else
    {
        rt_kprintf("websocket is not connected\n");
    }
}
void xz_audio_send_using_websocket(uint8_t *data, int len)
{
    if (g_xz_ws.is_connected == 1)
    {
        err_t err = wsock_write(&g_xz_ws.clnt, data, len, OPCODE_BINARY);
        //rt_kprintf("send audio = %d len=%d\n", err, len);
    }
    else
        rt_kprintf("Websocket disconnected\n");
}

err_t my_wsapp_fn(int code, char *buf, size_t len)
{
    if (code == WS_CONNECT)
    {
        rt_kprintf("websocket connected\n");
        int status = (uint16_t)(uint32_t)buf;
        if (status == 101)  // wss setup success
        {
            rt_sem_release(g_xz_ws.sem);
            g_xz_ws.is_connected = 1;
        }
    }
    else if (code == WS_DISCONNECT)
    {
        if(!g_xz_ws.is_connected)
        {
            rt_sem_release(g_xz_ws.sem);
        }
        else
        {
            xiaozhi_ui_chat_status("休眠中...");
            xiaozhi_ui_chat_output("请按键唤醒");
            xiaozhi_ui_update_emoji("sleepy");
        }
        rt_kprintf("WebSocket closed\n");
        g_xz_ws.is_connected = 0;

    }
    else if (code == WS_TEXT)
    {
        parse_helLo(buf, len);
    }
    else
    {
        // Receive Audio Data
        xz_audio_downlink(buf, len, NULL, 0);
    }
    return 0;
}
void xiaozhi2(int argc, char **argv);
void reconnect_websocket() {

    if (!g_pan_connected) {
        xiaozhi_ui_chat_status("请开启网络共享");
        xiaozhi_ui_chat_output("请在手机上开启网络共享后重新发起连接");
        xiaozhi_ui_update_emoji("embarrassed");
        return;
    }
    // Check if the websocket is already connected
    if (g_xz_ws.clnt.pcb != NULL && g_xz_ws.clnt.pcb->state != CLOSED) {
        rt_kprintf("WebSocket is not in CLOSED state, cannot reconnect\n");
        return;
    }
    xiaozhi_ui_chat_status("唤醒中...");
    xiaozhi_ui_chat_output("正在唤醒小智...");
    xiaozhi_ui_update_emoji("neutral");

    err_t result;
    uint32_t retry = 10;
    while (retry-- > 0)
    {

        if (g_xz_ws.sem == NULL)
            g_xz_ws.sem = rt_sem_create("xz_ws", 0, RT_IPC_FLAG_FIFO);
        char *Client_Id = get_client_id();
        wsock_init(&g_xz_ws.clnt, 1, 1, my_wsapp_fn);//初始化websocket,注册回调函数
        result = wsock_connect(&g_xz_ws.clnt, MAX_WSOCK_HDR_LEN, XIAOZHI_HOST, XIAOZHI_WSPATH,
                            LWIP_IANA_PORT_HTTPS, XIAOZHI_TOKEN, NULL,
                            "Protocol-Version: 1\r\nDevice-Id: %s\r\nClient-Id: %s\r\n", get_mac_address(),Client_Id);
        rt_kprintf("Web socket connection %d\r\n", result);
        if (result == 0)
        {
            rt_kprintf("result_g_xz_ws.sem = 0%d\n",g_xz_ws.sem->value);

            if (RT_EOK == rt_sem_take(g_xz_ws.sem, 50000))
            {
                rt_kprintf("g_xz_ws.is_connected = %d\n", g_xz_ws.is_connected);
                if (g_xz_ws.is_connected)
                {
                    result = wsock_write(&g_xz_ws.clnt, hello_message, strlen(hello_message), OPCODE_TEXT);
                    rt_kprintf("Web socket write %d\r\n", result);
                    break;
                }
                else
                {
                    xiaozhi_ui_chat_status("唤醒中...");
                    xiaozhi_ui_chat_output("唤醒小智失败,请重试！");
                    xiaozhi_ui_update_emoji("neutral");
                    rt_kprintf("result = wsock_write_Web socket disconnected\r\n");
                }
            }
            else
            {
                xiaozhi_ui_chat_output("连接超时,请重试！");
                rt_kprintf("Web socket connected timeout\r\n");
            }
        }
        else
        {
            rt_kprintf("Waiting reconnect ready(%d)... \r\n", retry);
            rt_thread_mdelay(1000);
        }
    }
}
extern rt_mailbox_t g_bt_app_mb;
#define WEBSOCKET_RECONNECT 3
static void xz_button_event_handler(int32_t pin, button_action_t action)//Session key
{
    rt_kprintf("button(%d) %d:", pin, action);
    rt_kprintf("g_state = %d\n", g_state);

    static button_action_t last_action=BUTTON_RELEASED;
    if(last_action==action)
    {
        return;
    }
    last_action=action;

#if defined(NERTC_AI)
    if (action == BUTTON_PRESSED)
    {
        rt_kprintf("pressed\r\n");
        xz_button1_pressed();
    }
    else if (action == BUTTON_RELEASED)
    {
        rt_kprintf("released\r\n");
    }
#else
    if (!g_xz_ws.is_connected)//唤醒  stop ? goodbye
    {
#ifdef BSP_USING_PM
        gui_pm_fsm(GUI_PM_ACTION_WAKEUP);
#endif // BSP_USING_PM
        rt_kprintf("please button11 attempting to reconnect WebSocket\n\r\n");
        xiaozhi_ui_chat_status("请按唤醒键...");
        xiaozhi_ui_chat_output("请按唤醒键重连！");
        xiaozhi_ui_update_emoji("embarrassed");

    }
    else
    {
        if (action == BUTTON_PRESSED)
        {
#ifdef BSP_USING_PM
            gui_pm_fsm(GUI_PM_ACTION_WAKEUP);
#endif // BSP_USING_PM
            rt_kprintf("pressed\r\n");
            if(g_state == kDeviceStateSpeaking)
            {
                rt_kprintf("speaking, abort\n");
                ws_send_speak_abort(&g_xz_ws.clnt, g_xz_ws.session_id,kAbortReasonWakeWordDetected);//发送停止说话
                xz_speaker(0);//关闭扬声器
            }
            ws_send_listen_start(&g_xz_ws.clnt, g_xz_ws.session_id, kListeningModeManualStop);//发送开始监听
            xiaozhi_ui_chat_status("聆听中...");
            xz_mic(1);
        }
        else if (action == BUTTON_RELEASED)
        {
#ifdef BSP_USING_PM
            gui_pm_fsm(GUI_PM_ACTION_WAKEUP);
#endif // BSP_USING_PM
            rt_kprintf("released\r\n");
            xiaozhi_ui_chat_status("待命中...");
            ws_send_listen_stop(&g_xz_ws.clnt, g_xz_ws.session_id);
            xz_mic(0);
        }
    }
#endif // defined(NERTC_AI)
}
#if PKG_XIAOZHI_USING_AEC
void simulate_button_pressed()
{
    xz_button_event_handler(BSP_KEY2_PIN, BUTTON_PRESSED);
}
void simulate_button_released()
{
    xz_button_event_handler(BSP_KEY2_PIN, BUTTON_RELEASED);
}
#endif
static void xz_button_init(void)//Session key
{
    static int initialized = 0;

    if (initialized == 0)
    {
        button_cfg_t cfg;
        cfg.pin = BSP_KEY2_PIN;

        cfg.active_state = BSP_KEY2_ACTIVE_HIGH;
        cfg.mode = PIN_MODE_INPUT;
        cfg.button_handler = xz_button_event_handler;//Session key
        int32_t id = button_init(&cfg);
        RT_ASSERT(id >= 0);
        RT_ASSERT(SF_EOK == button_enable(id));
        initialized = 1;
    }
}




void xz_ws_audio_init()
{
    rt_kprintf("xz_audio_init\n");
    rt_kprintf("exit sniff mode\n");
    bt_interface_exit_sniff_mode((unsigned char*)&g_bt_app_env.bd_addr);//exit sniff mode
    bt_interface_wr_link_policy_setting((unsigned char*)&g_bt_app_env.bd_addr, BT_NOTIFY_LINK_POLICY_ROLE_SWITCH);//close role switch
    audio_server_set_private_volume(AUDIO_TYPE_LOCAL_MUSIC, 12);//设置音量
    xz_audio_decoder_encoder_open(1);//打开音频解码器和编码器
    xz_button_init();
}


static char *my_json_string(cJSON *json, char *key)
{
    cJSON *item  = cJSON_GetObjectItem(json, key);
    char *r = cJSON_Print(item);

    if (r && ((*r) == '\"'))
    {
        r++;
        r[strlen(r) - 1] = '\0';
    }
    return r;
}

void parse_helLo(const u8_t *data, u16_t len)
{
    cJSON *item = NULL;
    cJSON *root = NULL;
    rt_kputs(data);
    rt_kputs("--\r\n");
    root = cJSON_Parse(data);   /*json_data 为MQTT的原始数据*/
    if (!root)
    {
        rt_kprintf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return;
    }

    char *type = cJSON_GetObjectItem(root, "type")->valuestring;
    rt_kprintf("type = %s\n", type);
    if (strcmp(type, "hello") == 0)
    {
        char *session_id = cJSON_GetObjectItem(root, "session_id")->valuestring;
        rt_kprintf("session_id = %s\n", session_id);
        cJSON *audio_param = cJSON_GetObjectItem(root, "audio_params");
        char *sample_rate = cJSON_GetObjectItem(audio_param, "sample_rate")->valuestring;
        char *duration = cJSON_GetObjectItem(audio_param, "duration")->valuestring;
        g_xz_ws.sample_rate = atoi(sample_rate);
        g_xz_ws.frame_duration = atoi(duration);
        strncpy(g_xz_ws.session_id, session_id, 9);
        g_state = kDeviceStateIdle;
        xz_ws_audio_init();//初始化音频
        send_iot_descriptors();//发送iot描述
        send_iot_states();//发送iot状态

        xiaozhi_ui_chat_status("待命中...");
        xiaozhi_ui_chat_output("小智已连接!");
        xiaozhi_ui_update_emoji("neutral");

    }
    else if (strcmp(type, "goodbye") == 0)
    {
        g_state = kDeviceStateUnknown;
        rt_kprintf("session ended\n");
        xiaozhi_ui_chat_status("睡眠中...");
        xiaozhi_ui_chat_output("等待唤醒...");
        xiaozhi_ui_update_emoji("sleep");
    }
    else if(strcmp(type,"stt") == 0)
    {
        char *txt = cJSON_GetObjectItem(root, "text")->valuestring;
        xiaozhi_ui_chat_output(txt);
    }
    else if (strcmp(type, "tts") == 0)
    {
        char *txt = cJSON_GetObjectItem(root, "text")->valuestring;
        rt_kputs(txt);
        rt_kputs("--\r\n");



        char *state = cJSON_GetObjectItem(root, "state")->valuestring;


        if (strcmp(state, "start") == 0)
        {
            if (g_state == kDeviceStateIdle || g_state == kDeviceStateListening)
            {
                g_state = kDeviceStateSpeaking;
                xz_speaker(1);//打开扬声器
                xiaozhi_ui_chat_status("讲话中...");
            }
        }
        else if (strcmp(state, "stop") == 0)
        {
            g_state = kDeviceStateIdle;
            xz_speaker(0);//关闭扬声器
            xiaozhi_ui_chat_status("待命中...");

        }
        else if (strcmp(state, "sentence_start") == 0)
        {
            char *txt = cJSON_GetObjectItem(root, "text")->valuestring;
            // rt_kputs(txt);
            xiaozhi_ui_chat_output(txt);

        }

    }
    else if (strcmp(type, "llm") == 0)
    {
        rt_kputs(cJSON_GetObjectItem(root, "emotion")->valuestring);
        xiaozhi_ui_update_emoji(cJSON_GetObjectItem(root, "emotion")->valuestring);
        
    }
    else if (strcmp(type, "iot") == 0) 
    {
        rt_kprintf("iot command\n");
        cJSON *commands = cJSON_GetObjectItem(root, "commands");
        // rt_kprintf("commands: %s\n", cJSON_Print(commands));
        for (int i = 0; i < cJSON_GetArraySize(commands); i++) 
        {   
            // rt_kprintf("command %d: %s\n", i, cJSON_Print(cJSON_GetArrayItem(commands, i)));
            cJSON *cmd = cJSON_GetArrayItem(commands, i);
            // rt_kprintf("cmd: %s\n", cJSON_Print(cmd));
            char *cmd_str = cJSON_PrintUnformatted(cmd);
            // rt_kprintf("cmd_str: %s\n", cmd_str);
            if (cmd_str) 
            {
                iot_invoke((uint8_t *)cmd_str, strlen(cmd_str));
                send_iot_states();// 发送 IoT 状态
                rt_free(cmd_str);
            }
        }
    }   
    else
    {
        rt_kprintf("Unkown type: %s\n", type);
    }

    cJSON_Delete(root);/*每次调用cJSON_Parse函数后，都要释放内存*/
}

static void svr_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    if (ipaddr != NULL)
    {
        rt_kprintf("DNS lookup succeeded, %s IP: %s\n", name, ipaddr_ntoa(ipaddr));
    }
}

static int check_internet_access()
{
    int r = 0;
    const char *hostname = XIAOZHI_HOST;
    ip_addr_t addr = {0};

    {
        err_t err = dns_gethostbyname(hostname, &addr, svr_found_callback, NULL);
        if (err != ERR_OK && err != ERR_INPROGRESS)
        {
            rt_kprintf("Coud not find %s, please check PAN connection\n", hostname);
        }
        else
            r = 1;
    }

    return r;
}
extern BOOL first_pan_connected;
char *get_xiaozhi()
{
    char *buffer = RT_NULL;
    int resp_status;
    struct webclient_session *session = RT_NULL;
    char *xiaozhi_url = RT_NULL;
    int content_length = -1, bytes_read = 0;
    int content_pos = 0;

    if (check_internet_access() == 0)
        return buffer;

    if (check_internet_access() == 1)
        first_pan_connected = TRUE;

    int size = strlen(ota_version) + sizeof(client_id_string) + sizeof(mac_address_string) * 2 + 16;
    char *ota_formatted = rt_malloc(size);
    if (!ota_formatted)
    {
        goto __exit;
    }
    rt_snprintf(ota_formatted, size, ota_version, get_mac_address(), get_client_id(), get_mac_address());

    /* 为 weather_url 分配空间 */
    xiaozhi_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (xiaozhi_url == RT_NULL)
    {
        rt_kprintf("No memory for xiaozhi_url!\n");
        goto __exit;
    }
    /* 拼接 GET 网址 */
    rt_snprintf(xiaozhi_url, GET_URL_LEN_MAX, GET_URI, XIAOZHI_HOST);

    /* 创建会话并且设置响应的大小 */
    session = webclient_session_create(GET_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }

    webclient_header_fields_add(session, "Device-Id: %s \r\n", get_mac_address());
    webclient_header_fields_add(session, "Client-Id: %s \r\n", get_client_id());
    webclient_header_fields_add(session, "Content-Type: application/json \r\n");
    webclient_header_fields_add(session, "Content-length: %d \r\n", strlen(ota_formatted));
    //webclient_header_fields_add(session, "X-language:");

    /* 发送 GET 请求使用默认的头部 */
    if ((resp_status = webclient_post(session, xiaozhi_url, ota_formatted, strlen(ota_formatted))) != 200)
    {
        rt_kprintf("webclient Post request failed, response(%d) error.\n", resp_status);
        //goto __exit;
    }

    /* 分配用于存放接收数据的缓冲 */
    buffer = rt_calloc(1, GET_RESP_BUFSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("No memory for data receive buffer!\n");
        goto __exit;
    }

    content_length = webclient_content_length_get(session);
    if (content_length > 0)
    {
        do
        {
            bytes_read = webclient_read(session, buffer + content_pos,
                                        content_length - content_pos > GET_RESP_BUFSZ ?
                                        GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }
            content_pos += bytes_read;
        }
        while (content_pos < content_length);
    }
    else
    {
        rt_free(buffer);
        buffer = NULL;
    }
__exit:
    /* 释放网址空间 */
    if (xiaozhi_url != RT_NULL)
    {
        rt_free(xiaozhi_url);
        xiaozhi_url = RT_NULL;
    }

    /* 关闭会话 */
    if (session != RT_NULL)
    {
        LOCK_TCPIP_CORE();
        webclient_close(session);
        UNLOCK_TCPIP_CORE();
    }
    if (ota_formatted)
    {
        rt_free(ota_formatted);
    }
    return buffer;
}

void xiaozhi_ws_connect(void)
{
    if (!g_pan_connected) {
        xiaozhi_ui_chat_status("请开启网络共享");
        xiaozhi_ui_chat_output("请在手机上开启网络共享后重新发起连接");
        xiaozhi_ui_update_emoji("embarrassed");
        return;
    }
     // 检查 WebSocket 的 TCP 控制块状态是否为 CLOSED
     if (g_xz_ws.clnt.pcb != NULL && g_xz_ws.clnt.pcb->state != CLOSED) {
        rt_kprintf("WebSocket is not in CLOSED state, cannot reconnect\n");
        return;
    }
    err_t err;
    uint32_t retry = 10;
    while (retry-- > 0)
    {

        if (g_xz_ws.sem == NULL)
            g_xz_ws.sem = rt_sem_create("xz_ws", 0, RT_IPC_FLAG_FIFO);


        wsock_init(&g_xz_ws.clnt, 1, 1, my_wsapp_fn);//初始化websocket,注册回调函数
        char *Client_Id = get_client_id();
        err = wsock_connect(&g_xz_ws.clnt, MAX_WSOCK_HDR_LEN, XIAOZHI_HOST, XIAOZHI_WSPATH,
                            LWIP_IANA_PORT_HTTPS, XIAOZHI_TOKEN, NULL,
                            "Protocol-Version: 1\r\nDevice-Id: %s\r\nClient-Id: %s\r\n", get_mac_address(),Client_Id);
        rt_kprintf("Web socket connection %d\r\n", err);
        if (err == 0)
        {
            rt_kprintf("err = 0\n");
            if (RT_EOK == rt_sem_take(g_xz_ws.sem, 50000))
            {
                rt_kprintf("g_xz_ws.is_connected = %d\n", g_xz_ws.is_connected);
                if (g_xz_ws.is_connected)
                {
                    err = wsock_write(&g_xz_ws.clnt, hello_message, strlen(hello_message), OPCODE_TEXT);
                    rt_kprintf("Web socket write %d\r\n", err);
                    break;
                }
                else
                {
                    rt_kprintf("err = wsock_write_Web socket disconnected\r\n");
                }
            }
            else
            {
                rt_kprintf("Web socket connected timeout\r\n");
            }
        }
        else
        {
            rt_kprintf("Waiting ws_connect ready%d... \r\n", retry);
            rt_thread_mdelay(1000);
        }
    }
}


int http_xiaozhi_data_parse(char *json_data)
{
    uint8_t i, j;
    uint8_t result_array_size = 0;
    char *endpoint;
    char *client_id;
    char *username;
    char *password;
    char *publish_topic;
    char *session;
    cJSON *item = NULL;
    cJSON *root = NULL;

    rt_kputs(json_data);
    root = cJSON_Parse(json_data);   /*json_data 为MQTT的原始数据*/
    if (!root)
    {
        rt_kprintf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return  -1;
    }


    cJSON *Presult = cJSON_GetObjectItem(root, "mqtt");  /*mqtt的键值对为数组，*/
    result_array_size = cJSON_GetArraySize(Presult);  /*求results键值对数组中有多少个元素*/
    item = cJSON_GetObjectItem(Presult, "endpoint");
    endpoint = cJSON_Print(item);
    item = cJSON_GetObjectItem(Presult, "client_id");
    client_id = cJSON_Print(item);
    item = cJSON_GetObjectItem(Presult, "username");
    username = cJSON_Print(item);
    item = cJSON_GetObjectItem(Presult, "password");
    password = cJSON_Print(item);
    item = cJSON_GetObjectItem(Presult, "publish_topic");
    publish_topic = cJSON_Print(item);

    // Skip the "..." in string
    endpoint++;
    endpoint[strlen(endpoint) - 1] = '\0';
    client_id++;
    client_id[strlen(client_id) - 1] = '\0';
    username++;
    username[strlen(username) - 1] = '\0';
    password++;
    password[strlen(password) - 1] = '\0';
    publish_topic++;
    publish_topic[strlen(publish_topic) - 1] = '\0';

    rt_kprintf("\r\nmqtt:\r\n\t%s\r\n\t%s\r\n\r\n", endpoint, client_id);
    rt_kprintf("\t%s\r\n\t%s\r\n", username, password);
    rt_kprintf("\t%s\r\n", publish_topic);
    xiaozhi_ws_connect();
    cJSON_Delete(root);/*每次调用cJSON_Parse函数后，都要释放内存*/
    return  0;
}



void xiaozhi2(int argc, char **argv)
{
    char *my_ota_version;
    uint32_t retry = 10;

    if (!g_pan_connected) {
        xiaozhi_ui_chat_status("请开启网络共享");
        xiaozhi_ui_chat_output("请在手机上开启网络共享后重新发起连接");
        xiaozhi_ui_update_emoji("embarrassed");
        return;
    }

    while (retry-- > 0)
    {
        xiaozhi_ui_chat_output("正在网络准备...");
        my_ota_version = get_xiaozhi();
        if (my_ota_version)
        {
            rt_kprintf("my_ota_version = %s\n", my_ota_version);
            http_xiaozhi_data_parse(my_ota_version);
            rt_free(my_ota_version);
            break;
        }
        else
        {
            rt_kprintf("Waiting internet ready(%d)... \r\n", retry);
            xiaozhi_ui_chat_status("等待网络...");
            xiaozhi_ui_chat_output("等待网络重新准备...");
            rt_thread_mdelay(1000);
        }
    }
    // xiaozhi_ui_chat_output("网络失败请重试...");
    if (!my_ota_version)
    {
        xiaozhi_ui_chat_output("请检查网络连接后重试");
        return;
    }
}

void OnError(const nertc_sdk_callback_context_t* ctx, int code, const char* msg) {
    rt_kprintf("NERtc OnError: %d, %s\r\n", code, msg);
}

nertc_sdk_channel_state_e nertc_status = NERTC_SDK_CHANNEL_STATE_JOINED;

void OnChannelStatusChanged(const nertc_sdk_callback_context_t* ctx, nertc_sdk_channel_state_e status, const char *msg) {
    rt_kprintf("NERtc OnChannelStatusChanged: %d, %s\r\n", (int)status, msg);
    nertc_status = status;
}

void nertc_audio_send(uint8_t *data, int len);
void OnJoin(const nertc_sdk_callback_context_t* ctx, uint64_t cid, uint64_t uid, nertc_sdk_error_code_e code, uint64_t elapsed, const nertc_sdk_recommended_config_t* recommended_config) {
    rt_kprintf("NERtc OnJoin: cid:%lld, uid:%lld, code:%d, elapsed:%lld\r\n", 
                cid, uid, code, elapsed);
    if (code == 0) {
        xiaozhi_ui_chat_status("rtc连接成功");
        nertc_start_ai(engine_);
        nertc_sdk_asr_caption_config_t asr_config;
        memset(&asr_config, 0, sizeof(asr_config));
        nertc_start_asr_caption(engine_, &asr_config);
        xz_speaker(1);
        xz_mic(1);
        // char a = 0;
        // nertc_audio_send(&a, 1);
    } else {
        xiaozhi_ui_chat_status("rtc连接失败");
        char buffer[16];
        rt_sprintf(buffer, "%d", (int)code);
        xiaozhi_ui_chat_output(buffer);
    }
}

void OnUserJoined(const nertc_sdk_callback_context_t* ctx, const nertc_sdk_user_info_t* user) {
    rt_kprintf("NERtc OnUserJoined: %lld, %s type:%d\r\n", user->uid, user->name, user->type);
}

void OnUserLeft(const nertc_sdk_callback_context_t* ctx, const nertc_sdk_user_info_t* user, int reason) {
    rt_kprintf("NERtc OnUserLeft: %lld, %d\r\n", user->uid, reason);
}

void OnUserAudioStart(const nertc_sdk_callback_context_t* ctx, uint64_t uid, nertc_sdk_media_stream_e stream_type) {
    rt_kprintf("NERtc OnUserAudioStart: %lld, %d\r\n", uid, stream_type);
}

void OnUserAudioStop(const nertc_sdk_callback_context_t* ctx, uint64_t uid, nertc_sdk_media_stream_e stream_type) {
    rt_kprintf("NERtc OnUserAudioStop: %lld, %d\r\n", uid, stream_type);
}

void OnAsrCaptionResult(const nertc_sdk_callback_context_t* ctx, nertc_sdk_asr_caption_result_t* results, int result_count) {
    rt_kprintf("NERtc OnAsrCaptionResult: result_count %d\r\n", result_count);
    const int max_len = 100;
    for (size_t i = 0; i < result_count; i++)
    {
        if (i == 0) {
            xiaozhi_ui_chat_output(results[i].content);
        }
        int len = strlen(results[i].content);
        rt_kprintf("NERtc OnAsr:%lld,%d,content:", results[i].user_id, len);
        for (int j = 0; j < len; j+=max_len) {
            rt_kprintf("%.*s", max_len, results[i].content + j);
        }
        rt_kprintf("\r\n");
    }
    rt_kprintf("NERtc OnAsrCaptionResult end\r\n");
}

void OnAiData(const nertc_sdk_callback_context_t* ctx, nertc_sdk_ai_data_result_t* ai_data) {
    if (!ai_data) 
        return;

    rt_kprintf("NERtc OnAiData type:%s data:", ai_data->type);
    int len = strlen(ai_data->data);
    const int max_len = 100;
    for (int j = 0; j < len; j+=max_len) {
        rt_kprintf("%.*s", max_len, ai_data->data + j);
    }
    rt_kprintf("\r\n");
    if (strcmp(ai_data->data, "audio.agent.speech_started") == 0) {
        xiaozhi_ui_chat_status("讲话中");
    } else if (strcmp(ai_data->data, "audio.agent.speech_stopped") == 0) {
        xiaozhi_ui_chat_status("聆听中");
    }
}

void OnAudioData(const nertc_sdk_callback_context_t* ctx, uint64_t uid, nertc_sdk_media_stream_e stream_type, nertc_sdk_audio_encoded_frame_t* encoded_frame, bool is_mute_packet) {
    // static bool empty = true;
    // static int count = 0;
    // if (empty != (encoded_frame->length==0)) {
    //     empty = encoded_frame->length==0;
    //     rt_kprintf("NERtc OnAudioData new length:%d, pre_count:%d\r\n", encoded_frame->length, count);
    //     count = 0;
    // }
    // count++;
    if (nertc_status == NERTC_SDK_CHANNEL_STATE_JOINED) {
        xz_audio_downlink(encoded_frame->data, encoded_frame->length, NULL, 0);
    }
}
static int check_nrtc_internet_access()
{
    int r = 0;
    const char *hostname = "nrtc.netease.im";
    ip_addr_t addr = {0};

    {
        err_t err = dns_gethostbyname(hostname, &addr, svr_found_callback, NULL);
        if (err != ERR_OK && err != ERR_INPROGRESS)
        {
            rt_kprintf("Coud not find %s, please check PAN connection\n", hostname);
        }
        else
            r = 1;
    }

    return r;
}
static int nertc_frame_duration_in = 20;

void tick_to_string(char *buffer, size_t buffer_size)
{
    // 获取当前的 Tick 计数值
    rt_tick_t tick_count = rt_tick_get();
    snprintf(buffer, buffer_size, "%lu", (unsigned long)tick_count);
}
void random_to_string(char *buffer, size_t buffer_size, int min, int max)
{
    int random_number = rand() % (max - min + 1) + min;
    snprintf(buffer, buffer_size, "%d", random_number);
}

void nertc_init_ui() {
    if (engine_) return;
    xz_ws_audio_init();
    nertc_sdk_config_t nertc_sdk_config = { 0 };
    nertc_sdk_config.app_key = "appkey";
    nertc_sdk_config.device_id = "device id"; // for show
    //如果不打开服务端aec，可以自由配置这3个参数
    nertc_sdk_config.audio_config.sample_rate = 16000; 
    nertc_sdk_config.audio_config.channels = 1;
    nertc_sdk_config.audio_config.frame_duration = nertc_frame_duration_in; 

    nertc_sdk_config.event_handler.on_error = &OnError;
    nertc_sdk_config.event_handler.on_channel_status_changed = &OnChannelStatusChanged;
    nertc_sdk_config.event_handler.on_join = &OnJoin;
    nertc_sdk_config.event_handler.on_user_joined = &OnUserJoined;
    nertc_sdk_config.event_handler.on_user_left = &OnUserLeft;
    nertc_sdk_config.event_handler.on_user_audio_start = &OnUserAudioStart;
    nertc_sdk_config.event_handler.on_user_audio_stop = &OnUserAudioStop;
    nertc_sdk_config.event_handler.on_asr_caption_result = &OnAsrCaptionResult;
    nertc_sdk_config.event_handler.on_ai_data = &OnAiData;
    nertc_sdk_config.event_handler.on_audio_encoded_data = &OnAudioData;
    nertc_sdk_config.user_data = NULL;

    engine_ = nertc_create_engine(&nertc_sdk_config);
    xiaozhi_ui_chat_status("等待网络...");
    int ret = nertc_init(engine_);
    if (ret != 0) {
        rt_kprintf("Failed to initialize NERtc SDK, error: %d\r\n", ret);
        xiaozhi_ui_chat_output("初始化NERTC失败！！！");
        return;
    }
    xiaozhi_ui_chat_output("初始化NERTC成功");
}

void nertc_start(int argc, char **argv) {
    rt_kprintf("nertc_start\r\n");
    nertc_init_ui();
    xiaozhi_ui_chat_status("join...");

    uint64_t uid = 1234;
    char cname_str[20];
    random_to_string(cname_str, sizeof(cname_str), 1000, 9000);
    int ret = nertc_join(engine_, cname_str, "", uid);
    if (ret != 0) {
        rt_kprintf("Join failed, error: %d\r\n", ret);
    }
    rt_kprintf("nertc_join %s, api ret: %d\r\n", cname_str, ret);
}

void nertc_join_ui() {
    rt_kprintf("nertc_join_ui\r\n");
    nertc_start(0, NULL);
}

void nertc_leave_ui() {
    rt_kprintf("nertc_join_ui\r\n");
    if (engine_) {
        xiaozhi_ui_chat_output("正在退出...");
        xz_mic(0);
        xz_speaker(0);
        int ret = nertc_leave(engine_);
        // nertc_destroy_engine(engine_);
        // engine_ = NULL;
        rt_kprintf("nertc_leave, ret:%d\r\n", ret);
    }
}

void nertc_audio_send(uint8_t *data, int len)
{
    if (engine_)
    {
        // rt_kprintf("nertc_audio_send, len: %d\r\n", len);
        nertc_sdk_audio_encoded_frame_t encoded_frame;
        encoded_frame.data = data;
        encoded_frame.length = len;
        int samples_per_channel = 16 * nertc_frame_duration_in;
        nertc_sdk_audio_config_t audio_config = {16000, nertc_frame_duration_in, 1, samples_per_channel};
        nertc_push_audio_encoded_frame(engine_, NERTC_SDK_MEDIA_MAIN_AUDIO, audio_config, 100, &encoded_frame);
    }
}
void nertc_audio_pcm_send(uint8_t *data, int len)
{
    int per_frame_size = 16 * nertc_frame_duration_in * 2;
    while (engine_ && len >= per_frame_size)
    {
        // rt_kprintf("nertc_audio_pcm_send, len: %d, size:%d\r\n", len, per_frame_size);
        nertc_sdk_audio_frame_t audio_frame;
        audio_frame.type = NERTC_SDK_AUDIO_PCM_16;
        audio_frame.data = data;
        data += per_frame_size;
        audio_frame.length = per_frame_size;
        len -= per_frame_size;
        int samples_per_channel = 16 * nertc_frame_duration_in;
        nertc_sdk_audio_config_t audio_config = {16000, nertc_frame_duration_in, 1, samples_per_channel};
        audio_frame.config = audio_config;
        nertc_push_audio_frame(engine_, NERTC_SDK_MEDIA_MAIN_AUDIO, &audio_frame);
    }
}
void nertc_audio_speaker(uint8_t *enc_data, int enc_len, uint8_t* pcm_data, int pcm_len) {
    if (engine_) {
        // rt_kprintf("nertc_audio_speaker, enc_len: %d, pcm_len:%d\r\n", enc_len, pcm_len);
        nertc_sdk_audio_encoded_frame_t encoded_frame;
        encoded_frame.data = enc_data;
        encoded_frame.length = enc_len;

        nertc_sdk_audio_frame_t audio_frame;
        audio_frame.type = NERTC_SDK_AUDIO_PCM_16;
        audio_frame.data = pcm_data;
        audio_frame.length = pcm_len;
        int duration = pcm_len / 2 / 16;
        nertc_sdk_audio_config_t audio_config = {16000, duration, 1, 16 * duration};
        audio_frame.config = audio_config;
        nertc_push_audio_reference_frame(engine_, NERTC_SDK_MEDIA_MAIN_AUDIO, &encoded_frame, &audio_frame);
    }
}
MSH_CMD_EXPORT(xiaozhi2, Get Xiaozhi)


/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/

