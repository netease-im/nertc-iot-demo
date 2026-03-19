#include <string.h>
#include <time.h>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <curl/curl.h>
#include <cJSON.h>

#define RTC_API_HOST "rtc.volcengineapi.com"
#define AK 			 "AKL"
#define SK 			 "="
#define ASR_APP_ID   ""
#define TTS_APP_ID 	 ""
#define LLM_APP_ID 	 ""
#define RTC_APP_ID   ""
#define RTC_APP_KEY  ""
#define BOT_ID       ""

typedef struct {
    char room_id[129];
    char uid[129];
    char app_id[25];
    char token[257];
    char task_id[129];
} rtc_room_info_t;

#if 1
#define MAX_TIME_LEN 20
#define SHA256_HEX_LEN 65
#define MAX_HEADER_LEN 1024

// HMAC-SHA256计算
int hmac_sha256_(const unsigned char *key, size_t keylen,
                 const unsigned char *data, size_t datalen,
                 unsigned char output[32])
{
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    return mbedtls_md_hmac(md, key, keylen, data, datalen, output);
}

// SHA256哈希计算
void sha256_hash(const char *content, unsigned char output[32])
{
    mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
               (const unsigned char *)content, strlen(content), output);
}

// 生成当前UTC时间戳
void get_x_date(char buffer[MAX_TIME_LEN])
{
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(buffer, MAX_TIME_LEN, "%Y%m%dT%H%M%SZ", tm);
}

// 生成规范请求
char *build_canonical_request(
    const char *method,
    const char *uri,
    const char *query,
    const char *content_type,
    const char *host,
    const char *x_content_sha256,
    const char *x_date,
    char **signed_headers_out)
{

    /* printf("x_content_sha256: %s\n", x_content_sha256); */

    /* printf("%s %d \n", __FUNCTION__, __LINE__); */
    // 构造规范头
    char canonical_headers[512] = {0};
    snprintf(canonical_headers, sizeof(canonical_headers),
             "content-type:%s\nhost:%s\nx-content-sha256:%s\nx-date:%s\n",
             content_type, host, x_content_sha256, x_date);
    /* printf("canonical_headers:\n%s\n", canonical_headers); */

    // 构造签名头
    *signed_headers_out = strdup("content-type;host;x-content-sha256;x-date");

    // 构造规范请求
    size_t len = snprintf(NULL, 0, "%s\n%s\n%s\n%s\n%s\n%s",
                          method, uri, query, canonical_headers, *signed_headers_out, x_content_sha256);

    char *canonical = malloc(len + 1);
    snprintf(canonical, len + 1, "%s\n%s\n%s\n%s\n%s\n%s",
             method, uri, query, canonical_headers, *signed_headers_out, x_content_sha256);
    // printf("canonical_headers:\n%s\n", canonical_headers);
    /* printf("canonical:\n%s\n", canonical); */
    return canonical;
}

// 生成待签字符串
char *build_string_to_sign(const char *x_date, const char *hashed_canonical)
{
    char credential_scope[64];
    char x_date_[9] = {0};
    strncpy(x_date_, x_date, 8);
    snprintf(credential_scope, sizeof(credential_scope), "%.8s/cn-north-1/rtc/request", x_date_);
    // snprintf(credential_scope, sizeof(credential_scope), "%.8s/cn-north-1/rtc/request", x_date);

    size_t len = snprintf(NULL, 0, "HMAC-SHA256\n%s\n%s\n%s", x_date, credential_scope, hashed_canonical);

    char *str = malloc(len + 1);
    snprintf(str, len + 1, "HMAC-SHA256\n%s\n%s\n%s",
             x_date, credential_scope, hashed_canonical);

    return str;
}

