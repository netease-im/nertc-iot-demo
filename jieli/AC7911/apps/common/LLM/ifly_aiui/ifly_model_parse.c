#include <string.h>
#include "asm/cpu.h"
#include "cJSON.h"
#include "ifly_aiui_net.h"
#include "ifly_aiui_sdk_common.h"
//#include "ifly_aiui_api.h"
//#include "aiui_audio_control.h"
//#include "ifly_aiui_play_pcm.h"
//#include "aiui_split_synthesize.h"
//#include "aiui_download_url.h"
//#include "aiui_log.h"
//#include "main_task.h"
#include "server/audio_server.h"
//#include "ifly_aiui.h"

#define AIUI_TAG "SPARK_PARSE"


typedef int (*pfun)(const cJSON *payload_json, const cJSON *header_json);
typedef struct aiui_function_map_s {
    char *type;
    pfun func;
} aiui_function_map_t;

extern char cur_tts_stmid[32];
extern char cur_audio_stmid[32];
char cur_cloud_stmid[32];

static bool s_drop_nlp = false;

//static AIUI_SPLIT_HANDLE s_split_handle = NULL;

char *ifly_model_base64_encode(char *input, int *output_len)
{
    char *des_buff = NULL;
    *output_len = 0;
    if (NULL == input) {
        printf("ifly_model_base64_encode, input error");
        return NULL;
    }

    mbedtls_base64_encode(NULL, 0, output_len, input, strlen(input));
    des_buff = malloc((*output_len) + 1);
    if (des_buff == NULL) {
        printf("ifly_model_base64_encode, malloc error");
        return NULL;
    }
    memset(des_buff, 0, (*output_len) + 1);
    if (mbedtls_base64_encode(des_buff, *output_len, output_len, input, strlen(input)) != 0) {
        printf("ifly_model_base64_encode, mbedtls_base64_encode error");
        free(des_buff);
        return NULL;
    }
    return des_buff;

}

char *ifly_model_base64_decode(char *input, int *output_len)
{
    char *des_buff = NULL;
    *output_len = 0;
    if (NULL == input) {
        printf("ifly_model_base64_decode, input error");
        return NULL;
    }

    mbedtls_base64_decode(NULL, 0, output_len, input, strlen(input));
    des_buff = malloc((*output_len) + 1);
    if (des_buff == NULL) {
        printf("ifly_model_base64_decode, malloc error");
        return NULL;
    }
    memset(des_buff, 0, (*output_len) + 1);
    if (mbedtls_base64_decode(des_buff, *output_len, output_len, input, strlen(input)) != 0) {
        printf("ifly_model_base64_decode, mbedtls_base64_decode error");
        free(des_buff);
        return NULL;
    }
    return des_buff;

}
char *find_last_chinese_end(char *str)
{
    const char *symbol[] = {"\xE3\x80\x81", "\xE3\x80\x82", "\xEF\xBC\x9B", "\xEF\xBC\x9A", "\xEF\xBC\x9F", "\xEF\xBC\x81", "\xEF\xBC\x8C"};
    char *last_addr = NULL;
    for (int i = 0; i < sizeof(symbol) / sizeof(symbol[0]); i++) {
        char *str_addr = strstr(str, symbol[i]);
        while (str_addr) {
            last_addr = str_addr;
            str_addr = strstr(str_addr + strlen(symbol[i]), symbol[i]);
        }
    }
    return last_addr;
}

