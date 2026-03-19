/***********************************MQTT 测试说明*******************************************
 *说明：
 * 	 通过MQTT协议连接阿里云, 向阿里云订阅主题和发布温度，湿度消息
 *********************************************************************************************/
#include "mqtt/MQTTClient.h"
#include "system/includes.h"
#include "wifi/wifi_connect.h"
#include "cJSON_common/cJSON.h"
#include "app_config.h"

#include "ai_coze_server.h"
#include "ai_device_manager.h"
#include "gizwits_mqtt.h"



#if 1//def USE_MQTT_TEST
#define COMMAND_TIMEOUT_MS 30000        //命令超时时间
#define MQTT_TIMEOUT_MS 10           //接收阻塞时间
#define MQTT_KEEPALIVE_TIME 30000       //心跳时间
#define SEND_BUF_SIZE 1024              //发送buf大小
#define READ_BUF_SIZE 1024              //接收buf大小
static char send_buf[SEND_BUF_SIZE];    //发送buf
static char read_buf[READ_BUF_SIZE];    //接收buf

static Client client; //MQTT客户端
static MQTTMessage message; //MQTT消息
static Network network; //网络


void mqtt_recv_parse(char *data, u32 len){
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        printf("parse json error\n");
        return;
    }
    printf("\n*****mqtt_recv_parse**************************\n");
    cJSON *method = cJSON_GetObjectItem(json, "method");
    if (strcmp(method->valuestring, "websocket.auth.response") == 0) {
        printf("websocket.auth.response\n");
        cJSON *body = cJSON_GetObjectItem(json, "body");
        cJSON *platform_type = cJSON_GetObjectItem(body, "platform_type");
        cJSON *token_quota = cJSON_GetObjectItem(body, "token_quota");
        cJSON *coze_websocket = cJSON_GetObjectItem(body, "coze_websocket");
        cJSON *bot_id = cJSON_GetObjectItem(coze_websocket, "bot_id");
        cJSON *voice_id = cJSON_GetObjectItem(coze_websocket, "voice_id");
        cJSON *user_id = cJSON_GetObjectItem(coze_websocket, "user_id");
        cJSON *conv_id = cJSON_GetObjectItem(coze_websocket, "conv_id");
        cJSON *access_token = cJSON_GetObjectItem(coze_websocket, "access_token");
        cJSON *expires_in = cJSON_GetObjectItem(coze_websocket, "expires_in");

        if(!body || !platform_type || !token_quota || !coze_websocket || !bot_id || !voice_id || !user_id || !conv_id || !access_token || !expires_in) {
            printf("json parse error\n");
            ai_send_event_to_sdk(AI_EVENT_MQTT_GET_COZE_INFO_FAIL, 0);
            return;
        }

        coze_login.platform_type = platform_type->valueint;
        coze_login.token_quota = token_quota->valueint;
        strcpy(coze_login.bot_id, bot_id->valuestring);
        strcpy(coze_login.voice_id, voice_id->valuestring);
        strcpy(coze_login.user_id, user_id->valuestring);
        strcpy(coze_login.conv_id, conv_id->valuestring);
        strcpy(coze_login.access_token, access_token->valuestring);
        coze_login.expires_in = expires_in->valueint;

        printf("platform_type:%d\n", coze_login.platform_type);
        printf("token_quota:%d\n", coze_login.token_quota);
        printf("bot_id:%s\n", coze_login.bot_id);
        printf("voice_id:%s\n", coze_login.voice_id);
        printf("user_id:%s\n", coze_login.user_id);
        printf("conv_id:%s\n", coze_login.conv_id);
        printf("access_token:%s\n", coze_login.access_token);
        printf("expires_in:%d\n", coze_login.expires_in);
        ai_send_event_to_sdk(AI_EVENT_MQTT_GET_COZE_INFO_SUCC, 0);
    } else if(strcmp(method->valuestring, "websocket.config.change") == 0) {
        ai_send_event_to_sdk(AI_EVENT_MQTT_CHANGE_ROOM, 0);
    }
    printf("***********************************************\n\n");

    cJSON_Delete(json);
}