// 生成签名
char *generate_signature(const char *sk, const char *credential_scope, const char *string_to_sign)
{
    // 分割credential_scope
    char *parts[4];
    char cred_scope_copy[256];
    strncpy(cred_scope_copy, credential_scope, sizeof(cred_scope_copy));
    char *token = strtok(cred_scope_copy, "/");
    for (int i = 0; i < 4 && token != NULL; i++) {
        parts[i] = token;
        token = strtok(NULL, "/");
    }

    // 构建HMAC内容数组
    const char *hmac_contents[] = {
        parts[0], parts[1], parts[2], parts[3], string_to_sign
    };
    const int hmac_contents_count = 5;

    // HMAC计算
    unsigned char signature[32];
    unsigned char temp_sig[32];
    mbedtls_md_context_t ctx;

    // 初始密钥（直接使用SK的UTF-8字节）
    size_t key_len = strlen(sk);
    unsigned char *key = (unsigned char *)sk;

    for (int i = 0; i < hmac_contents_count; i++) {
        const char *data = hmac_contents[i];
        size_t data_len = strlen(data);

        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

        mbedtls_md_hmac_starts(&ctx, key, key_len);
        mbedtls_md_hmac_update(&ctx, (const unsigned char *)data, data_len);
        mbedtls_md_hmac_finish(&ctx, temp_sig);

        // 更新密钥为本次计算结果
        key = temp_sig;
        key_len = 32;

        mbedtls_md_free(&ctx);
    }

    // 转换为十六进制输出
    char hex_sig[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hex_sig + 2 * i, "%02x", temp_sig[i]);
    }
    hex_sig[64] = '\0';
    // printf("hex_sig: %s\n", hex_sig);
    return hex_sig;
}

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    char **buf = (char **)userdata;
    size_t new_len = size * nmemb;
    size_t cur_len = *buf ? strlen(*buf) : 0;

    *buf = realloc(*buf, cur_len + new_len + 1);
    if (*buf) {
        memcpy(*buf + cur_len, ptr, new_len);
        (*buf)[cur_len + new_len] = 0;
    }
    return new_len;
}

