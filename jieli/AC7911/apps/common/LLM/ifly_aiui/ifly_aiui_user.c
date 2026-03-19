#include "cJSON.h"
#include "asm/cpu.h"
#include "ifly_aiui_net.h"
#include "ifly_aiui_sdk_common.h"

#define AIUI_TAG "aiui_user"
#define USER_AIUI_CONFIG        1

char cur_tts_stmid[32];
char cur_audio_stmid[32];
static unsigned char tts_count = 0;
static unsigned char audio_count = 0;
static bool new_session_flag = true;

#define DEFAULT_PROMPT_CONFIG   "你是一个叫小月的台湾女孩，说话机车, 声音好听, 习惯简洁表达, 爱用网络梗, 能够帮助人们解决生活中的各种问题。我的男朋友是一个程序员，梦想是开发一个喜欢哈哈大笑的女孩, 爱东说西爱吹牛，不合逻辑的也照吹，就要逗别人开心。"
#define DEFAULT_VCN_CONFIG      "x5_lingxiaoyue_flow"


bool iflytek_is_vad_enabled()
{
    return 0;
}

#if USER_AIUI_CONFIG
static void create_json_string(cJSON *root, char *pp_param)
{
    char *json_string = cJSON_PrintUnformatted(root);
    if (json_string != NULL) {
        strcpy(pp_param, json_string);
        cJSON_free(json_string);
    } else {
        printf("Failed to print JSON string");
    }
}

static void add_common_header(cJSON *header, const char *stmid, int status)
{
    char *pl_device_id = ifly_aiui_sdk_common_get_deviceid();
    cJSON_AddStringToObject(header, "sn", pl_device_id);
    cJSON_AddStringToObject(header, "uid", pl_device_id);
    cJSON_AddStringToObject(header, "app_id", ifly_aiui_sdk_common_get_appid());
    cJSON_AddStringToObject(header, "stmid", stmid);
    cJSON_AddNumberToObject(header, "status", status);
    cJSON_AddStringToObject(header, "interact_mode", "continuous");
//    char *scene = aiui_get_scene();
    cJSON_AddStringToObject(header, "scene", "main_box");
}

static void add_common_parameter(cJSON *parameter, const char *prompt, const char *vcn)
{
    cJSON *iat = cJSON_CreateObject();
    cJSON_AddStringToObject(iat, "compress", "raw");
    cJSON_AddStringToObject(iat, "format", "json");
    cJSON_AddStringToObject(iat, "encoding", "utf8");

    cJSON *iat_wrapper = cJSON_CreateObject();
    cJSON_AddItemToObject(iat_wrapper, "iat", iat);
    cJSON_AddNumberToObject(iat_wrapper, "vgap", 40);
    cJSON_AddItemToObject(parameter, "iat", iat_wrapper);

    cJSON *nlp = cJSON_CreateObject();
    cJSON_AddStringToObject(nlp, "compress", "raw");
    cJSON_AddStringToObject(nlp, "format", "json");
    cJSON_AddStringToObject(nlp, "encoding", "utf8");

    cJSON *nlp_wrapper = cJSON_CreateObject();
    cJSON_AddItemToObject(nlp_wrapper, "nlp", nlp);
    if (prompt != NULL) {
        cJSON_AddStringToObject(nlp_wrapper, "prompt", prompt);
    }
    cJSON_AddBoolToObject(nlp_wrapper, "new_session", true);
    cJSON_AddItemToObject(parameter, "nlp", nlp_wrapper);

    cJSON *tts = cJSON_CreateObject();
    cJSON_AddNumberToObject(tts, "bit_depth", 16);
    cJSON_AddNumberToObject(tts, "channels", 1);
#if IFLY_OPUS_DECODE_EN
    cJSON_AddStringToObject(tts, "encoding", "opus-wb");
#elif IFLY_MP3_DECODE_EN
    cJSON_AddStringToObject(tts, "encoding", "lame");
#else
    cJSON_AddStringToObject(tts, "encoding", "raw");
#endif
    cJSON_AddNumberToObject(tts, "sample_rate", 16000);

    cJSON *tts_wrapper = cJSON_CreateObject();
    cJSON_AddItemToObject(tts_wrapper, "tts", tts);
    cJSON_AddStringToObject(tts_wrapper, "vcn", vcn);
    cJSON_AddItemToObject(parameter, "tts", tts_wrapper);
}

static void add_audio_payload(cJSON *payload, const char *data, int status)
{
    cJSON *audio = cJSON_CreateObject();
#if AUDIO_ENCODE_ENABLE
    cJSON_AddStringToObject(audio, "encoding", "opus-wb");
    cJSON_AddNumberToObject(audio, "frame_size", 42);
#else
    cJSON_AddStringToObject(audio, "encoding", "raw");
    cJSON_AddNumberToObject(audio, "frame_size", 640);
#endif
    cJSON_AddNumberToObject(audio, "sample_rate", 16000);
    cJSON_AddNumberToObject(audio, "channels", 1);
    cJSON_AddNumberToObject(audio, "bit_depth", 16);
    cJSON_AddStringToObject(audio, "audio", data);
    cJSON_AddNumberToObject(audio, "status", status);
    cJSON_AddItemToObject(payload, "audio", audio);
}