//接收回调，当订阅的主题有信息下发时，在这里接收
static void messageArrived1(MessageData *data){
    char temp[1024] = {0};

    strncpy(temp, data->topicName->lenstring.data, data->topicName->lenstring.len);
    temp[data->topicName->lenstring.len] = '\0';
    printf("Message1 arrived on topic (len : %d, topic : %s)\n", data->topicName->lenstring.len, temp);

    memset(temp, 0, sizeof(temp));
    strncpy(temp, data->message->payload, data->message->payloadlen);
    temp[data->message->payloadlen] = '\0';
    printf("message1 (len : %d, payload : %s)\n", data->message->payloadlen, temp);

    mqtt_recv_parse(temp, data->message->payloadlen);
}

static void messageArrived2(MessageData *data){
    char temp[1024] = {0};

    strncpy(temp, data->topicName->lenstring.data, data->topicName->lenstring.len);
    temp[data->topicName->lenstring.len] = '\0';
    printf("Message2 arrived on topic (len : %d, topic : %s)\n", data->topicName->lenstring.len, temp);

    memset(temp, 0, sizeof(temp));
    strncpy(temp, data->message->payload, data->message->payloadlen);
    temp[data->message->payloadlen] = '\0';
    printf("message2 (len : %d, payload : %s)\n", data->message->payloadlen, temp);

    mqtt_recv_parse(temp, data->message->payloadlen);
}

static void messageArrived3(MessageData *data){

    printf("messageArrived3 , %d, %s\n", data->topicName->lenstring.len, data->topicName->lenstring.data);
    u8 temp[512] = {0};
    memcpy(temp, data->message->payload, data->message->payloadlen);
    put_buf(temp, data->message->payloadlen);

    // strncpy(temp, data->topicName->lenstring.data, data->topicName->lenstring.len);
    // temp[data->topicName->lenstring.len] = '\0';
    // printf("Message3 arrived on topic (len : %d, topic : %s)\n", data->topicName->lenstring.len, temp);

    // memset(temp, 0, sizeof(temp));
    // strncpy(temp, data->message->payload, data->message->payloadlen);
    // temp[data->message->payloadlen] = '\0';
    // put_buf(temp, data->message->payloadlen);

    // printf("message3 (len : %d, payload : %s)\n", data->message->payloadlen, temp);
     //mqtt_recv_parse(temp, data->message->payloadlen);
     
    
    u16 index = 0;
    u32 head = temp[index] << 24 | temp[index+1] << 16 | temp[index+2] << 8 | temp[index+3];
    if(head != FIXED_HEADER)return;
    
    index += 4;
    u16 cmd = temp[index] << 8 | temp[index+1];
    if(cmd != 0x021d)return;
    index += 2;

    //命令标识
    u16 module = temp[index] << 8 | temp[index+1];
    printf("module: 0x%04x\n", module);
    index += 2;

    //命令数据长度
    u16 data_len = (u8)temp[index] << 8 | (u8)temp[index+1];
    printf("data_len: %d, 0x%02x, 0x%02x\n", data_len, temp[index],temp[index+1]);

    index += 2;
    //子设备ID长度
    if(module & BIT(0)){
        u16 sub_id_len = temp[index] << 8 | temp[index+1];
        printf("sub_id_len: %d, 0x%02x, 0x%02x\n", sub_id_len, temp[index],temp[index+1]);

        index += 2;
        u8 sub_id_temp[100] = {0};
        memcpy(sub_id_temp, &temp[index], sub_id_len);
        printf("sub_id: %s\n", sub_id_temp);
        index += sub_id_len;
    }
    if(module & BIT(1)){
        u16 version_len = (u8)temp[index] << 8 | (u8)temp[index+1];
        printf("version_len: %d, 0x%02x, 0x%02x\n", version_len, temp[index],temp[index+1]);

        index += 2;
        u8 version_temp[100] = {0};
        memcpy(version_temp, &temp[index], version_len);
        printf("version: %s\n", version_temp);
        index += version_len;
    }

    u16 md5_len = (u8)temp[index] << 8 | (u8)temp[index+1];
    printf("md5_len: %d, 0x%02x, 0x%02x\n", md5_len, temp[index],temp[index+1]);    
    index += 2;

    u8 md5_temp[100] = {0};
    memcpy(md5_temp, &temp[index], md5_len);
    printf("md5: %s\n", md5_temp);
    index += md5_len;

    u16 http_url_len = (u8)temp[index] << 8 | (u8)temp[index+1];
    printf("http_url_len: %d, 0x%02x, 0x%02x\n", http_url_len, temp[index],temp[index+1]);   
    index += 2;

    u8 http_url_temp[200] = {0};
    memcpy(http_url_temp, &temp[index], http_url_len);
    printf("http_url: %s\n", http_url_temp);

    void my_app_firware_download(char *url);    
    my_app_firware_download(http_url_temp);
}


