#ifndef _BLUFI_WIFI_H_
#define _BLUFI_WIFI_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    BLUFI_CUSTOM_CONNECTED_EVENT = 0,
    BLUFI_CUSTOM_READY_EVENT = 1,
    BLUFI_CUSTOM_4G_START_EVENT,
    BLUFI_CUSTOM_WIFI_START_EVENT,
} BlufiCustomEvent_t;

typedef void (*blufi_custom_event_callback_t)(BlufiCustomEvent_t event, void* data, int data_len);

typedef struct {
    int succ;
    char ssid[32];      // SSID最大长度32字节（ESP32限制）
    char password[64];  // 密码最大长度64字节（ESP32限制）
} wifi_credential_t;

wifi_credential_t initialise_wifi_and_blufi(const char* prefix, const char* device_info, size_t device_info_len);

void uninitialise_blufi();

#ifdef __cplusplus
}
#endif

#endif