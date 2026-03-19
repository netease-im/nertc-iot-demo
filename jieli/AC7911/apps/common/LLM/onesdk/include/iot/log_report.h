#ifndef ONESDK_IOT_LOG_REPORT_H
#define ONESDK_IOT_LOG_REPORT_H

#include "iot_log.h"
#include "iot_mqtt.h"

// 日志上报
typedef struct log_handler {
    iot_mqtt_ctx_t *mqtt_handle;
    struct aws_allocator *allocator;
    bool log_report_switch;
    pthread_mutex_t lock;
    enum onesdk_log_level lowest_level; // 最新上报level
    // struct aws_hash_table* stream_id_config_map;
    bool log_report_config_topic_ready;
    bool stream_log_config_topic_ready;
    bool local_log_config_topic_ready;
} log_handler_t;

log_handler_t *aiot_log_init(void);

int aiot_log_set_mqtt_handler(log_handler_t *handle, iot_mqtt_ctx_t *mqtt_handle);

void aiot_log_set_report_switch(log_handler_t *handle, bool is_upload_log, enum onesdk_log_level lowest_level);

void aiot_log_deinit(log_handler_t *handle);


#endif //ONESDK_IOT_LOG_REPORT_H