// 发送HTTP请求
int request_rtc_api(
    const char *host,
    const char *method,
    const char *uri,
    const char *query,
    cJSON *headers_json,
    const char *body,
    const char *ak,
    const char *sk)
{

    // 生成时间戳
    char x_date[MAX_TIME_LEN];
    get_x_date(x_date);
    char *response = NULL;
    // 计算内容哈希
    unsigned char body_hash[32];
    sha256_hash(body, body_hash);
    char body_hash_hex[SHA256_HEX_LEN];
    for (int i = 0; i < 32; i++) {
        sprintf(body_hash_hex + i * 2, "%02x", body_hash[i]);
    }
    body_hash_hex[64] = '\0';
    // printf("body_hash_hex: %s\n", body_hash_hex);

    // 构建规范请求
    char *signed_headers;

    char *canonical = build_canonical_request(
                          method, uri, query, "application/json; charset=utf-8", host,
                          body_hash_hex, x_date, &signed_headers);

    // printf("%s %d \n", __FUNCTION__, __LINE__);
    // 哈希规范请求
    unsigned char hashed_canonical[32];
    sha256_hash(canonical, hashed_canonical);
    char hashed_canonical_hex[SHA256_HEX_LEN];
    for (int i = 0; i < 32; i++) {
        sprintf(hashed_canonical_hex + i * 2, "%02x", hashed_canonical[i]);
    }
    hashed_canonical_hex[64] = 0;

    // 构建待签字符串
    char *string_to_sign = build_string_to_sign(x_date, hashed_canonical_hex);

    // 生成凭证范围
    char credential_scope[64];
    char x_date_[9] = {0};
    strncpy(x_date_, x_date, 8);

    snprintf(credential_scope, sizeof(credential_scope), "%.8s/cn-north-1/rtc/request", x_date_);

    // 计算签名
    char *signature = generate_signature(sk, credential_scope, string_to_sign);
    // printf("signature: %s\n", signature);
    // 生成Authorization头
    char auth_header[MAX_HEADER_LEN];
    snprintf(auth_header, sizeof(auth_header),
             "HMAC-SHA256 Credential=%s/%s, SignedHeaders=%s, Signature=%s", ak, credential_scope, signed_headers, signature);

    // 构造请求URL
    char url[512];
    snprintf(url, sizeof(url), "https://%s%s%s%s",
             host, (uri[0] == '/') ? "" : uri,
             query[0] ? "?" : "", query);


    // printf("url: %s\n", url);
    CURLcode res;
    struct curl_slist *headers = NULL;
    char header_buffer[MAX_HEADER_LEN];
    // 设置CURL
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if (curl) {
        // 设置URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // 构建headers
        headers = curl_slist_append(headers, "Content-Type:application/json; charset=utf-8");
        headers = curl_slist_append(headers, "Host:rtc.volcengineapi.com");

        // 动态构造 X-Content-Sha256 头
        char sha256_header[256];
        snprintf(sha256_header, sizeof(sha256_header), "X-Content-Sha256:%s", body_hash_hex);
        headers = curl_slist_append(headers, sha256_header);

        // 动态构造 X-Date 头
        char xdate_header[256];
        snprintf(xdate_header, sizeof(xdate_header), "X-Date:%s", x_date);
        headers = curl_slist_append(headers, xdate_header);

        // 动态构造 Authorization 头
        char auth_header_[512];
        snprintf(auth_header_, sizeof(auth_header_), "Authorization:%s", auth_header);
        headers = curl_slist_append(headers, auth_header_);

        struct curl_slist *item = headers;
        // printf("打印headers:\n");
        // while (item) {
        //     printf("%s\n", item->data);
        //     item = item->next;
        // }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        // 设置回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        // 在 curl_easy_perform 调用前添加：
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 禁用对等证书验证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // 禁用主机名验证
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("API Response: %s\n", response);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    free(response);
    // 清理资源
    free(canonical);
    free(signed_headers);
    free(string_to_sign);

    return (int)res;
}
#endif

char *start_voice_chat_json(const char *app_id, const char *room_id, const char *user_id)
{
    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "AppId", app_id);
    cJSON_AddStringToObject(body, "RoomId", room_id);
    cJSON_AddStringToObject(body, "UserId", user_id);
    cJSON_AddStringToObject(body, "TaskId", "task1");

    cJSON *config = cJSON_CreateObject();
    cJSON_AddNumberToObject(config, "InterruptMode", 0);
    cJSON *asrConfig = cJSON_CreateObject();
    cJSON_AddStringToObject(asrConfig, "Provider", "volcano");
    cJSON *ProviderParams = cJSON_CreateObject();
    cJSON_AddStringToObject(ProviderParams, "Mode", "smallmodel");
    cJSON_AddStringToObject(ProviderParams, "AppId", ASR_APP_ID);
    cJSON_AddStringToObject(ProviderParams, "Cluster", "volcengine_streaming_common");
    cJSON_AddItemToObject(asrConfig, "ProviderParams", ProviderParams);
    cJSON *VADConfig = cJSON_CreateObject();
    cJSON_AddNumberToObject(VADConfig, "SilenceTime", 500);
    cJSON_AddItemToObject(asrConfig, "VADConfig", VADConfig);

    cJSON_AddNumberToObject(asrConfig, "VolumeGain", 0.3);
    cJSON_AddItemToObject(config, "ASRConfig", asrConfig);

    cJSON *TTSConfig = cJSON_CreateObject();
    cJSON_AddStringToObject(TTSConfig, "Provider", "volcano");

    cJSON *ProviderParams2 = cJSON_CreateObject();
    cJSON *app = cJSON_CreateObject();
    cJSON_AddStringToObject(app, "appid", TTS_APP_ID);
    cJSON_AddStringToObject(app, "cluster", "volcano_tts");
    cJSON_AddItemToObject(ProviderParams2, "app", app);
    cJSON_AddItemToObject(TTSConfig, "ProviderParams", ProviderParams2);
    cJSON_AddItemToObject(config, "TTSConfig", TTSConfig);

    cJSON *LLMConfig = cJSON_CreateObject();
    cJSON_AddStringToObject(LLMConfig, "Mode", "ArkV3");
    cJSON_AddStringToObject(LLMConfig, "EndPointId", LLM_APP_ID);
    /* cJSON_AddStringToObject(LLMConfig, "BotId", BOT_ID); */
    cJSON_AddItemToObject(config, "LLMConfig", LLMConfig);
    cJSON_AddItemToObject(body, "Config", config);

    cJSON *AgentConfig = cJSON_CreateObject();

    // 创建TargetUserId数组
    cJSON *target_user_id = cJSON_CreateArray();
    cJSON_AddItemToArray(target_user_id, cJSON_CreateString(user_id));

    // 将TargetUserId数组添加到AgentConfig对象中
    cJSON_AddItemToObject(AgentConfig, "TargetUserId", target_user_id);

    cJSON_AddStringToObject(AgentConfig, "WelcomeMessage", "\u4f60\u597d");
    cJSON_AddStringToObject(AgentConfig, "UserId", "test_user_001");
    cJSON_AddItemToObject(body, "AgentConfig", AgentConfig);
    // 添加其他字段和签名逻辑
    char *json = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    return json;
}

