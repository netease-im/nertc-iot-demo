#ifndef ONESDK_IOT_IOT_UTILS_H
#define ONESDK_IOT_IOT_UTILS_H

#define SDK_VERSION "1.0.0"
#define TAG_IOT_MQTT "IOT_MQTT"

#include "aws/common/string.h"
#include "fs/fs.h"
char *iot_get_common_topic(struct aws_allocator *allocator, char *fmt, struct aws_string *product_key, struct aws_string *device_name);

char *get_random_string_with_time_suffix(struct aws_allocator *allocator);

struct aws_string *md5File(FILE *file);

uint64_t get_current_time_mil();

int32_t secure_strlen(const char *str);

char *iot_get_topic_with_1_c_str_param(struct aws_allocator *allocator, char *fmt, struct aws_string *product_key,
                                       struct aws_string *device_name, char *p1);

char *iot_get_topic_with_1_param(struct aws_allocator *allocator, char *fmt, struct aws_string *product_key,
                                 struct aws_string *device_name, struct aws_string *p1);

char *iot_get_topic_with_2_c_str_param(struct aws_allocator *allocator, char *fmt, struct aws_string *product_key,
                                       struct aws_string *device_name, char *p1, char *p2);

char *iot_get_topic_with_3_c_str_param(struct aws_allocator *allocator, const char *fmt, const char *product_key,
                                       const char *device_name, const char *p1, const char *p2, const char *p3);

const char *get_random_string_id_c_str(struct aws_allocator *allocator);

char *aws_buf_to_char_str(
    struct aws_allocator *allocator,
    const struct aws_byte_buf *cur);

#endif
