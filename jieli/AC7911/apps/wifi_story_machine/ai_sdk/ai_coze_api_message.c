#include "app_config.h"
#include "system/includes.h"
#include "cJSON_common/cJSON.h"
#include "ai_coze_config.h"
#include "ai_coze_websocket.h"
#include "ai_coze_api_message.h"
#include "ai_coze_server.h"
#include "ai_device_manager.h"
#include "nertc_protocol.h"
#include "nertc_sdk.h"


ws_chat_state_t ws_chat_state = WS_CHAT_STATE_INIT;

void ws_set_chat_state(ws_chat_state_t state){
    //log_debug("ws_set_chat_state: from %d to %d", ws_chat_state, state);
    ws_chat_state = state;
}

//luy
CJSON_PUBLIC(cJSON *) cJSON_CreateAddObject(cJSON *object, const char *string){
    cJSON *item = cJSON_CreateObject();
    if (item) {
        cJSON_AddItemToObject(object, string, item);
    }

    return item;
}

u32 esp_timer_get_time(){
    static u32 count = 0;
    return ++count;
}


static cJSON *create_event_json(const char *event_type){
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;
    
    u8 id[20];
    snprintf(id,sizeof(id),"%d",esp_timer_get_time());
    cJSON_AddStringToObject(root, "id", id);
    cJSON_AddStringToObject(root, "event_type", event_type);
    return root;
}


void send_chat_update_request(void){
    log_debug("send_chat_update_request");
    // if (ws_chat_state != WS_CHAT_STATE_CREATED) {
    //     log_error("send_chat_update_request: invalid state");
    //     return;
    // }

    cJSON *root = create_event_json(COZE_WS_EVENT_CHAT_UPDATE);
    if (!root)return;

    cJSON *data = cJSON_CreateAddObject(root, "data");
    if (!data)goto cleanup;


    //设备输出音频到coze
    cJSON *input_audio = cJSON_CreateAddObject(data, "input_audio");if (!input_audio)goto cleanup;
        cJSON_AddStringToObject(input_audio, "format", COZE_INPUT_ENC_FORMAT_STRING);
        cJSON_AddStringToObject(input_audio, "codec", COZE_INPUT_SERVER_FORMAT_STRING);
        cJSON_AddNumberToObject(input_audio, "sample_rate", COZE_INPUT_ENC_SAMPLE_RATE);
        cJSON_AddNumberToObject(input_audio, "channel", 1/*COZE_INPUT_ENC_CHANNEL*/);
        cJSON_AddNumberToObject(input_audio, "bit_depth", 16);


    //coze输出音频到设备
    cJSON *output_audio = cJSON_CreateAddObject(data, "output_audio");if (!output_audio)goto cleanup;
    cJSON_AddStringToObject(output_audio, "codec", COZE_OUTPUT_DEC_FORMAT_STRING);

#if (COZE_OUTPUT_DEC_FORMAT ==1)
    cJSON *opus_config =  cJSON_CreateAddObject(output_audio, "opus_config");if (!opus_config)goto cleanup;
        cJSON_AddNumberToObject(opus_config, "sample_rate", COZE_OUTPUT_DEC_SAMPLE_RATE);
        cJSON_AddNumberToObject(opus_config, "bitrate", 24000);
        cJSON_AddBoolToObject(opus_config, "use_cbr", COZE_OUTPUT_DEC_USE_CBR);
        cJSON_AddNumberToObject(opus_config, "frame_size_ms", COZE_OUTPUT_DEC_SAMPLE_MS);
        cJSON *limit_config = cJSON_CreateAddObject(opus_config, "limit_config");if (!limit_config)goto cleanup;
            cJSON_AddNumberToObject(limit_config, "period", 1);
            cJSON_AddNumberToObject(limit_config, "max_frame_num", 80);
#else
    cJSON *pcm_config = cJSON_CreateAddObject(output_audio, "pcm_config");if (!pcm_config)goto cleanup;
        cJSON_AddNumberToObject(pcm_config, "sample_rate", COZE_OUTPUT_DEC_SAMPLE_RATE);
        cJSON_AddNumberToObject(pcm_config, "frame_size_ms", COZE_OUTPUT_DEC_SAMPLE_MS);
        // cJSON *limit_config = cJSON_CreateAddObject(pcm_config, "limit_config");if (!limit_config)goto cleanup;
        //     cJSON_AddNumberToObject(limit_config, "period", 1);
        //     cJSON_AddNumberToObject(limit_config, "max_frame_num", OPUS_MAX_FRAME_NUM);
#endif

    //speech_rate
    //loudness_rate
    if(strlen(coze_login.voice_id))cJSON_AddStringToObject(output_audio, "voice_id", coze_login.voice_id);

#if GIZWITS_RTC_ENABLE
    cJSON *chat_config = cJSON_CreateAddObject(data, "chat_config");if (!chat_config)goto cleanup;
    cJSON *meta_data = cJSON_CreateAddObject(chat_config, "meta_data");if (!meta_data)goto cleanup;
    cJSON *custom_variables = cJSON_CreateAddObject(chat_config, "custom_variables");if (!custom_variables)goto cleanup;
    cJSON *extra_params = cJSON_CreateAddObject(chat_config, "extra_params");if (!extra_params)goto cleanup;

    if(strlen(coze_login.user_id))cJSON_AddStringToObject(chat_config, "user_id", coze_login.user_id);
    if(strlen(coze_login.conv_id))cJSON_AddStringToObject(chat_config, "conversation_id", coze_login.conv_id);
    cJSON_AddBoolToObject(chat_config, "auto_save_history", true);


    cJSON *turn_detection = cJSON_CreateAddObject(data, "turn_detection");if (!turn_detection)goto cleanup;
    cJSON_AddStringToObject(turn_detection, "type", "server_vad");
    cJSON_AddNumberToObject(turn_detection, "prefix_padding_ms", 300);
    cJSON_AddNumberToObject(turn_detection, "silence_duration_ms", 300);

    cJSON *voice_processing_config = cJSON_CreateAddObject(data, "voice_processing_config");if (!voice_processing_config)goto cleanup;
    cJSON_AddBoolToObject(voice_processing_config, "enable_ans", false);//主动噪声抑制。自动识别并过滤掉背景环境中的各种噪音（如键盘声、空调声、街道嘈杂声），让说话者的声音更清晰。
    cJSON_AddBoolToObject(voice_processing_config, "enable_pdns", true);//声纹降噪。专门针对特定说话人的声音进行优化，能更精准地保留目标人声。
#endif

    // Serialize the JSON object.
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str)goto cleanup;
    printf("json_str:%s\n", json_str);

    // Send the serialized JSON message.
    int ret = ws_client_send(json_str,strlen(json_str), WCT_TXTDATA);
    if (ret != 0) {
        log_error("Failed to send chat update, ret:%d", ret);
    }else {
        ws_set_chat_state(WS_CHAT_STATE_UPDATE_SENT);
    }
    free(json_str);

