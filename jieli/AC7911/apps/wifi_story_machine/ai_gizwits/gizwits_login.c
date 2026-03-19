
#include "system/includes.h"
#include "wifi/wifi_connect.h"
#include "http/http_cli.h"
#include "app_config.h"
#include "ai_device_manager.h"
#include "ai_sdk/ai_coze_server.h"
#include "le_gizwits_profile.h"
#include "app_sha_256.h"
#include "mbedtls/md5.h"
#include "syscfg_id.h"


#define HTTP_RANDOM_LEN    16
#define MQTT_RANDOM_LEN     10
#define MAX_VALUE_LEN 64


extern void pseudo_random_genrate(uint8_t *dest, unsigned size);
void gizwits_create_token(char *random, char *token_str);
void generate_random_number(u8 *random_str, u16 len);
static void pack_mqtt_login_info(const char *str);

#define HTTP_HEAD_OPTION   \
    "POST /test/gateway HTTP/1.1\r\n" \
    "Host: wordcard-ets.ceshiservice.cn\r\n"\
    "Connection: close\r\n"\
    "Content-Length: %d\r\n"\
    "Content-Type: application/json\r\n"\
    "Cache-Control: no-store\r\n\r\n"\

// \r\n\r\n 终止符： 必须存在，否则服务器永远在等待请求的结束。样例
// #define GIZWITS_HEAD_FIX                 \
//     "GET /v2/devices/n721e21d/bootstrap HTTP/1.1\r\n"    \
//     "Host: agent.gizwitsapi.com\r\n"        \
//     "Accept: */*\r\n" \
//     "X-Sign-Method: sha256\r\n" \
//     "X-Sign-Nonce: FURBDUVMXXAVGPPR\r\n" \
//     "X-Sign-Token: 9a72e52a6fea831d668453761ee342fa80232a5540b744619d9d300f4c9a0104\r\n" \
//     "Connection: close\r\n" \
//     "\r\n"
//3.3 获取设备登录地址
#define GIZWITS_HEAD                 \
    "GET /v2/devices/%s/bootstrap HTTP/1.1\r\n"    \
    "Host: agent.gizwitsapi.com\r\n"        \
    "Accept: */*\r\n" \
    "X-Sign-Method: sha256\r\n" \
    "X-Sign-Nonce: %s\r\n" \
    "X-Sign-Token: %s\r\n" \
    "Connection: close\r\n" \
    "\r\n"

#define GIZWITS_ONBARD_HEAD                 \
    "POST /v2/devices/%s/network HTTP/1.1\r\n"    \
    "Host: agent.gizwitsapi.com\r\n"        \
    "Accept: */*\r\n" \
    "X-Sign-Method: sha256\r\n" \
    "X-Sign-Nonce: %s\r\n" \
    "X-Sign-Token: %s\r\n" \
    "X-Sign-ETag: %s\r\n" \
    "X-Trace-Id: %s\r\n"\
    "Content-type: text/plain\r\n" \
    "Content-length: %d\r\n"\
    "Connection: close\r\n"\
    "\r\n"

    // "Connection: close\r\n" \
    // "Content-Type: text/plain\r\n" \
    // "Content-Length: %d\r\n" \

#define GIZWITS_OTA_HEAD                 \
        "PATCH /v2/products/%s/firmwares/ HTTP/1.1\r\n"  \
        "Host: agent.gizwitsapi.com\r\n"    \
        "Accept: */*\r\n"   \
        "Content-Type: application/json\r\n"    \
        "X-Sign-Method: sha256\r\n"     \
        "X-Sign-Nonce: %s\r\n"  \
        "X-Sign-Token: %s\r\n"  \
        "Content-Length: %d\r\n"    \
        "Connection: close\r\n" \
        "\r\n",



// #define HTTP_GIZWITS_URL_FIX  "http://agent.gizwitsapi.com/v2/devices/n721e21d/bootstrap"

#define HTTP_GIZWITS_URL  "http://agent.gizwitsapi.com/v2/devices/%s/bootstrap"
#define ONBOARDING_URL  "http://agent.gizwitsapi.com/v2/devices/%s/network"
#define OTA_CHECK_URL "http://agent.gizwitsapi.com/v2/products/%s/firmwares/%s/devices"
#define OTA_DOWNLOAD_URL "http://download.gizwits.com/%s"

#define ONBOARDING_BODY_DATA "is_reset=1&random_code=%s&lan_proto_ver=v5.0&user_id=%s"

static char user_head_buf[512];
static char user_curl_buf[256];
static char user_body_buf[256];
static char ota_buf[1024];

static char uid[33];        // UID（最大 32 字节 + 1 字节结束符）


void set_uid(char *uuid){
    if(strlen(uuid) > 0){
        strcpy(uid, uuid);
    }
}