static char *parse_iat_text(const char *json_str)
{
    cJSON *pl_json = cJSON_Parse(json_str);
    if (pl_json == NULL) {
        printf("Failed to parse json_str");
        return NULL;
    }

    cJSON *text = cJSON_GetObjectItem(pl_json, "text");
    if (text == NULL) {
        printf("Failed to get text");
        cJSON_Delete(pl_json);
        return NULL;
    }

    cJSON *ws = cJSON_GetObjectItem(text, "ws");
    if (ws == NULL) {
        printf("Failed to get ws");
        cJSON_Delete(pl_json);
        return NULL;
    }

    int ws_size = cJSON_GetArraySize(ws);
    if (ws_size <= 0) {
        printf("ws array is empty");
        cJSON_Delete(pl_json);
        return NULL;
    }

    // 计算总长度
    size_t total_length = 0;
    for (int i = 0; i < ws_size; i++) {
        cJSON *ws_item = cJSON_GetArrayItem(ws, i);
        if (ws_item == NULL) {
            printf("Failed to get ws_item at index %d", i);
            continue;
        }

        cJSON *cw = cJSON_GetObjectItem(ws_item, "cw");
        if (cw == NULL) {
            printf("Failed to get cw at index %d", i);
            continue;
        }

        cJSON *cw_item = cJSON_GetArrayItem(cw, 0);
        if (cw_item == NULL) {
            printf("Failed to get cw_item at index %d", i);
            continue;
        }

        cJSON *w = cJSON_GetObjectItem(cw_item, "w");
        if (w == NULL) {
            printf("Failed to get w at index %d", i);
            continue;
        }

        total_length += strlen(w->valuestring);
    }

    // 分配内存
    char *result = malloc(total_length + 1); // +1 用于存储字符串结束符
    if (result == NULL) {
        printf("Memory allocation failed");
        cJSON_Delete(pl_json);
        return NULL;
    }
    result[0] = '\0'; // 初始化为空字符串

    // 拼接文本
    for (int i = 0; i < ws_size; i++) {
        cJSON *ws_item = cJSON_GetArrayItem(ws, i);
        if (ws_item == NULL) {
            printf("Failed to get ws_item at index %d", i);
            continue;
        }

        cJSON *cw = cJSON_GetObjectItem(ws_item, "cw");
        if (cw == NULL) {
            printf("Failed to get cw at index %d", i);
            continue;
        }

        cJSON *cw_item = cJSON_GetArrayItem(cw, 0);
        if (cw_item == NULL) {
            printf("Failed to get cw_item at index %d", i);
            continue;
        }

        cJSON *w = cJSON_GetObjectItem(cw_item, "w");
        if (w == NULL) {
            printf("Failed to get w at index %d", i);
            continue;
        }

        strcat(result, w->valuestring); // 拼接文本
    }

    cJSON_Delete(pl_json);
    return result;
}

static void trim_punctuation(char *str)
{
    const char *punctuation[] = {".", ",", "!", "?", "，", "。", "！", "？"};
    size_t len = strlen(str);
    while (len > 0) {
        bool is_punctuation = false;
        for (int i = 0; i < sizeof(punctuation) / sizeof(punctuation[0]); i++) {
            size_t p_len = strlen(punctuation[i]);
            if (len >= p_len && memcmp(str + len - p_len, punctuation[i], p_len) == 0) {
                len -= p_len;
                str[len] = '\0';
                is_punctuation = true;
                break;
            }
        }
        if (!is_punctuation) {
            break;
        }
    }
}

