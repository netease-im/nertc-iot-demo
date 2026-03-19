#include "duer_common.h"

#if INTELLIGENT_DUER

TokenData *duer_parse_token_json(const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        MY_LOG_E("JSON parse error:\n");
        return NULL;
    }
    TokenData *data = (TokenData *)malloc(sizeof(TokenData));
    if (!data) {
        MY_LOG_E("Memory allocation failed");
        cJSON_Delete(root);
        return NULL;
    }
    memset(data, 0, sizeof(TokenData));
    cJSON *item;
    if ((item = cJSON_GetObjectItem(root, "refresh_token")) && cJSON_IsString(item)) {
        data->refresh_token = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "expires_in")) && cJSON_IsNumber(item)) {
        data->expires_in = item->valueint;
    }
    if ((item = cJSON_GetObjectItem(root, "session_key")) && cJSON_IsString(item)) {
        data->session_key = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "access_token")) && cJSON_IsString(item)) {
        data->access_token = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "scope")) && cJSON_IsString(item)) {
        data->scope = strdup(item->valuestring);
    }
    if ((item = cJSON_GetObjectItem(root, "session_secret")) && cJSON_IsString(item)) {
        data->session_secret = strdup(item->valuestring);
    }
    cJSON_Delete(root);
    return data;
}


void duer_free_token_data(TokenData *data)
{
    if (data) {
        free(data->refresh_token);
        free(data->session_key);
        free(data->access_token);
        free(data->scope);
        free(data->session_secret);
        free(data);
    }
}


