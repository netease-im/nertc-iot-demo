#include "duer_common.h"

#if INTELLIGENT_DUER
static TokenData *duer_get_access_token_child()
{
    char url_buffer[256];
    snprintf(url_buffer, sizeof(url_buffer),
             "https://openapi.baidu.com/oauth/2.0/token?"
             "grant_type=client_credentials&"
             "client_id=%s&"
             "client_secret=%s",
             DUER_CLIENT_AK, DUER_CLIENT_SK);
    char *response = NULL;
    my_http_get_request(url_buffer, &response);
    if (!response) {
        MY_LOG_E("API request failed\n");
        return NULL;
    }
    TokenData *token = duer_parse_token_json(response);
    free(response);  // 无论成功与否都要释放响应
    if (!token) {
        MY_LOG_E("JSON parsing failed\n");
    }
    return token;
}

TokenData *duer_get_access_token()
{
    TokenData *token = duer_get_access_token_child();
    if (!token) {
        MY_LOG_E("Failed to get access token\n");
        return NULL;
    }
    MY_LOG_D("refresh_token: %s\n", token->refresh_token);
    MY_LOG_D("expires_in: %d\n", token->expires_in);
    MY_LOG_D("session_key: %s\n", token->session_key);
    MY_LOG_D("access_token: %s\n", token->access_token);
    MY_LOG_D("scope: %s\n", token->scope);
    MY_LOG_D("session_secret: %s\n", token->session_secret);
    return token;
}

#endif