static u16 http_get_task_pid = 0;
static u16 http_post_task_pid = 0;



static void http_post_task(void *priv){
    printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&http_post_task\n");


    int ret = 0;
    http_body_obj http_body_buf;
    httpcli_ctx *ctx = (httpcli_ctx *)calloc(1, sizeof(httpcli_ctx));
    if (!ctx) goto __login_err;
    printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&http_post_task1\n");

    memset(&http_body_buf, 0x0, sizeof(http_body_obj));
    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 4 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *) malloc(http_body_buf.buf_len * http_body_buf.buf_count);
    if (!http_body_buf.p) {
        free(ctx);
        goto __login_err;
    }
    printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&http_post_task2\n");

    // 如果是post的就加上，如果get的就去掉
    ctx->post_data = user_body_buf;
    ctx->data_len = strlen(ctx->post_data);// strlen(ctx->post_data);
    ctx->data_format = "text/plain";
    ctx->timeout_millsec = 5000;
    ctx->priv = &http_body_buf;
    ctx->url = user_curl_buf;
    ctx->user_http_header = user_head_buf;
    ret = httpcli_post(ctx);
    if (ret == HERROR_OK) {
        printf("====================================>:%d",ret);
        if (http_body_buf.recv_len > 0) {
            http_body_buf.p[http_body_buf.recv_len] = '\0';
            printf("\nReceive len:  %d\n", http_body_buf.recv_len);
            printf("%s\n", http_body_buf.p);
            pack_mqtt_login_info(http_body_buf.p);
            device_param.user_info.is_reset = 0;
            device_param_save(CFG_USER_DEVICE_INFO);
        }
    }

    //关闭连接
    httpcli_close(ctx);
    if (http_body_buf.p) {
        free(http_body_buf.p);
        http_body_buf.p = NULL;
    }
    if (ctx) {
        free(ctx);
        ctx = NULL;
    }
    printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&http_post_task3\n");
    if(ret != HERROR_OK)goto __login_err;
    ai_send_event_to_sdk(AI_EVENT_MQTT_REG_SUCC, 0);
    printf("*********************************************************HTTP end*********************************************************\n");
    return;

__login_err:
    ai_send_event_to_sdk(AI_EVENT_MQTT_REG_FAIL, 0); 
    printf("*********************************************************HTTP error*********************************************************\n");
}



void generate_random_number(u8 *random_str, u16 len)
{
    u8 test[8] = {0};
    char poor[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // 用于测试
    for(int i = 0; i < len; i++){
        pseudo_random_genrate(test, 1);
        // printf("random:%d, gewei: %d\n", test[0], test[0]%10);
        random_str[i] = poor[test[0]%10];
    }
    printf("random_str: %s",random_str);


}

static char http_head_buf[256] = {0};

void hex2str(const unsigned char *hex, size_t hex_len, char *str) {
    static const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < hex_len; ++i) {
        str[i * 2] = hex_chars[(hex[i] & 0xF0) >> 4];
        str[i * 2 + 1] = hex_chars[hex[i] & 0x0F];
    }
    str[hex_len * 2] = '\0';
}

// 提取键对应的值
int extract_value(const char *src, const char *key, char *value, size_t value_size) {
    char *start = strstr(src, key);
    if (!start) return 0;

    start += strlen(key); // 跳过键名
    if (*start != '=') return 0; // 检查等号
    start++;

    char *end = strchr(start, '&');
    size_t len = end ? (size_t)(end - start) : strlen(start);

    if (len >= value_size) return 0; // 确保不会写入超过缓冲区

    strncpy(value, start, len);
    value[len] = '\0';
    return 1;
}

// 解析函数
int parse_string(const char *str, device_mqtt_config_t *data){
    char value[MAX_VALUE_LEN];

    if (!extract_value(str, "address", data->address, sizeof(data->address))) return 0;
    if (!extract_value(str, "port", value, sizeof(value))) return 0;
    data->port = atoi(value);

    if (!extract_value(str, "restart_time", value, sizeof(value))) return 0;
    data->restart_time = atoi(value);

    if (!extract_value(str, "tz_offset", value, sizeof(value))) return 0;
    data->tz_offset = atoi(value);

    return 0;
}