cleanup:
    // Clean up the entire JSON structure.
    // This will recursively free all child objects that have been added to the root.
    cJSON_Delete(root);
}



void send_hello_message(void){
    // 发送初始化消息 ，给我讲个故事
    const char *init_message = "{"
        "\"event_type\":\"conversation.message.create\","
        "\"data\":{"
            "\"role\":\"user\","
            "\"content_type\":\"text\","
            "\"content\":\"你好，给我讲个故事\""
        "}"
    "}";

    // Send the serialized JSON message.
    int ret = ws_client_send(init_message,strlen(init_message), WCT_TXTDATA);
    if (ret != 0) {
        log_error("Failed to send chat update, ret:%d", ret);
    }else {
        ws_set_chat_state(WS_CHAT_STATE_UPDATE_SENT);
    }
}

static u8 audio_buf[COZE_OUTPUT_DEC_FORMAT_BUFFER];
static void handle_conversation_audio_delta(cJSON *json){
    ws_set_chat_state(WS_CHAT_STATE_CONVERSATION_MESSAGE_COMPLETED);

    cJSON *data = cJSON_GetObjectItem(json, "data");
    if (!data)return;

    char *content = cJSON_GetObjectItem(data, "content")->valuestring;
    u32 decoded_len = 0;
    mbedtls_base64_decode(audio_buf, COZE_OUTPUT_DEC_FORMAT_BUFFER, &decoded_len,content, strlen(content));
    dec_save_cbuf_put(audio_buf,decoded_len);
}


