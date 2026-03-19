#ifndef _DUER_JSON_PARSE_H_
#define _DUER_JSON_PARSE_H_


#include "duer_common.h"
#include "my_platform_common.h"
#include <string.h>

typedef struct {
    char *refresh_token;
    int expires_in;
    char *session_key;
    char *access_token;
    char *scope;
    char *session_secret;
} TokenData;


// extern TokenData *duer_parse_token_json(const char *json_str);
extern void duer_free_token_data(TokenData *data);


typedef struct {
    char *namespace;
    char *name;
    char *messageId;
    char *dialogRequestId;
} Header;

typedef struct {
    char *text;
    char *content;
    char *format;
    char *token;
    char *url;
    int rolling;
    char *type;
    char *answer;
    char *id;
    int index;
    int payload_is_end;
    char *part;
    char *reasoning_part;
    char *tts;
    int timeoutInMilliseconds;
} Payload;

typedef struct {
    Header *header;
    Payload *payload;
    char *serviceCategory;
} DataItem;

typedef struct {
    int is_in_multi;
    char *target_bot_id;
    char *intent;
} MultiRoundInfo;

typedef struct {
    MultiRoundInfo *multi_round_info;
} Metadata;

typedef struct {
    char *content;
    char *nlu;
    Metadata *metadata;
    int is_end;
} AssistantAnswer;

typedef struct {
    int code;
    char *msg;
    char *logid;
    char *qid;
    int is_end;
    AssistantAnswer *assistant_answer;
    int need_clear_history;
    DataItem **data_items;
    int data_items_count;
    char *lj_thread_id;
    char *ab_conversation_id;
    char *xiaoice_session_id;
    char *dialog_request_id;
} Data;

typedef struct {
    char *status;
    char *type;
    Data *data;
    char *sn;
    int end;
} InsideRCResponse;


extern InsideRCResponse *duer_parse_inside_rc_json(const char *json_string);

extern void duer_free_inside_rc_response(InsideRCResponse *response);

#endif