//上报版本信息
static int pack_version_report(Client *c, const char *topicName, MQTTMessage *message){
    printf("pack_version_report, %s\n"SOFT_VERSION","HARD_VERSION);
    u8 buf[128] = {0};
    u8 buf_len = 0;
    buf[0] = (FIXED_HEADER >> 24) & 0xFF;
    buf[1] = (FIXED_HEADER >> 16) & 0xFF;
    buf[2] = (FIXED_HEADER >> 8) & 0xFF;
    buf[3] = FIXED_HEADER & 0xFF;

    buf[4] = (CMD_VERSION_REPORT >> 8) & 0xFF;
    buf[5] = CMD_VERSION_REPORT & 0xFF;

    buf[6] = (MODULE_BITS >> 8) & 0xFF;
    buf[7] = MODULE_BITS & 0xFF;
    // 长度2byte
    buf[8] = 0x00;
    buf[9] = 0x19;

    // 版本号长度
    u16 len = strlen(SOFT_VERSION","HARD_VERSION);
    printf("version len: %d\n",len);
    buf[10] = (len >> 8) & 0xFF;
    buf[11] = len & 0xFF;

    // 版本号 23个字节
    strncpy((char *)&buf[12], SOFT_VERSION","HARD_VERSION, len);
    message->payload = (char*)buf;
    message->payloadlen = len+12;

    printf("message %d\n", message->payloadlen);
    put_buf(message->payload, message->payloadlen);

    //发布消息
    int err = MQTTPublish(c, topicName, message);
    if (err != 0) {
        printf("MQTTPublish failed, err : 0x%x\n", err);
    }else{
        printf("%s publish success", topicName);
    }
    return err;
}

//发送房间认证请求
static int get_room_info(Client *c, const char *topicName, MQTTMessage *message){
    printf("get_room_info\n");
    char buf[128] = "{\"method\": \"websocket.auth.request\"}";
    message->payload = buf;
    message->payloadlen = strlen((char*)buf);
    printf("payloadlen: %d, message->payload: %s", message->payloadlen, message->payload);
    int err = MQTTPublish(c, topicName, message);
    if (err != 0) {
        printf("MQTTPublish fail, err : 0x%x\n", err);
    }else{
        printf("%s publish success\n", topicName);
    }
    return err;
}