static int _aiui_bm_sub_iat(const cJSON *data, const cJSON *header_data)
{
    int len, vl_olen;

    printf("sub is iat");
    if (cJSON_IsInvalid(data)) {
        return -1;
    }
    cJSON *text = cJSON_GetObjectItem(data, "text");
    if (NULL == text || text->valuestring == NULL) {
        return -1;
    }
    len = strlen(text->valuestring);
    if (len == 0) {
        return -1;
    }
    char *pl_buf = NULL;
    pl_buf = ifly_model_base64_decode(text->valuestring, &vl_olen);
    if (NULL == pl_buf) {
        return 0;
    }
    *(pl_buf + vl_olen) = 0;
    printf("iat is %s", pl_buf);

    char *extracted_text = parse_iat_text(pl_buf);
    if (extracted_text != NULL) {
        printf("Extracted text: %s", extracted_text);

        if (strncmp(extracted_text, "切到助手", 12) == 0) {
            /* char* assistant_name = extracted_text + 12;
             trim_punctuation(assistant_name);
             if (aiui_get_character_by_name(assistant_name) == 0) {
                 printf("Switched to assistant: %s", assistant_name);
                 app_music_local_play("ConfigAck.mp3");
             } else {
                 printf("Assistant not found: %s", assistant_name);
                 app_music_local_play("NofondVcn.mp3");
             }
             ifly_aiui_cancel_recognize_process();
             ifly_aiui_wakeup_notify();
             aiui_auido_control_stopEX();*/
        } else if (strncmp(extracted_text, "你退下吧", 12) == 0
                   || strncmp(extracted_text, "退下", 6) == 0
                   || strncmp(extracted_text, "闭嘴", 6) == 0
                   || strncmp(extracted_text, "再见", 6) == 0
                   || strncmp(extracted_text, "拜拜", 6) == 0
                   || strncmp(extracted_text, "结束", 6) == 0) {
            if (ifly_aiui_app_get_connect_status()) {
//            main_task_post_message(MSG_MAIN_TASK_AIUI_DISCONNECT, 0, 0, NULL, 0);
            }
//            ifly_aiui_cancel_recognize_process();
//            app_music_local_play("AiuiExit.mp3");
//            aiui_auido_control_stopEX();
        } else if (strlen(extracted_text) < 21
                   && (strncmp(extracted_text, "小飞小飞", 12) == 0
                       || strncmp(extracted_text, "小v小v", 8) == 0)) {
//            ifly_aiui_ivw_res_cb(0);
//            ifly_aiui_cancel_recognize_process();
////            IFLY_AIUI_OS_TASKQ_POST(IFLY_AIUI_APP_TASK_NAME, 1, IFLY_AIUI_WAKEUP_EVENT);
//            ifly_aiui_wakeup_notify();
        }

        free(extracted_text);
    }

    aiui_free(pl_buf);
    return 0;
}

static int _aiui_bm_sub_nlp(const cJSON *data, const cJSON *header_data)
{
    if (s_drop_nlp) {
        return 0;
    }
    size_t vl_olen;

    if (cJSON_IsInvalid(data)) {
        return -1;
    }
    cJSON *seq = cJSON_GetObjectItem(data, "seq");
    if (cJSON_IsInvalid(seq)) {
        return -1;
    }
    cJSON *status = cJSON_GetObjectItem(data, "status");
    if (cJSON_IsInvalid(status)) {
        return -1;
    }
    cJSON *text = cJSON_GetObjectItem(data, "text");
    if (cJSON_IsInvalid(text)) {
        return -1;
    }
    int len = strlen(text->valuestring);
    bool new = (status->valueint == 0 || status->valueint == 3) ? true : false;
    bool finish = (status->valueint == 2 || status->valueint == 3) ? true : false;
    if (len == 0) {
        return 0;
    }

    char *pl_buf = NULL;
    pl_buf = ifly_model_base64_decode(text->valuestring, &vl_olen);
    if (NULL == pl_buf) {
        return 0;
    }

    printf("nlp is %s, seq:%d, status:%d", pl_buf, seq->valueint, status->valueint);
    aiui_free(pl_buf);
    return 0;
}

static int _aiui_bm_sub_tts(const cJSON *data, const cJSON *header_data)
{
    if (cJSON_IsInvalid(data)) {
        return -1;
    }
    cJSON *tts_base64 = cJSON_GetObjectItem(data, "audio");
    cJSON *status = cJSON_GetObjectItem(data, "status");
    cJSON *seq = cJSON_GetObjectItem(data, "seq");
    if (cJSON_IsInvalid(status)) {
        return -1;
    }
    if (cJSON_IsInvalid(seq)) {
        return -1;
    }
    if (NULL == tts_base64 || cJSON_IsNull(tts_base64)) {
        return 0;
    }
    int len = 0;
    char *tts = ifly_model_base64_decode(tts_base64->valuestring, &len);
    if (NULL == tts) {
        return 0;
    }
    printf("sub is tts:%d, seq:%d, status:%d", len, seq->valueint, status->valueint);
    if (status->valueint == 0) {
        _device_rbuf_clear();
    }
    if (len > 0 || status->valueint == 2) {
        _device_write_voice_data(tts, len);
        if (len > 0 && status->valueint == 2) {
            _device_write_voice_data(tts, 0);
        }
    }
#if USER_DELAY_TTS
    if (ifly_aiui_play_pcm_get_len() > (AIUI_PLAY_PCM_BUFF_LEN / 8)) {
        os_time_dly((len / 320) + 1);
    }
#endif
    aiui_free(tts);
    return 0;
}