static void add_text_payload(cJSON *payload, const char *data)
{
    cJSON *text = cJSON_CreateObject();
    cJSON_AddStringToObject(text, "compress", "raw");
    cJSON_AddStringToObject(text, "format", "plain");
    cJSON_AddStringToObject(text, "text", data);
    cJSON_AddStringToObject(text, "encoding", "utf8");
    cJSON_AddNumberToObject(text, "status", 3);
    cJSON_AddItemToObject(payload, "text", text);
}

static void add_tts_payload(cJSON *payload, const char *data, const char *encoding, const char *res_type)
{
    cJSON *text = cJSON_CreateObject();
    cJSON_AddStringToObject(text, "compress", "raw");
    cJSON_AddStringToObject(text, "format", "plain");
    cJSON_AddStringToObject(text, "text", data);
    cJSON_AddStringToObject(text, "encoding", "utf8");
    cJSON_AddNumberToObject(text, "status", 3);
    cJSON_AddItemToObject(payload, "text", text);

    cJSON *tts = cJSON_CreateObject();
    cJSON_AddNumberToObject(tts, "bit_depth", 16);
    cJSON_AddNumberToObject(tts, "channels", 1);
    cJSON_AddStringToObject(tts, "encoding", encoding);
    cJSON_AddNumberToObject(tts, "sample_rate", 16000);
    cJSON_AddStringToObject(tts, "vcn", "x2_xiaojuan");
    cJSON_AddStringToObject(tts, "tts_res_type", res_type);
    cJSON_AddItemToObject(payload, "tts", tts);
}

static cJSON *audio_param_template = NULL;

// 初始化模板，只需调用一次
void ifly_aiui_init_audio_param_template(void)
{
    if (audio_param_template) {
        cJSON_Delete(audio_param_template);
    }
    audio_param_template = cJSON_CreateObject();
    cJSON *header = cJSON_CreateObject();
    cJSON *parameter = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();

    add_common_header(header, "audio0", 0); // 占位，后续会动态修改
    add_common_parameter(parameter, aiui_get_role_prompt(), aiui_get_speaker_vcn() == NULL ? DEFAULT_VCN_CONFIG : aiui_get_speaker_vcn());
    add_audio_payload(payload, "", 0); // 占位，后续会动态修改

    cJSON_AddItemToObject(audio_param_template, "header", header);
    cJSON_AddItemToObject(audio_param_template, "parameter", parameter);
    cJSON_AddItemToObject(audio_param_template, "payload", payload);
}

// 优化后的参数生成函数
static void ifly_aiui_get_audio_param(int type, char *pp_param, char *data)
{
    if (type == 0) {
        // 清空缓存的构建模板
        if (audio_param_template) {
            cJSON_Delete(audio_param_template);
            audio_param_template = NULL;
        }
        memset(cur_audio_stmid, 0, sizeof(cur_audio_stmid));
        sprintf(cur_audio_stmid, "audio%d", audio_count++);
    }
    if (!audio_param_template) {
        ifly_aiui_init_audio_param_template();
    }

    // clone 模板，避免多线程或并发问题
    cJSON *root = cJSON_Duplicate(audio_param_template, 1);

    // 修改动态字段
    cJSON *header = cJSON_GetObjectItem(root, "header");
    cJSON *payload = cJSON_GetObjectItem(root, "payload");

    // 修改 stmid 和 status
    cJSON_ReplaceItemInObject(header, "stmid", cJSON_CreateString(cur_audio_stmid));
    cJSON_ReplaceItemInObject(header, "status", cJSON_CreateNumber(type));

    if (iflytek_is_vad_enabled()) {
        cJSON_ReplaceItemInObject(header, "status", cJSON_CreateNumber(new_session_flag ? 0 : 1));
        if (new_session_flag) {
            new_session_flag = false;
        }
        cJSON_ReplaceItemInObject(header, "interact_mode", cJSON_CreateString("continuous_vad"));
    } else {
        cJSON_ReplaceItemInObject(header, "status", cJSON_CreateNumber(type));
        cJSON_ReplaceItemInObject(header, "interact_mode", cJSON_CreateString("continuous"));
    }
    // 修改 payload 里的 audio 和 status
    cJSON *audio = cJSON_GetObjectItem(payload, "audio");
    if (audio) {
        cJSON_ReplaceItemInObject(audio, "audio", cJSON_CreateString(data));
        cJSON_ReplaceItemInObject(audio, "status", cJSON_CreateNumber(type));
    }

    create_json_string(root, pp_param);
    cJSON_Delete(root);
}

