#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>

#define VERSION "001"
#define VERSION_LENGTH 3
#define APP_ID_LENGTH 24

typedef enum {
    PrivPublishStream = 0,
    privPublishAudioStream = 1,
    privPublishVideoStream = 2,
    privPublishDataStream = 3,
    PrivSubscribeStream = 4
} Privileges;

typedef struct {
    char *app_id;
    char *app_key;
    char *room_id;
    char *user_id;
    uint32_t issued_at;
    uint32_t nonce;
    uint32_t expire_at;
    uint32_t privileges[16]; // [privilege_type] = expire_ts
} AccessToken;

typedef struct {
    const unsigned char *data;
    size_t length;
    size_t position;
} ByteBuffer;

// 辅助函数声明
void pack_uint16(uint16_t value, unsigned char *buf);
void pack_uint32(uint32_t value, unsigned char *buf);
size_t pack_string(const char *str, unsigned char *buf);
size_t pack_map_uint32(const uint32_t *privileges, unsigned char *buf);
void free_token(AccessToken *token);

// 初始化AccessToken
AccessToken *token_init(const char *app_id, const char *app_key,
                        const char *room_id, const char *user_id)
{
    AccessToken *token = malloc(sizeof(AccessToken));
    memset(token, 0, sizeof(AccessToken));

    token->app_id = strdup(app_id);
    token->app_key = strdup(app_key);
    token->room_id = strdup(room_id);
    token->user_id = strdup(user_id);
    token->issued_at = (uint32_t)time(NULL);
    token->nonce = rand() % 99999999 + 1;
    token->expire_at = 0;
    memset(token->privileges, 0, sizeof(token->privileges));

    return token;
}

// 添加权限
void token_add_privilege(AccessToken *token, Privileges priv, uint32_t expire_ts)
{
    if (priv >= 0 && priv < 16) {
        token->privileges[priv] = expire_ts;
        if (priv == PrivPublishStream) {
            token->privileges[privPublishAudioStream] = expire_ts;
            token->privileges[privPublishVideoStream] = expire_ts;
            token->privileges[privPublishDataStream] = expire_ts;
        }
    }
}

// 打包消息
unsigned char *pack_message(AccessToken *token, size_t *msg_len)
{
    // 计算各部分大小
    size_t room_len = strlen(token->room_id);
    size_t user_len = strlen(token->user_id);

    // 计算总长度
    *msg_len = 4 + 4 + 4 + 2 + room_len + 2 + user_len + 2;
    for (int i = 0; i < 16; i++) {
        if (token->privileges[i] != 0) {
            *msg_len += 2 + 4;
        }
    }

    unsigned char *msg = malloc(*msg_len);
    unsigned char *p = msg;

    pack_uint32(token->nonce, p);
    p += 4;
    pack_uint32(token->issued_at, p);
    p += 4;
    pack_uint32(token->expire_at, p);
    p += 4;
    p += pack_string(token->room_id, p);
    p += pack_string(token->user_id, p);

    // Pack privileges
    unsigned char map_buf[512];
    size_t map_len = pack_map_uint32(token->privileges, map_buf);
    memcpy(p, map_buf, map_len);
    p += map_len;

    return msg;
}

// 生成token字符串
char *token_serialize(AccessToken *token)
{
    size_t msg_len;
    unsigned char *msg = pack_message(token, &msg_len);

    // 计算HMAC
    unsigned char hmac[32];
    mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                    (const unsigned char *)token->app_key, strlen(token->app_key),
                    msg, msg_len, hmac);

    // 打包内容：msg + hmac
    size_t content_len = 2 + msg_len + 2 + sizeof(hmac);
    unsigned char *content = malloc(content_len);
    unsigned char *p = content;

    pack_uint16(msg_len, p);
    p += 2;
    memcpy(p, msg, msg_len);
    p += msg_len;
    pack_uint16(sizeof(hmac), p);
    p += 2;
    memcpy(p, hmac, sizeof(hmac));
    p += sizeof(hmac);

    // Base64编码
    size_t b64_len;
    mbedtls_base64_encode(NULL, 0, &b64_len, content, content_len);
    char *b64 = malloc(strlen(VERSION) + APP_ID_LENGTH + b64_len + 1);

    char *ptr = b64;
    memcpy(ptr, VERSION, VERSION_LENGTH);
    ptr += VERSION_LENGTH;
    memcpy(ptr, token->app_id, APP_ID_LENGTH);
    ptr += APP_ID_LENGTH;

    mbedtls_base64_encode((unsigned char *)ptr, b64_len, &b64_len, content, content_len);
    ptr[b64_len] = '\0';

    free(msg);
    free(content);
    return b64;
}

// 辅助函数实现
void pack_uint16(uint16_t value, unsigned char *buf)
{
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
}

void pack_uint32(uint32_t value, unsigned char *buf)
{
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
}

size_t pack_string(const char *str, unsigned char *buf)
{
    size_t len = strlen(str);
    pack_uint16(len, buf);
    memcpy(buf + 2, str, len);
    return 2 + len;
}

size_t pack_map_uint32(const uint32_t *privileges, unsigned char *buf)
{
    unsigned char *start = buf;
    uint16_t count = 0;
    buf += 2; // 预留位置写count

    for (int i = 0; i < 16; i++) {
        if (privileges[i] != 0) {
            pack_uint16(i, buf);
            pack_uint32(privileges[i], buf + 2);
            buf += 6;
            count++;
        }
    }

    pack_uint16(count, start);
    return 2 + count * 6;
}

void free_token(AccessToken *token)
{
    free(token->app_id);
    free(token->app_key);
    free(token->room_id);
    free(token->user_id);
    free(token);
}

char *get_token(char *app_id, char *app_key, char *room_id, char *user_id)
{
    srand(time(NULL));
    /* AccessToken* token = token_init(RTC_APP_ID, RTC_APP_KEY, ROOM_ID, USER_ID); */
    AccessToken *token = token_init(app_id, app_key, room_id, user_id);
    token_add_privilege(token, PrivPublishStream, time(NULL) + 3600); //设置token存活时间
    printf("time:%d", time(NULL));
    char *token_str = token_serialize(token);
    printf("Generated Token: %s\n", token_str);

    free_token(token);
    return token_str;
}