static int _aiui_bm_cbm_semantic(const cJSON *data, const cJSON *header_data)
{
#define CBM_SEMANTIC_TEXT_LEN_MAX       (10*1024)
    size_t vl_olen;

    printf("sub is cbm semantic");
    if (cJSON_IsInvalid(data)) {
        return -1;
    }
    //printf("semantic data is %s", cJSON_PrintUnformatted(data));
    cJSON *text = cJSON_GetObjectItem(data, "text");
    if (NULL == text || text->valuestring == NULL) {
        return -1;
    }

    char *pl_buf = NULL;
    pl_buf = ifly_model_base64_decode(text->valuestring, &vl_olen);
    if (NULL == pl_buf) {
        return 0;
    }

    printf("cbm_semantic_text=\r\n%s\r\n", pl_buf);

    cJSON *pl_CJson_cbm_semantic_text = cJSON_Parse(pl_buf);
    if (pl_CJson_cbm_semantic_text == NULL) {
        aiui_free(pl_buf);
        return -1;
    }
    s_drop_nlp = false;
//    if (aiui_skill_parse(pl_CJson_cbm_semantic_text, NULL) == 1)
//    {
//        cJSON *answer = cJSON_GetObjectItem(pl_CJson_cbm_semantic_text, "answer");
//        cJSON *result = cJSON_GetObjectItem(answer, "text");
//        s_drop_nlp = true;
//        if (result != NULL && strlen(result->valuestring) > 0)
//        {
//            printf("result->valuestring = %s\n", result->valuestring);
//            //aiui_append_answer(s_split_handle, true, result->valuestring, true);
//        }
//        else
//        {
////            aiui_audio_ctrl_cmd(AIUI_AUDIO_CTRL_TTS, NULL);
//        }
//    }
    cJSON_Delete(pl_CJson_cbm_semantic_text);
    aiui_free(pl_buf);
    return 0;
}

static int _aiui_bm_cloud_event(const cJSON *data, const cJSON *header_data)
{
    size_t vl_olen;

    printf("sub is cloud event");
    if (cJSON_IsInvalid(data)) {
        return -1;
    }
    //printf("sub is cloud event is %s", cJSON_PrintUnformatted(data));
    cJSON *text = cJSON_GetObjectItem(data, "text");
    cJSON *pl_stmid = cJSON_GetObjectItem(header_data, "stmid");
    if (NULL == text || text->valuestring == NULL) {
        return -1;
    }

    char *pl_buf = NULL;
    pl_buf = ifly_model_base64_decode(text->valuestring, &vl_olen);
    if (NULL == pl_buf) {
        return 0;
    }

    printf("cloud_event_text=\r\n%s\r\n", pl_buf);

    cJSON *pl_CJson_cloud_event_text = cJSON_Parse(pl_buf);
    if (pl_CJson_cloud_event_text == NULL) {
        aiui_free(pl_buf);
        return -1;
    }
    cJSON *key = cJSON_GetObjectItem(pl_CJson_cloud_event_text, "key");
    if (key != NULL) {
        printf("cloud event:%s", key->valuestring);
        if (0 == memcmp(key->valuestring, "Bos", sizeof("Bos"))) {
//            IFLY_AIUI_OS_TASKQ_POST(IFLY_AIUI_APP_TASK_NAME, 2, IFLY_AIUI_WAKEUP_EVENT, IFLY_AIUI_INTERRUPT_WAKEUP);
            memset(cur_cloud_stmid, 0, sizeof(cur_cloud_stmid));
            memcpy(cur_cloud_stmid, pl_stmid->valuestring, strlen(pl_stmid->valuestring));
        } else if (0 == memcmp(key->valuestring, "Eos", sizeof("Eos"))) {
            if (iflytek_is_vad_enabled()) {
//                ifly_aiui_vad_notify();
            }
        } else if (0 == memcmp(key->valuestring, "Silence", sizeof("Silence"))
                   && (strncmp(pl_stmid->valuestring, cur_audio_stmid, strlen(cur_audio_stmid)) == 0)) {
//            ifly_aiui_cancel_recognize_process();
        }
    }
    cJSON_Delete(pl_CJson_cloud_event_text);
    aiui_free(pl_buf);
    return 0;
}