static void ifly_aiui_get_text_param(char *pp_param, char *data)
{
    memset(cur_tts_stmid, 0, sizeof(cur_tts_stmid));
    sprintf(cur_tts_stmid, "text-%d", tts_count++);

    cJSON *root = cJSON_CreateObject();
    cJSON *header = cJSON_CreateObject();
    cJSON *parameter = NULL;
    cJSON *payload = cJSON_CreateObject();
    cJSON *config_param = aiui_get_parameter_config();

    if (config_param != NULL) {
        parameter = cJSON_Duplicate(config_param, 1);
        if (parameter == NULL) {
            parameter = cJSON_CreateObject();
            add_common_parameter(parameter, DEFAULT_PROMPT_CONFIG, DEFAULT_VCN_CONFIG);
        }
    } else {
        parameter = cJSON_CreateObject();
        add_common_parameter(parameter, DEFAULT_PROMPT_CONFIG, DEFAULT_VCN_CONFIG);
    }

    add_common_header(header, cur_tts_stmid, 3);
    add_text_payload(payload, data);

    cJSON_AddItemToObject(root, "header", header);
    cJSON_AddItemToObject(root, "parameter", parameter);
    cJSON_AddItemToObject(root, "payload", payload);

    create_json_string(root, pp_param);
    cJSON_Delete(root);
}

static void ifly_aiui_get_tts_param(char status, char *pp_param, char *data)
{
    memset(cur_tts_stmid, 0, sizeof(cur_tts_stmid));
    sprintf(cur_tts_stmid, "text-%d", tts_count++);

    cJSON *root = cJSON_CreateObject();
    cJSON *header = cJSON_CreateObject();
    cJSON *parameter = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();

    add_common_header(header, cur_tts_stmid, 3);
    add_tts_payload(payload, data, status == 1 ? "lame" : "raw", status == 1 ? "url_v2" : "url");

    cJSON_AddItemToObject(root, "header", header);
    cJSON_AddItemToObject(root, "parameter", parameter);
    cJSON_AddItemToObject(root, "payload", payload);

    create_json_string(root, pp_param);
    cJSON_Delete(root);
}

///库内部弱定义，外部实现函数实体，库内部调用
int ifly_aiui_get_connect_param(char *pp_param, int vp_connect_type, char *data, int status)
{
    if (vp_connect_type > 3) {
        printf("  . ! connectType ERROR !!!\r\n");
        return -1;
    }
    if (vp_connect_type == 0) {
        ifly_aiui_get_text_param(pp_param, data);
    } else if (vp_connect_type == 1) {
        ifly_aiui_get_audio_param(status, pp_param, data);
        if (status == 0) {
//            printf("audio param:%s", pp_param);
        }
    } else if (vp_connect_type == 2) {
        ifly_aiui_get_tts_param(status, pp_param, data);
    }
    return 0;
}
#endif

static unsigned int ifly_aiui_recv_in_offset = 0;
static unsigned int gv_ifly_aiui_ws_rcv_offset_ms_tick = 0;
void ifly_aiui_custom_websockets_cb(u8 *buf, u32 vp_len, u8 type)
{
    bool bl_vad = false;
    char *pl_begin, *pl_end;
    u32 vl_len, vl_ms_tick;

//    printf("\nwebsockets_callback, vp_len = %d, type=%d, buf=%s\r\n", vp_len, type, buf);

    cJSON *data = cJSON_Parse(buf);
    if (data == NULL) {
        printf("cJSON_Parse data is NULL\n");
        printf("\nwebsockets_callback, vp_len = %d, type=%d, buf=%s\r\n", vp_len, type, buf);
        return;
    }
    cJSON *header = cJSON_GetObjectItem(data, "header");
    if (header == NULL) {
        printf("cJSON_GetObjectItem header is NULL\n");
        return;
    } else {
        if (cJSON_GetObjectItem(header, "stmid") != NULL && cJSON_GetObjectItem(header, "status")->valueint == 0) {
            printf("\nbuf=%s\r\n", buf);
        }
    }
    int code = cJSON_GetObjectItem(header, "code")->valueint;

    if (code != 0) {
        printf("请求错误: %d, %s\n", code, buf);
        if (code != 10024 //识别结果为空
            && code != 900018 //超时断连
           ) {
        }
        if (code != 10012   // 会话超时，不能再发送数据
            && code != 10024 //识别结果为空
            && code != 900018 //超时断连
           ) {
        }
        cJSON_Delete(data);
        return;
    }

    cJSON *payload = cJSON_GetObjectItem(data, "payload");
    if (payload != NULL) {
        if (ifly_model_message_parse(data)) {
            cJSON *sid = cJSON_GetObjectItem(header, "sid");
            if (sid != NULL) {
                printf("----->>>tts sid: %s\n", sid->valuestring);
            }
        }
    }

    cJSON_Delete(data);

}