char *stop_voice_chat_json(const char *app_id, const char *room_id, const char *user_id)
{
    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "AppId", app_id);
    cJSON_AddStringToObject(body, "RoomId", room_id);
    cJSON_AddStringToObject(body, "UserId", user_id);
    cJSON_AddStringToObject(body, "TaskId", "task1");
    char *json = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    return json;
}

char *update_voice_chat_json(const char *app_id, const char *room_id, const char *user_id)
{
    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "AppId", app_id);
    cJSON_AddStringToObject(body, "RoomId", room_id);
    cJSON_AddStringToObject(body, "TaskId", "task1");
    cJSON_AddStringToObject(body, "Command", "interrupt");
    char *json = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    return json;
}

char *generate_random_string(int length)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char *str = malloc(length + 1);

    if (str == NULL) {
        return NULL;
    }

    // 在第一次调用前初始化随机种子（只需在整个程序中执行一次）
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));  // 使用当前时间作为种子
        seeded = 1;
    }

    for (int i = 0; i < length; i++) {
        int index = rand() % (sizeof(charset) - 1);
        str[i] = charset[index];
    }
    str[length] = '\0';

    return str;
}

int start_voice_chat(rtc_room_info_t *room_info)
{
    char *temp = generate_random_string(10);
    char room_id[100];
    char user_id[100];
    snprintf(room_id, sizeof(room_id), "G711A%s", temp);
    snprintf(user_id, sizeof(user_id), "user%s", temp);
    char *json_body = start_voice_chat_json(RTC_APP_ID, room_id, user_id);
    char query_string[50];
    snprintf(query_string, sizeof(query_string), "Action=%s&Version=%s", "StartVoiceChat", "2024-12-01");
    int ret = request_rtc_api("rtc.volcengineapi.com", "POST", "/", query_string, NULL, json_body, AK, SK);
    /* int ret = 0; */
    char *token = get_token(RTC_APP_ID, RTC_APP_KEY, room_id, user_id);
    strcpy(room_info->app_id, RTC_APP_ID);
    strcpy(room_info->uid, user_id);
    strcpy(room_info->room_id, room_id);
    strcpy(room_info->token, token);
    strcpy(room_info->task_id, "task1");
    free(token);
    free(json_body);
    free(temp);
    return ret;
}

int stop_voice_chat(const rtc_room_info_t *room_info)
{
    char *json_body = stop_voice_chat_json(room_info->app_id, room_info->room_id, room_info->uid);
    char query_string[50];
    snprintf(query_string, sizeof(query_string), "Action=%s&Version=%s", "StopVoiceChat", "2024-12-01");
    int ret = request_rtc_api("rtc.volcengineapi.com", "POST", "/", query_string, NULL, json_body, AK, SK);
    free(json_body);
    return ret;
}

int update_voice_chat(const rtc_room_info_t *room_info)
{
    char *json_body = update_voice_chat_json(room_info->app_id, room_info->room_id, room_info->uid);
    char query_string[50];
    snprintf(query_string, sizeof(query_string), "Action=%s&Version=%s", "UpdateVoiceChat", "2024-12-01");
    int ret = request_rtc_api("rtc.volcengineapi.com", "POST", "/", query_string, NULL, json_body, AK, SK);
    free(json_body);
    return ret;
}

