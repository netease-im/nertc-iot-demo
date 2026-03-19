#ifndef __IOT_DYNREG_H
#define __IOT_DYNREG_H

#include <stdint.h>
#include "aws/common/allocator.h"
#include "protocols/http.h"
#include "iot_basic.h"

typedef struct {
    int32_t CodeN;
    char *Code;
    char *Message;
} iot_http_response_meta_data_error_t;


typedef struct {
    char *action;
    char *version;
    iot_http_response_meta_data_error_t responseMetaDataError;
} iot_http_response_meta_data_t;


typedef struct {
    int32_t len;
    char *payload;
} iot_http_response_dynamic_register_result_t;


typedef struct {
    iot_http_response_dynamic_register_result_t result;
    iot_http_response_meta_data_t meta_info;
} iot_http_response_dynamic_register_t;

typedef struct iot_dynamic_register_basic_param {
    const char *instance_id;
    const char *product_key;
    const char *device_name;
    int32_t random_num;
    uint64_t timestamp;
    onesdk_auth_type_t auth_type;
} iot_dynamic_register_basic_param_t;

struct aws_string *iot_hmac_sha256_encrypt(struct aws_allocator *allocator,
        const struct iot_dynamic_register_basic_param *registerBasicParam, const char *secret);

static iot_http_response_dynamic_register_t *parse_dynamic_register(struct aws_allocator *allocator, const http_response_t *response);

int dynamic_register(iot_basic_ctx_t *ctx);

int32_t random_num();

#endif // __IOT_DYNREG_H