//接收数据管理
void coze_ws_recv_handler(u8 *buf, u32 len){
    cJSON *json = cJSON_Parse(buf);
    if (json == NULL) {
        printf("Failed to parse JSON, %s\n", buf);
        return;
    }

    cJSON *event_type = cJSON_GetObjectItem(json, "event_type");
    if (event_type == NULL)goto __del_json;
    //printf("===========>event_type->valuestring: %s", event_type->valuestring);

    // char *json_str = cJSON_PrintUnformatted(json);
    // if (!json_str)goto __del_json;
    // printf("===========>json_str: %s", json_str);
    // free(json_str);


    if(!strcmp(event_type->valuestring, "input_audio_buffer.completed")){
        printf("===========>event_type->valuestring: %s", event_type->valuestring);
        //ws_set_chat_state(WS_CHAT_STATE_INPUT_AUDIO_COMPLETED);
    }else
    if(!strcmp(event_type->valuestring, "chat.created")){//服务器创建对话
        printf("===========>event_type->valuestring: %s", event_type->valuestring);
        ws_set_chat_state(WS_CHAT_STATE_CREATED);
        send_chat_update_request();
    }else
    if(!strcmp(event_type->valuestring, "chat.updated")){//服务器获取对话配置
        printf("===========>event_type->valuestring: %s", event_type->valuestring);
            ws_set_chat_state(WS_CHAT_STATE_UPDATED);
            ai_send_event_to_sdk(AI_EVENT_COZE_WEBSOCKET_BUILD_SUCC, 0);
    }else
    if(!strcmp(event_type->valuestring, "conversation.audio_transcript.completed")){//语义识别完成

        //ws_set_chat_state(WS_CHAT_STATE_AUDIO_TRANSCRIPT_COMPLETED);
        // cJSON *data = cJSON_GetObjectItem(json, "data");
        // if (!data)goto __del_json;
        // char *content = cJSON_GetObjectItem(data, "content")->valuestring;
        // printf("len:%d, content: %s", strlen(content), content);
        // if(strlen(content) > 0){
        //     ai_send_event_to_sdk(AI_EVENT_MEDIA_START, 0);
        // }else{
        //     ai_send_event_to_sdk(AI_EVENT_MEDIA_STOP, 0);
        // }
    }else
    if(!strcmp(event_type->valuestring, "conversation.chat.created")){//语音，对话开始
        //dec_start();
    }else
    if(!strcmp(event_type->valuestring, "conversation.chat.in_progress")){//服务器语音处理

    }else
    if(!strcmp(event_type->valuestring, "conversation.audio.delta")){//语音增量消息
        handle_conversation_audio_delta(json);
    }else
    if(!strcmp(event_type->valuestring, "conversation.audio.completed")){//语音传输完成
        //ws_set_chat_state(WS_CHAT_STATE_AUDIO_COMPLETED);
    }else
    if(!strcmp(event_type->valuestring, "conversation.message.delta")){//信息增量

        //ws_set_chat_state(WS_CHAT_STATE_MESSAGE_DELTA);
    }else
    if(!strcmp(event_type->valuestring, "conversation.message.completed")){
        printf("===========>event_type->valuestring: %s", event_type->valuestring);
        //ws_set_chat_state(WS_CHAT_STATE_CONVERSATION_MESSAGE_COMPLETED);
    }else
    if(!strcmp(event_type->valuestring, "conversation.audio.sentence_start")){//增量语音字幕


    }else
    if(!strcmp(event_type->valuestring, "conversation.chat.completed")){
        printf("===========>event_type->valuestring: %s", event_type->valuestring);
        //ws_set_chat_state(WS_CHAT_STATE_CHAT_COMPLETED);
        //ai_send_event_to_sdk(AI_EVENT_MEDIA_STOP, 0);
    }else

    if(!strcmp(event_type->valuestring, "conversation.audio_transcript.update")){//对话当中，服务端语义识别更新
        //ws_set_chat_state(WS_CHAT_STATE_AUDIO_TRANSCRIPT_UPDATE);
    }else
    if(!strcmp(event_type->valuestring, "input_audio_buffer.speech_started")){//此事件表示服务端识别到用户正在说话。只有在 server_vad 模式下，才会返回此事件。
        printf("-------------------------------input_audio_buffer.speech_started");
        //ws_set_chat_state(WS_CHAT_STATE_AUDIO_SPEECH_STARTED);
    }else
    if(!strcmp(event_type->valuestring, "input_audio_buffer.speech_stopped")){//此事件表示服务端识别到用户已停止说话。只有在 server_vad 模式下，才会返回此事件。
        printf("-------------------------------input_audio_buffer.speech_stopped");
        //ws_set_chat_state(WS_CHAT_STATE_AUDIO_SPEECH_STOPED);
    }else
    if(!strcmp(event_type->valuestring, "error")){ 
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%s",COZE_WS_EVENT_ERROR);
        char *json_str = cJSON_PrintUnformatted(json);
        if (!json_str)goto __del_json;
        printf("===========>json_str: %s", json_str);
        free(json_str);
    }else
    if(!strcmp(event_type->valuestring, "conversation.chat.failed")){
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%s",COZE_WS_EVENT_CONVERSATION_CHAT_FAILED);
        //void my_set_prompt_poweron_close(int id);
        //my_set_prompt_poweron_close(CFG_USER_CLOSE_POWERON_PROMPT);
        //my_set_prompt_poweron_close(CFG_USER_CLOSE_NETWORK_PROMPT);
        //cpu_reset(); 
    }else{
        printf("====????=======>event_type->valuestring: %s", event_type->valuestring);
    }




__del_json:
    cJSON_Delete(json);
}




