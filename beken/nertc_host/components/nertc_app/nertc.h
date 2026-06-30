#ifndef __NERTC_H__
#define __NERTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nertc_config.h"

typedef enum
{
    NERTC_MSG_JOIN_CHANNEL_RESULT = 0,
    NERTC_MSG_REJOIN_CHANNEL_RESULT,
    NERTC_MSG_USER_JOINED,
    NERTC_MSG_USER_LEAVED,
    NERTC_MSG_CONNECTION_LOST,
    NERTC_MSG_INVALID_CHANNEL_NAME,
    NERTC_MSG_LICENSE_EXPIRED,

    NERTC_MSG_KEY_FRAME_REQUEST,
    NERTC_MSG_BWE_TARGET_BITRATE_UPDATE
} nertc_msg_e;

typedef enum
{
    NERTC_STATE_NULL = 0,
    NERTC_STATE_IDLE,
    NERTC_STATE_WORKING,
} nertc_state_t;

typedef struct
{
    uint32_t target_bitrate;
} nertc_bwe_t;

typedef struct
{
    nertc_msg_e msg_code;
    int32_t code;
    uint64_t uid;
} nertc_msg_t;

typedef void (*nertc_msg_notify_cb)(nertc_msg_t *p_msg);
typedef int (*nertc_audio_rx_data_handler)(uint64_t uid, nertc_sdk_audio_encoded_frame_t* encoded_frame, bool is_mute_packet);

typedef struct
{
    char *p_app_key;
    char *p_device_id;
    char *p_license;
    char *p_custom_setting;

    bool enable_server_aec;
} nertc_config_t;

#define DEFAULT_NERTC_CONFIG() {                  \
    .p_app_key = NULL,                            \
    .p_device_id = NULL,                          \
    .p_license = NULL,                            \
    .p_custom_setting = NULL,                     \
    .enable_server_aec = false,                   \
}

typedef struct
{
    char *p_channel_name;
    uint32_t uid;;
} nertc_option_t;

#define DEFAULT_NERTC_OPTION() {                  \
    .p_channel_name = NULL,                       \
    .uid = 0,                                     \
}

typedef struct
{
    nertc_sdk_engine_t engine;
    nertc_state_t state;
    nertc_msg_notify_cb user_message_callback;
    bool has_channel_joined;
    bool has_ai_started;

    nertc_audio_rx_data_handler audio_rx_data_handler;
} nertc_t;

bk_err_t bk_nertc_create(nertc_config_t *p_config, nertc_msg_notify_cb message_callback);
bk_err_t bk_nertc_destroy(void);

bk_err_t bk_nertc_start(nertc_option_t *p_option);
bk_err_t bk_nertc_stop(void);

bk_err_t bk_nertc_start_ai(void);
bk_err_t bk_nertc_stop_ai(void);

bk_err_t bk_nertc_start_listen(void);
bk_err_t bk_nertc_stop_listen(void);

int bk_nertc_audio_data_send(uint8_t *data_ptr, size_t data_len);
bk_err_t bk_nertc_register_audio_rx_handle(nertc_audio_rx_data_handler audio_rx_handler);

#ifdef __cplusplus
}
#endif
#endif /* __NERTC_H__ */