static aiui_function_map_t gs_aiui_bm_pay_load_func[] = {
    {"iat", _aiui_bm_sub_iat},
    {"nlp", _aiui_bm_sub_nlp},
    {"tts", _aiui_bm_sub_tts},
    {"cbm_semantic", _aiui_bm_cbm_semantic},
    {"event", _aiui_bm_cloud_event},
};

int ifly_model_message_parse(cJSON *data)
{
    int data_len;
    int ret = 0;
    static int nlp_text_len = 0;
    static char nlp_text_buff[512];
    cJSON *pl_header = cJSON_GetObjectItem(data, "header");
    cJSON *pl_stmid = cJSON_GetObjectItem(pl_header, "stmid");
    cJSON *pl_payload = cJSON_GetObjectItem(data, "payload");
    if (pl_payload != NULL) {
        for (int i = 0; i < sizeof(gs_aiui_bm_pay_load_func) / sizeof(gs_aiui_bm_pay_load_func[0]); i++) {
            cJSON *pl_ele = cJSON_GetObjectItem(pl_payload, gs_aiui_bm_pay_load_func[i].type);

            if (pl_ele == NULL) {
                continue;
            }
            /* if (strcmp(gs_aiui_bm_pay_load_func[i].type, "tts") == 0)
             {
                 if (strcmp(pl_stmid->valuestring, cur_tts_stmid) != 0)
                 {
                     printf("%s drop, pl_stmid->valuestring:%s , cur_tts_stmid:%s", gs_aiui_bm_pay_load_func[i].type,pl_stmid->valuestring,cur_tts_stmid);
                     break;
                 }
             }
             else
             {
                 if (strcmp(pl_stmid->valuestring, cur_audio_stmid) != 0)
                 {
                     printf("%s drop, pl_stmid->valuestring:%s , cur_audio_stmid:%s", gs_aiui_bm_pay_load_func[i].type,pl_stmid->valuestring,cur_audio_stmid);
                     break;
                 }
             }*/

            if (strcmp(pl_stmid->valuestring, cur_cloud_stmid) != 0) {
                if (memcmp(gs_aiui_bm_pay_load_func[i].type, "event", sizeof("event") != 0)) {
                    printf("%s drop, pl_stmid->valuestring:%s , cur_cloud_stmid:%s", gs_aiui_bm_pay_load_func[i].type, pl_stmid->valuestring, cur_cloud_stmid);
                    break;
                }
            }
            gs_aiui_bm_pay_load_func[i].func(pl_ele, pl_header);
            break;
        }
    } else {
        cJSON *pl_code = cJSON_GetObjectItem(pl_header, "code");
        if (NULL == pl_code) {
            return -1;
        }
        if (pl_code->valueint != 0) {
//            aiui_audio_ctrl_cmd(AIUI_AUDIO_CTRL_TTS, NULL);
            return -1;
        }
    }

    return ret;
}

static void on_split_synthesize(const void *param, const char *text, bool completed)
{
}

//void aiui_message_parse_init()
//{
//    s_split_handle = aiui_split_synthesize_init(on_split_synthesize, NULL);
//    aiui_audio_ctrl_init();
//}

//void aiui_message_parse_uninit()
//{
//    aiui_audio_control_deinit();
//    aiui_split_synthesize_uninit(s_split_handle);
//}

