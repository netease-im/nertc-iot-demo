#include "duer_common.h"

#if INTELLIGENT_DUER
char *duer_start_frame(const char *appid, const char *appkey, const char *client_id, const char *pid,
                       const char *cuid, const char *format, int support_dcs, int sample,
                       const char *user_id, const char *access_token, int support_tts,
                       int support_text2dcs, const char *dialog_request_id, int access_rc,
                       const char *rc_version)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "type", cJSON_CreateString("start"));
    cJSON *data = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddItemToObject(data, "appid", cJSON_CreateString(appid));
    cJSON_AddItemToObject(data, "appkey", cJSON_CreateString(appkey));
    cJSON_AddItemToObject(data, "client_id", cJSON_CreateString(client_id));
    cJSON_AddItemToObject(data, "pid", cJSON_CreateString(pid));
    cJSON_AddItemToObject(data, "cuid", cJSON_CreateString(cuid));
    cJSON_AddItemToObject(data, "format", cJSON_CreateString(format));
    cJSON_AddItemToObject(data, "support_dcs", cJSON_CreateNumber(support_dcs));
    cJSON_AddItemToObject(data, "sample", cJSON_CreateNumber(sample));
    cJSON_AddItemToObject(data, "user_id", cJSON_CreateString(user_id));
    cJSON_AddItemToObject(data, "access_token", cJSON_CreateString(access_token));
    cJSON_AddItemToObject(data, "support_tts", cJSON_CreateBool(support_tts));
    cJSON_AddItemToObject(data, "support_text2dcs", cJSON_CreateBool(support_text2dcs));
    cJSON_AddItemToObject(data, "dialog_request_id", cJSON_CreateString(dialog_request_id));
    cJSON_AddItemToObject(data, "access_rc", cJSON_CreateBool(access_rc));
    cJSON_AddItemToObject(data, "rc_version", cJSON_CreateString(rc_version));
    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_string;
}



char *build_finish_frame()
{
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }
    cJSON_AddStringToObject(root, "type", "finish");
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}

char *duer_text_info_frame(const char *CUID,
                           const char *current_dialog_request_id,
                           const char *text_query)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        return NULL;
    }
    cJSON *event = cJSON_CreateObject();
    cJSON *header = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();
    cJSON *initiator = cJSON_CreateObject();
    if (!event || !header || !payload || !initiator) {
        cJSON_Delete(root);
        if (event) {
            cJSON_Delete(event);
        }
        if (header) {
            cJSON_Delete(header);
        }
        if (payload) {
            cJSON_Delete(payload);
        }
        if (initiator) {
            cJSON_Delete(initiator);
        }
        return NULL;
    }
    cJSON_AddStringToObject(root, "status", "ok");
    cJSON_AddStringToObject(root, "type", "dcs_decide");
    cJSON_AddStringToObject(root, "sn", CUID ? CUID : "");
    cJSON_AddNumberToObject(root, "end", 1);
    cJSON_AddItemToObject(root, "event", event);
    cJSON_AddItemToObject(event, "header", header);
    cJSON_AddItemToObject(event, "payload", payload);
    cJSON_AddStringToObject(header, "namespace", "ai.dueros.device_interface.screen");
    cJSON_AddStringToObject(header, "name", "LinkClicked");
    cJSON_AddStringToObject(header, "dialogRequestId", current_dialog_request_id ? current_dialog_request_id : "");
    char url[1024];
    if (text_query) {
        snprintf(url, sizeof(url), "dueros://server.dueros.ai/query?q=%s", text_query);
    } else {
        snprintf(url, sizeof(url), "dueros://server.dueros.ai/query");
    }
    cJSON_AddStringToObject(payload, "url", url);
    cJSON_AddItemToObject(payload, "initiator", initiator);
    cJSON_AddStringToObject(initiator, "type", "USER_CLICK");
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    return json_str;
}


#endif