static int mqtt_start(void){
    int err;
    while (1) {
        printf("CCCCCCCCCCCCCCCCCCCCC   Connecting to the network...\n");
        if (WIFI_STA_NETWORK_STACK_DHCP_SUCC == wifi_get_sta_connect_state()) {
            printf("Network connection is successful!\n");
            break;
        }
        os_time_dly(1000);
    }


_reconnect:
    //初始化网络接口
    NewNetwork(&network);
    SetNetworkRecvTimeout(&network, 1000);

    //初始化客户端
    MQTTClient(&client, &network, COMMAND_TIMEOUT_MS, send_buf, sizeof(send_buf), read_buf, sizeof(read_buf));
    //tcp层连接服务器
    err = ConnectNetwork(&network, device_mqtt_config.address, device_mqtt_config.port);
    if (err != 0)goto __err;
    

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    connectData.willFlag = 0;
    connectData.MQTTVersion = 3;                                   //mqtt版本号
    connectData.clientID.cstring = device_mqtt_config.username;                      //连接时的用户名 
    connectData.username.cstring = device_mqtt_config.client_id;                       //客户端id
    connectData.password.cstring = device_mqtt_config.password;                       //连接时的密码
    connectData.keepAliveInterval = MQTT_KEEPALIVE_TIME / 1000;    //心跳时间
    connectData.cleansession = 1;                                  //是否使能服务器的cleansession，0:禁止, 1:使能

    //mqtt层连接,向服务器发送连接请求
    err = MQTTConnect(&client, &connectData);
    if (err != 0)goto __err;
    
    //订阅主题1
    err = MQTTSubscribe(&client, device_mqtt_config.subscribeTopic1, MQTT_INFO_QOS, messageArrived1);
    if (err != 0)goto __err;
    printf("MQTTSubscribe1 success %s", device_mqtt_config.subscribeTopic1);

    //订阅主题2
    err = MQTTSubscribe(&client, device_mqtt_config.subscribeTopic2, MQTT_INFO_QOS, messageArrived2);
    if (err != 0)goto __err;
    printf("MQTTSubscribe2 success %s", device_mqtt_config.subscribeTopic2);

    //订阅主题3
    err = MQTTSubscribe(&client, device_mqtt_config.subscribeTopic3, MQTT_INFO_QOS, messageArrived3);
    if (err != 0)goto __err;
    printf("MQTTSubscribe3 success %s", device_mqtt_config.subscribeTopic3);

    //取消主题订阅
    //MQTTUnsubscribe(&client, subscribeTopic);

    message.qos = MQTT_INFO_QOS;
    message.retained = 0;
    //start_publish_task(); //启动发布任务
    err = pack_version_report(&client, MQTT_VERSION_PUBLISH_TOPIC, &message);
    if (err != 0)goto __err;
    printf("pack_version_report success %s", device_mqtt_config.subscribeTopic3);

    char tmp_topic[64] = {0};
    sprintf(tmp_topic, MQTT_GET_ROOM_PUBLISH_TOPIC, device_param.device_id);
    err = get_room_info(&client, tmp_topic, &message);
    if (err != 0)goto __err;
    printf("get_room_info success %s", device_mqtt_config.subscribeTopic3);

// void gizwits_check_ota_update(void);
// gizwits_check_ota_update();

    while (1) {
        err = MQTTYield(&client, MQTT_TIMEOUT_MS);
        if (err != 0) {
            printf("err : %d , client.isconnected: %d\n", err, client.isconnected);
            break;
        }
    }

    ai_send_event_to_sdk(AI_EVENT_MQTT_DISCONNECTED, 0);
return 0;

__err:
    printf("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk mqtt");
    
    MQTTDisconnect(&client);
    network.disconnect(&network);
    ai_send_event_to_sdk(AI_EVENT_MQTT_GET_COZE_INFO_FAIL, 0);
    return -1;
}

static int task_id_mqtt_publish = 0;
void gizwits_mqtt_main(void){

    if (thread_fork("mqtt_start", 10, 4 * 1024, 0, &task_id_mqtt_publish, mqtt_start, NULL) != OS_NO_ERR) {
         ai_send_event_to_sdk(AI_EVENT_MQTT_GET_COZE_INFO_FAIL, 0);
    }
}

void mqtt_task_kill(void){
    //MQTTDisconnect(&client);
    //network.disconnect(&network);

    os_time_dly(10);
    if(task_id_mqtt_publish){
        printf("mqtt_task_kill\n ");
        thread_kill(&task_id_mqtt_publish, KILL_WAIT);      //删除任务
        task_id_mqtt_publish = 0;
    }
}

#endif//USE_MQTT_TEST