static void pack_mqtt_login_info(const char *str){
    parse_string(str, &device_mqtt_config);

    char random[MQTT_RANDOM_LEN+1] = {0};
    generate_random_number(random, MQTT_RANDOM_LEN);
    snprintf(device_mqtt_config.client_id, sizeof(device_mqtt_config.client_id),
        "%s|signmethod=sha256,signnonce=%s", device_param.device_id, random);
    snprintf(device_mqtt_config.username, sizeof(device_mqtt_config.client_id),
        "%s", device_param.device_id);

    gizwits_create_token(random, device_mqtt_config.password);

    snprintf(device_mqtt_config.subscribeTopic1, sizeof(device_mqtt_config.subscribeTopic1),
        "llm/%s/config/response", device_param.device_id);

    snprintf(device_mqtt_config.subscribeTopic2, sizeof(device_mqtt_config.subscribeTopic2),
    "llm/%s/config/push", device_param.device_id);

    snprintf(device_mqtt_config.subscribeTopic3, sizeof(device_mqtt_config.subscribeTopic3),
    "ser2cli_res/%s", device_param.device_id);


    printf("\n**************device_mqtt_config**************\n", device_mqtt_config.address);
    printf("device_mqtt_config.address:%s\n", device_mqtt_config.address);
    printf("device_mqtt_config.port:%d\n", device_mqtt_config.port);
    printf("device_mqtt_config.restart_time:%d\n", device_mqtt_config.restart_time);
    printf("device_mqtt_config.tz_offset:%d\n", device_mqtt_config.tz_offset);
    printf("device_mqtt_config.client_id:%s\n", device_mqtt_config.client_id);
    printf("device_mqtt_config.username:%s\n", device_mqtt_config.username);
    printf("device_mqtt_config.password:%s\n", device_mqtt_config.password);
    printf("device_mqtt_config.subscribeTopic1:%s\n", device_mqtt_config.subscribeTopic1);
    printf("device_mqtt_config.subscribeTopic2:%s\n", device_mqtt_config.subscribeTopic2);
    printf("device_mqtt_config.subscribeTopic3:%s\n", device_mqtt_config.subscribeTopic3);
    printf("**************************************************\n");
}