#define BASE64_LEN (COZE_INPUT_OUTPUT_LEN)
static u8 message[BASE64_LEN];
static u32 nertc_audio_timestamp = 0;

/* NERTC 下行音频回调处理 - 将接收到的音频喂给播放设备 */
static void nertc_audio_data_callback(const nertc_protocol_audio_data_t *audio, void *user_data)
{
    if (!audio || !audio->data || audio->length <= 0) {
        return;
    }
    
    /* 将音频数据喂给播放设备 */
    int ret = dec_save_cbuf_put((void *)audio->data, audio->length);
}

/* 注册 NERTC 音频回调 */
void nertc_register_audio_callback(void)
{
    nertc_protocol_set_audio_callback(nertc_audio_data_callback, NULL);
}

/* 发送音频到 NERTC 服务器 */
int send_voice_data_to_nertc_server(u8 *voice_data, int len){
    if (!voice_data || len <= 0) {
        return -1;
    }

    if (!nertc_protocol_is_joined()) {
        return -2;
    }

    int ret = nertc_protocol_push_audio_encoded(voice_data, len, 0);
    
    return ret;
}

//流式上传音频片段
int send_voice_data_to_server(u8 *voice_data, int len){
    if (!ai_coze.ai_connected || !voice_data || len <= 0 ) {
        printf("no voice data or websocket not connected\n");
        return -1;
    }

    // 进行 base64 编码
    size_t encoded_len;
    memset(message, 0, BASE64_LEN);
    snprintf(message,BASE64_LEN,"{\"id\":\"%d\",\"event_type\":\"input_audio_buffer.append\",\"data\":{\"delta\":\"",esp_timer_get_time());

    mbedtls_base64_encode(&message[strlen(message)], BASE64_LEN, &encoded_len, voice_data, len);

    snprintf(&message[strlen(message)],5,"\"}}");
    return ws_client_send(message,strlen(message), WCT_TXTDATA);
}


//提交完成
void end_recorder(void){
    if (!ai_coze.ai_connected)return;
    memset(message, 0, BASE64_LEN);
    snprintf(message, BASE64_LEN,"{\"id\":\"%d\",\"event_type\":\"input_audio_buffer.complete\",\"data\":{}}", esp_timer_get_time());
    printf("msg len:%d",strlen(message));
    //发送完成消息

    ws_client_send(message,strlen(message),WCT_TXTDATA);
}


//智能体输出中断
void cancel_ai_agent_task(void){
    // 使用静态变量避免栈溢出


    static const char *cancel_message_format = "{"
        "\"id\":\"%s\","
        "\"event_type\":\"conversation.chat.cancel\""
    "}";
    static char cancel_message[1024];
    char event_id[32];
    memset(event_id, 0, sizeof(event_id));
    sprintf(event_id, "%d", esp_timer_get_time());

    // 构建取消消息
    snprintf(cancel_message, sizeof(cancel_message), cancel_message_format, event_id);

    // 发送取消消息
    if (ai_coze.ai_connected) {
        int ret =  ws_client_send(cancel_message,strlen(cancel_message),WCT_TXTDATA);
        if (ret != 0) {
            log_error("Failed to send chat update, ret:%d", ret);   //发送失败
        }
        else {
            ws_set_chat_state(WS_CHAT_STATE_UPDATE_SENT);
        }
    }

}