InsideRCResponse *duer_parse_inside_rc_json(const char *json_string)
{
    InsideRCResponse *response = calloc(1, sizeof(InsideRCResponse));
    if (!response) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_string);
    if (!root) {
        MY_LOG_E("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(response);
        return NULL;
    }

    cJSON *type = cJSON_GetObjectItem(root, "type");
    if (cJSON_IsString(type)) {
        response->type = strdup(type->valuestring);
    }

    if (!response->type || strcmp(response->type, "inside_rc") != 0) {
        cJSON_Delete(root);
        return response;
    }

    cJSON *status = cJSON_GetObjectItem(root, "status");
    if (cJSON_IsString(status)) {
        response->status = strdup(status->valuestring);
    }

    cJSON *sn = cJSON_GetObjectItem(root, "sn");
    if (cJSON_IsString(sn)) {
        response->sn = strdup(sn->valuestring);
    }

    cJSON *end = cJSON_GetObjectItem(root, "end");
    if (cJSON_IsNumber(end)) {
        response->end = end->valueint;
    }

    cJSON *data_obj = cJSON_GetObjectItem(root, "data");
    if (data_obj && cJSON_IsObject(data_obj)) {
        response->data = calloc(1, sizeof(Data));
        if (!response->data) {
            cJSON_Delete(root);
            duer_free_inside_rc_response(response);
            return NULL;
        }

        cJSON *code = cJSON_GetObjectItem(data_obj, "code");
        if (cJSON_IsNumber(code)) {
            response->data->code = code->valueint;
        }

        cJSON *msg = cJSON_GetObjectItem(data_obj, "msg");
        if (cJSON_IsString(msg)) {
            response->data->msg = strdup(msg->valuestring);
        }

        cJSON *logid = cJSON_GetObjectItem(data_obj, "logid");
        if (cJSON_IsString(logid)) {
            response->data->logid = strdup(logid->valuestring);
        }

        cJSON *qid = cJSON_GetObjectItem(data_obj, "qid");
        if (cJSON_IsString(qid)) {
            response->data->qid = strdup(qid->valuestring);
        }

        cJSON *is_end = cJSON_GetObjectItem(data_obj, "is_end");
        if (cJSON_IsNumber(is_end)) {
            response->data->is_end = is_end->valueint;
        }

        cJSON *need_clear_history = cJSON_GetObjectItem(data_obj, "need_clear_history");
        if (cJSON_IsNumber(need_clear_history)) {
            response->data->need_clear_history = need_clear_history->valueint;
        }

        cJSON *assistant_answer = cJSON_GetObjectItem(data_obj, "assistant_answer");
        if (assistant_answer) {
            response->data->assistant_answer = calloc(1, sizeof(AssistantAnswer));
            if (response->data->assistant_answer) {
                if (cJSON_IsString(assistant_answer)) {
                    cJSON *aa_root = cJSON_Parse(assistant_answer->valuestring);
                    if (aa_root) {
                        cJSON *content = cJSON_GetObjectItem(aa_root, "content");
                        if (cJSON_IsString(content)) {
                            response->data->assistant_answer->content = strdup(content->valuestring);
                        }

                        cJSON *nlu = cJSON_GetObjectItem(aa_root, "nlu");
                        if (cJSON_IsString(nlu)) {
                            response->data->assistant_answer->nlu = strdup(nlu->valuestring);
                        }

                        cJSON *aa_is_end = cJSON_GetObjectItem(aa_root, "is_end");
                        if (cJSON_IsNumber(aa_is_end)) {
                            response->data->assistant_answer->is_end = aa_is_end->valueint;
                        }

                        cJSON *metadata = cJSON_GetObjectItem(aa_root, "metadata");
                        if (metadata && cJSON_IsObject(metadata)) {
                            response->data->assistant_answer->metadata = calloc(1, sizeof(Metadata));
                            if (response->data->assistant_answer->metadata) {
                                cJSON *multi_round_info = cJSON_GetObjectItem(metadata, "multi_round_info");
                                if (multi_round_info && cJSON_IsObject(multi_round_info)) {
                                    response->data->assistant_answer->metadata->multi_round_info = calloc(1, sizeof(MultiRoundInfo));
                                    if (response->data->assistant_answer->metadata->multi_round_info) {
                                        cJSON *is_in_multi = cJSON_GetObjectItem(multi_round_info, "is_in_multi");
                                        if (cJSON_IsBool(is_in_multi)) {
                                            response->data->assistant_answer->metadata->multi_round_info->is_in_multi = is_in_multi->valueint;
                                        }

                                        cJSON *target_bot_id = cJSON_GetObjectItem(multi_round_info, "target_bot_id");
                                        if (cJSON_IsString(target_bot_id)) {
                                            response->data->assistant_answer->metadata->multi_round_info->target_bot_id = strdup(target_bot_id->valuestring);
                                        }

                                        cJSON *intent = cJSON_GetObjectItem(multi_round_info, "intent");
                                        if (cJSON_IsString(intent)) {
                                            response->data->assistant_answer->metadata->multi_round_info->intent = strdup(intent->valuestring);
                                        }
                                    }
                                }
                            }
                        }

                        cJSON_Delete(aa_root);
                    } else {
                        response->data->assistant_answer->content = strdup(assistant_answer->valuestring);
                    }
                } else if (cJSON_IsObject(assistant_answer)) {
                    cJSON *content = cJSON_GetObjectItem(assistant_answer, "content");
                    if (cJSON_IsString(content)) {
                        response->data->assistant_answer->content = strdup(content->valuestring);
                    }

                    cJSON *nlu = cJSON_GetObjectItem(assistant_answer, "nlu");
                    if (cJSON_IsString(nlu)) {
                        response->data->assistant_answer->nlu = strdup(nlu->valuestring);
                    }

                    cJSON *aa_is_end = cJSON_GetObjectItem(assistant_answer, "is_end");
                    if (cJSON_IsNumber(aa_is_end)) {
                        response->data->assistant_answer->is_end = aa_is_end->valueint;
                    }

                    cJSON *metadata = cJSON_GetObjectItem(assistant_answer, "metadata");
                    if (metadata && cJSON_IsObject(metadata)) {
                        response->data->assistant_answer->metadata = calloc(1, sizeof(Metadata));
                        if (response->data->assistant_answer->metadata) {
                            cJSON *multi_round_info = cJSON_GetObjectItem(metadata, "multi_round_info");
                            if (multi_round_info && cJSON_IsObject(multi_round_info)) {
                                response->data->assistant_answer->metadata->multi_round_info = calloc(1, sizeof(MultiRoundInfo));
                                if (response->data->assistant_answer->metadata->multi_round_info) {
                                    cJSON *is_in_multi = cJSON_GetObjectItem(multi_round_info, "is_in_multi");
                                    if (cJSON_IsBool(is_in_multi)) {
                                        response->data->assistant_answer->metadata->multi_round_info->is_in_multi = is_in_multi->valueint;
                                    }

                                    cJSON *target_bot_id = cJSON_GetObjectItem(multi_round_info, "target_bot_id");
                                    if (cJSON_IsString(target_bot_id)) {
                                        response->data->assistant_answer->metadata->multi_round_info->target_bot_id = strdup(target_bot_id->valuestring);
                                    }

                                    cJSON *intent = cJSON_GetObjectItem(multi_round_info, "intent");
                                    if (cJSON_IsString(intent)) {
                                        response->data->assistant_answer->metadata->multi_round_info->intent = strdup(intent->valuestring);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        cJSON *data_array = cJSON_GetObjectItem(data_obj, "data");
        if (data_array && cJSON_IsArray(data_array)) {
            int array_size = cJSON_GetArraySize(data_array);
            response->data->data_items_count = array_size;

            if (array_size > 0) {
                response->data->data_items = calloc(array_size, sizeof(DataItem *));
                if (!response->data->data_items) {
                    cJSON_Delete(root);
                    duer_free_inside_rc_response(response);
                    return NULL;
                }

                for (int i = 0; i < array_size; i++) {
                    DataItem *item = calloc(1, sizeof(DataItem));
                    if (!item) {
                        continue;
                    }
                    response->data->data_items[i] = item;

                    cJSON *array_item = cJSON_GetArrayItem(data_array, i);

                    cJSON *header = cJSON_GetObjectItem(array_item, "header");
                    if (header && cJSON_IsObject(header)) {
                        item->header = calloc(1, sizeof(Header));
                        if (item->header) {
                            cJSON *namespace = cJSON_GetObjectItem(header, "namespace");
                            if (cJSON_IsString(namespace)) {
                                item->header->namespace = strdup(namespace->valuestring);
                            }

                            cJSON *name = cJSON_GetObjectItem(header, "name");
                            if (cJSON_IsString(name)) {
                                item->header->name = strdup(name->valuestring);
                            }

                            cJSON *messageId = cJSON_GetObjectItem(header, "messageId");
                            if (cJSON_IsString(messageId)) {
                                item->header->messageId = strdup(messageId->valuestring);
                            }

                            cJSON *dialogRequestId = cJSON_GetObjectItem(header, "dialogRequestId");
                            if (cJSON_IsString(dialogRequestId)) {
                                item->header->dialogRequestId = strdup(dialogRequestId->valuestring);
                            }
                        }
                    }

                    cJSON *payload = cJSON_GetObjectItem(array_item, "payload");
                    if (payload && cJSON_IsObject(payload)) {
                        item->payload = calloc(1, sizeof(Payload));
                        if (item->payload) {
                            cJSON *rolling = cJSON_GetObjectItem(payload, "rolling");
                            if (cJSON_IsBool(rolling)) {
                                item->payload->rolling = rolling->valueint;
                            }

                            cJSON *text = cJSON_GetObjectItem(payload, "text");
                            if (cJSON_IsString(text)) {
                                item->payload->text = strdup(text->valuestring);
                            }

                            cJSON *content = cJSON_GetObjectItem(payload, "content");
                            if (cJSON_IsString(content)) {
                                item->payload->content = strdup(content->valuestring);
                            }

                            cJSON *format = cJSON_GetObjectItem(payload, "format");
                            if (cJSON_IsString(format)) {
                                item->payload->format = strdup(format->valuestring);
                            }

                            cJSON *token = cJSON_GetObjectItem(payload, "token");
                            if (cJSON_IsString(token)) {
                                item->payload->token = strdup(token->valuestring);
                            }

                            cJSON *url = cJSON_GetObjectItem(payload, "url");
                            if (cJSON_IsString(url)) {
                                item->payload->url = strdup(url->valuestring);
                                my_url_set(item->payload->url);
                                my_print_urls();
                            }

                            cJSON *type = cJSON_GetObjectItem(payload, "type");
                            if (cJSON_IsString(type)) {
                                item->payload->type = strdup(type->valuestring);
                            }

                            cJSON *answer = cJSON_GetObjectItem(payload, "answer");
                            if (cJSON_IsString(answer)) {
                                item->payload->answer = strdup(answer->valuestring);
                            }

                            cJSON *id = cJSON_GetObjectItem(payload, "id");
                            if (cJSON_IsString(id)) {
                                item->payload->id = strdup(id->valuestring);
                            }

                            cJSON *index = cJSON_GetObjectItem(payload, "index");
                            if (cJSON_IsNumber(index)) {
                                item->payload->index = index->valueint;
                            }

                            cJSON *payload_is_end = cJSON_GetObjectItem(payload, "is_end");
                            if (cJSON_IsNumber(payload_is_end)) {
                                item->payload->payload_is_end = payload_is_end->valueint;
                            }

                            cJSON *part = cJSON_GetObjectItem(payload, "part");
                            if (cJSON_IsString(part)) {
                                item->payload->part = strdup(part->valuestring);
                            }

                            cJSON *reasoning_part = cJSON_GetObjectItem(payload, "reasoning_part");
                            if (cJSON_IsString(reasoning_part)) {
                                item->payload->reasoning_part = strdup(reasoning_part->valuestring);
                            }

                            cJSON *tts = cJSON_GetObjectItem(payload, "tts");
                            if (cJSON_IsString(tts)) {
                                item->payload->tts = strdup(tts->valuestring);
                            }

                            cJSON *timeoutInMilliseconds = cJSON_GetObjectItem(payload, "timeoutInMilliseconds");
                            if (cJSON_IsNumber(timeoutInMilliseconds)) {
                                item->payload->timeoutInMilliseconds = timeoutInMilliseconds->valueint;
                            }
                        }
                    }

                    cJSON *property = cJSON_GetObjectItem(array_item, "property");
                    if (property && cJSON_IsObject(property)) {
                        cJSON *serviceCategory = cJSON_GetObjectItem(property, "serviceCategory");
                        if (cJSON_IsString(serviceCategory)) {
                            item->serviceCategory = strdup(serviceCategory->valuestring);
                        }
                    }
                }
            }
        }

        cJSON *lj_thread_id = cJSON_GetObjectItem(data_obj, "lj_thread_id");
        if (cJSON_IsString(lj_thread_id)) {
            response->data->lj_thread_id = strdup(lj_thread_id->valuestring);
        }

        cJSON *ab_conversation_id = cJSON_GetObjectItem(data_obj, "ab_conversation_id");
        if (cJSON_IsString(ab_conversation_id)) {
            response->data->ab_conversation_id = strdup(ab_conversation_id->valuestring);
        }

        cJSON *xiaoice_session_id = cJSON_GetObjectItem(data_obj, "xiaoice_session_id");
        if (cJSON_IsString(xiaoice_session_id)) {
            response->data->xiaoice_session_id = strdup(xiaoice_session_id->valuestring);
        }

        cJSON *dialog_request_id = cJSON_GetObjectItem(data_obj, "dialog_request_id");
        if (cJSON_IsString(dialog_request_id)) {
            response->data->dialog_request_id = strdup(dialog_request_id->valuestring);
        }
    }
    cJSON_Delete(root);
    if (response &&
        response->data &&
        response->data->is_end == 1) {
        extern void my_list_download_task();
        my_list_download_task();
    }
    return response;
}

void duer_free_inside_rc_response(InsideRCResponse *response)
{
    if (!response) {
        return;
    }

    free(response->status);
    free(response->type);
    free(response->sn);

    if (response->data) {
        free(response->data->msg);
        free(response->data->logid);
        free(response->data->qid);
        free(response->data->lj_thread_id);
        free(response->data->ab_conversation_id);
        free(response->data->xiaoice_session_id);
        free(response->data->dialog_request_id);

        if (response->data->assistant_answer) {
            free(response->data->assistant_answer->content);
            free(response->data->assistant_answer->nlu);

            if (response->data->assistant_answer->metadata) {
                if (response->data->assistant_answer->metadata->multi_round_info) {
                    free(response->data->assistant_answer->metadata->multi_round_info->target_bot_id);
                    free(response->data->assistant_answer->metadata->multi_round_info->intent);
                    free(response->data->assistant_answer->metadata->multi_round_info);
                }
                free(response->data->assistant_answer->metadata);
            }
            free(response->data->assistant_answer);
        }

        if (response->data->data_items) {
            for (int i = 0; i < response->data->data_items_count; i++) {
                DataItem *item = response->data->data_items[i];
                if (!item) {
                    continue;
                }

                if (item->header) {
                    free(item->header->namespace);
                    free(item->header->name);
                    free(item->header->messageId);
                    free(item->header->dialogRequestId);
                    free(item->header);
                }

                if (item->payload) {
                    free(item->payload->text);
                    free(item->payload->content);
                    free(item->payload->format);
                    free(item->payload->token);
                    free(item->payload->url);
                    free(item->payload->type);
                    free(item->payload->answer);
                    free(item->payload->id);
                    free(item->payload->part);
                    free(item->payload->reasoning_part);
                    free(item->payload->tts);
                    free(item->payload);
                }

                free(item->serviceCategory);
                free(item);
            }
            free(response->data->data_items);
        }

        free(response->data);
    }

    free(response);
}


#if 0
int cjson_demo()
{
    const char *json = "{\"status\":\"ok\",\"type\":\"inside_rc\",\"data\":{\"code\":0,\"msg\":\"ok\",\"logid\":\"1753665417\",\"qid\":\"1184118576_a2f3e6cc\",\"is_end\":1,\"assistant_answer\":\"{\\\"content\\\":\\\"\\\\n对不起,没有听到你说了什么\\\\n\\\",\\\"nlu\\\":\\\"event.dcs\\\",\\\"metadata\\\":{\\\"multi_round_info\\\":{\\\"is_in_multi\\\":false,\\\"target_bot_id\\\":\\\"\\\",\\\"intent\\\":\\\"\\\"}}}\",\"need_clear_history\":0,\"data\":[{\"header\":{\"namespace\":\"ai.dueros.device_interface.voice_output\",\"name\":\"Speak\",\"messageId\":\"Njg4NmNmODk2NWU4NjUzNjI=\",\"dialogRequestId\":\"1184118576_a2f3e6cc\"},\"payload\":{\"content\":\"对不起,没有听到你说了什么\",\"format\":\"AUDIO_MPEG\",\"token\":\"eyJib3RfaWQiOiJ1cyIsInJlc3VsdF90b2tlbiI6IjhkMWEwMDcxLTE4YjItNDMwYi05NDMxLWZkMTEwM2I1MGI1NyIsImJvdF90b2tlbiI6Im51bGwiLCJsYXVuY2hfaWRzIjpbIiJdfQ==\",\"url\":\"http://duer-kids.baidu.com/sandbox/tts-proxy/precache/tts?sn=9TI8gUtVespTyjLT3xEDnpjQ85mlnKWum0Od8%2FtK8FfJvQxcxtFoK1EMGQfPC2Yh&0026token=1753665417_rg8gfoGIKkCR\"},\"property\":{\"serviceCategory\":\"\"}}],\"lj_thread_id\":\"\",\"ab_conversation_id\":\"\",\"xiaoice_session_id\":\"\",\"dialog_request_id\":\"1184118576_a2f3e6cc\"},\"sn\":\"UFMira\",\"end\":0}";

    const char *jsona = "{\"status\":\"ok\",\"type\":\"inside_rc\",\"data\":{\"code\":0,\"msg\":\"ok\",\"logid\":\"1753666085\",\"qid\":\"1990458576_a952a448\",\"is_end\":0,\"assistant_answer\":\"小朋友，你是没听懂我刚才说的什么吗？没关系哦，\",\"need_clear_history\":0,\"data\":[{\"header\":{\"namespace\":\"ai.dueros.device_interface.screen\",\"name\":\"RenderStreamCard\",\"messageId\":\"\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"answer\":\"\",\"id\":\"b327299b-5924-4441-b740-dec422bedfb4\",\"index\":13,\"is_end\":0,\"part\":\"，\",\"reasoning_part\":\"\",\"tts\":\"小朋友，你是没听懂我刚才说的什么吗？没关系哦，\"},\"property\":{\"serviceCategory\":\"\"}},{\"header\":{\"namespace\":\"ai.dueros.device_interface.voice_output\",\"name\":\"Speak\",\"messageId\":\"7be6d381-dd3d-49b9-880e-efc080297311\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"format\":\"AUDIO_MPEG\",\"token\":\"iwLyLvVe3pKWsT9x\",\"url\":\"http://duer-kids.baidu.com/sandbox/tts-proxy/precache/tts?sn=HUYU%2BvFGxXcbCNyJNnEb6Lsq4uRNSZKUkBrnt%2BSVE40dHk3t6T0RJl%2FLi%2FMSA0Jm&0026token=1753666086_szo1p09GCQmC\"},\"property\":{\"serviceCategory\":\"\"}}],\"lj_thread_id\":\"\",\"ab_conversation_id\":\"\",\"xiaoice_session_id\":\"\",\"dialog_request_id\":\"1990458576_a952a448\"},\"sn\":\"g3BbCO\",\"end\":0}";

    const char *jsonb = "{\"status\":\"ok\",\"type\":\"inside_rc\",\"data\":{\"code\":0,\"msg\":\"ok\",\"logid\":\"1753666085\",\"qid\":\"1990458576_a952a448\",\"is_end\":0,\"assistant_answer\":\"小朋友，你是没听懂我刚才说的什么吗？没关系哦，你可以再跟我说一遍你想聊的话题，或者问我刚刚说的话是什么意思。\",\"need_clear_history\":0,\"data\":[{\"header\":{\"namespace\":\"ai.dueros.device_interface.screen\",\"name\":\"RenderStreamCard\",\"messageId\":\"\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"answer\":\"\",\"id\":\"b327299b-5924-4441-b740-dec422bedfb4\",\"index\":27,\"is_end\":0,\"part\":\"。\",\"reasoning_part\":\"\",\"tts\":\"你可以再跟我说一遍你想聊的话题，或者问我刚刚说的话是什么意思。\"},\"property\":{\"serviceCategory\":\"\"}},{\"header\":{\"namespace\":\"ai.dueros.device_interface.voice_output\",\"name\":\"Speak\",\"messageId\":\"b2562170-a4e5-474f-88bd-30e55cb50b22\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"format\":\"AUDIO_MPEG\",\"token\":\"18Qnpj1EizrPpcdm\",\"url\":\"http://duer-kids.baidu.com/sandbox/tts-proxy/precache/tts?sn=%2FhYTk3AMbnKc0%2FritQ5tSKPo5QX6pIyDUU8jKNSfuXopuB8Z44bQg%2BWSf77RZgIc&0026token=1753666087_W0JSVb0LnOEX\"},\"property\":{\"serviceCategory\":\"\"}}],\"lj_thread_id\":\"\",\"ab_conversation_id\":\"\",\"xiaoice_session_id\":\"\",\"dialog_request_id\":\"1990458576_a952a448\"},\"sn\":\"g3BbCO\",\"end\":0}";

    const char *jsonc = "{\"status\":\"ok\",\"type\":\"inside_rc\",\"data\":{\"code\":0,\"msg\":\"ok\",\"logid\":\"1753666085\",\"qid\":\"1990458576_a952a448\",\"is_end\":1,\"assistant_answer\":\"{\\\"content\\\":\\\"小朋友，你是没听懂我刚才说的什么吗？没关系哦，你可以再跟我说一遍你想聊的话题，或者问我刚刚说的话是什么意思。小度最喜欢帮小朋友解答问题啦！\\\",\\\"nlu\\\":\\\"Chat.Translation\\\",\\\"metadata\\\":{\\\"multi_round_info\\\":{\\\"is_in_multi\\\":false,\\\"target_bot_id\\\":\\\"\\\",\\\"intent\\\":\\\"\\\"}}}\",\"need_clear_history\":0,\"data\":[{\"header\":{\"namespace\":\"ai.dueros.device_interface.screen\",\"name\":\"RenderStreamCard\",\"messageId\":\"\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"answer\":\"\",\"id\":\"b327299b-5924-4441-b740-dec422bedfb4\",\"index\":37,\"is_end\":1,\"part\":\"\",\"reasoning_part\":\"\",\"tts\":\"小度最喜欢帮小朋友解答问题啦！\"},\"property\":{\"serviceCategory\":\"\"}},{\"header\":{\"namespace\":\"ai.dueros.device_interface.voice_output\",\"name\":\"Speak\",\"messageId\":\"4765f38e-311a-4d8e-916b-59712d1de523\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"format\":\"AUDIO_MPEG\",\"token\":\"b8KfiCB946Y0xUkF\",\"url\":\"http://duer-kids.baidu.com/sandbox/tts-proxy/precache/tts?sn=%2FhYTk3AMbnKc0%2FritQ5tSAha6q%2FlpY0%2FpYGDqc9kPjuGB3%2BsSRrN6tORd%2FrKSHUg&0026token=1753666087_2H8FD5p2iXsw\"},\"property\":{\"serviceCategory\":\"\"}},{\"header\":{\"namespace\":\"ai.dueros.device_interface.voice_input\",\"name\":\"Listen\",\"messageId\":\"0979acb1-76fe-4dfc-a76d-e06cd0380994\",\"dialogRequestId\":\"1990458576_a952a448\"},\"payload\":{\"timeoutInMilliseconds\":60000},\"property\":{\"serviceCategory\":\"\"}}],\"lj_thread_id\":\"\",\"ab_conversation_id\":\"\",\"xiaoice_session_id\":\"\",\"dialog_request_id\":\"1990458576_a952a448\"},\"sn\":\"g3BbCO\",\"end\":0}";


    InsideRCResponse *response = duer_parse_inside_rc_json(json);
    if (response) {
        duer_free_inside_rc_response(response);
    }

    InsideRCResponse *responsea = duer_parse_inside_rc_json(jsona);
    if (responsea) {
        duer_free_inside_rc_response(responsea);
    }

    InsideRCResponse *responseb = duer_parse_inside_rc_json(jsonb);
    if (responseb) {
        duer_free_inside_rc_response(responseb);
    }

    InsideRCResponse *responsec = duer_parse_inside_rc_json(jsonc);
    if (responsec) {
        duer_free_inside_rc_response(responsec);
    }

    return 0;
}
#endif

#endif