void gizwits_create_token(char *random, char *token_str){
    char token[256] = "";
    char jl_token[64] = "ZhuHai JieLi Technology Co.,Ltd";
    memcpy(device_param.szPK, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy(device_param.device_mac, DEVICE_MAC, strlen(DEVICE_MAC));
    memcpy(device_param.szAuthKey, AUTH_KEY, strlen(AUTH_KEY));
    memcpy(device_param.device_id, DEVICE_ID, strlen(DEVICE_ID));
    memcpy(device_param.secret, DEVICE_SECRET, strlen(DEVICE_SECRET));
    //memcpy(device_param.nonce, random, strlen(random));


    strcat(token,device_param.szPK);
    strcat(token,",");
    strcat(token,device_param.device_mac);
    strcat(token,",");
    strcat(token,device_param.szAuthKey);
    strcat(token,",");
    strcat(token,random);


    u8 sha256_out[32];
    memset(sha256_out, 0, 32);
    printf("token: len: %d, %s\n", strlen(token), token);
    app_sha256(token, strlen(token), sha256_out);
    put_buf(sha256_out, 32);


    hex2str(sha256_out, 32, token_str);
    printf("token_str: %s\n", token_str);
}

void medtls_md5(char *input, u32 len, u8 *output){
    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts(&ctx);

    mbedtls_md5_update(&ctx, (const unsigned char *)input, strlen(len));
    // 计算最终的 MD5 哈希值
    mbedtls_md5_finish(&ctx, output);

    // 释放上下文资源
    mbedtls_md5_free(&ctx);
}

int gizwits_server_register_onboarding(void){
    printf("-----------------------------------------------------gizwits_server_register_onboarding\n");
    // stage 1: 生成随机码
    char random[HTTP_RANDOM_LEN+1] = {0};
    generate_random_number(random, HTTP_RANDOM_LEN);

    memset(user_curl_buf, 0, sizeof(user_curl_buf));
    snprintf(user_curl_buf, sizeof(user_curl_buf), ONBOARDING_URL, device_param.device_id);
    printf("Onboarding UAL: %s\n", user_curl_buf);

    // stage 2: 根据random生成sha 256 token
    char token_str[65] = {0};
    gizwits_create_token(random, token_str);

    struct wifi_mode_info info;
    info.mode = STA_MODE;
    wifi_get_mode_cur_info(&info);

    // char uid[33] = "63ad88e0768341db9729c2dee52eee91";
    prov_result_t *prov_result = get_wifi_prov_result();
    if(strlen(prov_result->uid) >= 32){
        memcpy(device_param.user_info.uid, prov_result->uid, 32);
        device_param_save(CFG_USER_DEVICE_INFO);
    }
    // for test
    // strcpy(prov_result->ssid, info.ssid);
    // strcpy(prov_result->pwd, info.pwd);
    // strcpy(prov_result->uid, "63ad88e0768341db9729c2dee52eee91");


    // stage 3: 生成md5随机码
    char aes_key[AES_KEY_LENGTH + 1];
    char sn[] = "00000000";
    generate_aes_key(device_param.szPK, aes_key);
    printf("prov_result->ssid %s", info.ssid);
    printf("prov_result->pwd  %s", info.pwd);
    printf("prov_result->uid %s", device_param.user_info.uid);
    printf("pk %s", device_param.szPK);
    printf("sn %s", sn);
    printf("aes_key %s", aes_key);
    // comput md5
    char md5_input[256] = {0};
    strcat(md5_input,info.ssid);
    strcat(md5_input,info.pwd);
    strcat(md5_input,sn);

    u8 md5_output[32] = {0};
    char md5_output_str[33] = {0};
    app_md5(md5_input, strlen(md5_input), md5_output);
    put_buf(md5_output, 16);
    hex2str(md5_output, 16, md5_output_str);

    
    printf("md5 input: %s", md5_input);
    printf("md5 output: %s", md5_output_str);
    put_buf(md5_output, 16);
    printf("base64 random code: %s", md5_output_str);


    // stage 4: 组装post body
    memset(user_body_buf, 0, sizeof(user_body_buf));
    snprintf(user_body_buf, sizeof(user_body_buf),ONBOARDING_BODY_DATA,md5_output_str, device_param.user_info.uid);

    // stage 5: 生成etag
    char etag_input[256] = {0};
    strcat(etag_input,device_param.szAuthKey);
    strcat(etag_input,",");
    strcat(etag_input,random);
    strcat(etag_input,",");
    strcat(etag_input,user_body_buf);

    u8 sha256_out[32] = {0};
    char etag_str[65] = {0};
    app_sha256(etag_input, strlen(etag_input), sha256_out);
    printf("sha256_out:");
    put_buf(sha256_out, 32);


    hex2str(sha256_out, 32, etag_str);
    printf("etag input: %s\n", etag_input);
    printf("etag: %s\n", etag_str);
    
    u8 flash_uid[16];
    memset(flash_uid, 0, sizeof(flash_uid));
    memcpy(flash_uid, get_norflash_uuid(), 16);
    printf("flash_uid:");
    put_buf(flash_uid, 16);
    char flash_uid_str[33] = {0};
    hex2str(flash_uid, 16, flash_uid_str);

    memset(user_head_buf, 0, sizeof(user_head_buf));
    snprintf(user_head_buf, sizeof(user_head_buf),GIZWITS_ONBARD_HEAD,
    device_param.device_id ,random, token_str, etag_str, flash_uid_str, strlen(user_body_buf));

    printf("\n========Onboarding request info===========\n");
    printf("Onboarding URL: %s\n", user_curl_buf);
    printf("Onboarding head: %s\n", user_head_buf);
    printf("Onboarding body: %s\n", user_body_buf);
    printf("=========================================\n");
    printf("\n");

    if (thread_fork("http_post_task", 10, 1024 * 10, 0, NULL, http_post_task, NULL) != OS_NO_ERR) {
        printf("thread fork fail\n");
        ai_send_event_to_sdk(AI_EVENT_MQTT_REG_FAIL, 0);
    }
}

int gizwits_server_register_provision(void){
    printf("ggggggggggggggggggggggggggggggggggggggggggggggggggg");
    printf("gizwits_server_register_provision\n");
    char random[HTTP_RANDOM_LEN+1] = {0};
    generate_random_number(random, HTTP_RANDOM_LEN);
    // a54283350726462daaeab498ffee87de,8caaaa8b11e0,b1223028af054bb1804d65f82dff26e7,VLDXFEQONBTKJXWG


    memset(user_curl_buf, 0, sizeof(user_curl_buf));
    memset(user_head_buf, 0, sizeof(user_head_buf));
    memset(user_body_buf, 0, sizeof(user_body_buf));
    snprintf(user_curl_buf, sizeof(user_curl_buf), ONBOARDING_URL, device_param.device_id);
    printf("Onboarding UAL: %s\n", user_curl_buf);


// stage 2: 根据random生成sha 256 token
    char token_str[65] = {0};
    gizwits_create_token(random, token_str);

    sprintf(user_head_buf, GIZWITS_HEAD, device_param.device_id, random, token_str);
    printf("user_head_buf is :%s\n", user_head_buf);
    sprintf(user_curl_buf, HTTP_GIZWITS_URL, device_param.device_id);
    printf("user_curl_buf is :%s\n", user_curl_buf);

    if (thread_fork("http_test_task", 10, 1024 * 10, 0, NULL, http_post_task, NULL) != OS_NO_ERR) {
        printf("thread fork fail\n");
        ai_send_event_to_sdk(AI_EVENT_MQTT_REG_FAIL, 0);
    }
}



void gizwits_login(void){
    printf("*********************************************************gizwits_login*********************************************************\n");

    //重新分配信息
    if(device_param.user_info.is_reset){
        printf("is_reset");
        gizwits_server_register_onboarding();
    }else{
        printf("mot_reset");
        //gizwits_server_register_onboarding();
        gizwits_server_register_provision();
    }
}


