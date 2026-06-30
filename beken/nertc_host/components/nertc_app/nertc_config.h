#pragma once

#include "nertc_sdk.h"
#include "audio_config.h"

#define OTA_HOST         "nrtc.netease.im"
#define OTA_PORT         80

#define CONFIG_NERTC_APPKEY             "xxxxxxxxxxxx"
#define CONFIG_NERTC_CUSTOM_SETTING     "{}"

/**
 * @brief Engine mode — set to NERTC_SDK_ENGINE_MODE_LITE for MQTT signaling.
 *        Default: NERTC_SDK_ENGINE_MODE_AI (WebSocket signaling)
 */
#ifndef CONFIG_NERTC_ENGINE_MODE
#define CONFIG_NERTC_ENGINE_MODE          NERTC_SDK_ENGINE_MODE_LITE // NERTC_SDK_ENGINE_MODE_NORMAL
#endif

#define CONFIG_NERTC_TEST_UID             6669

/* ------------------------------------------------------------------ */
/*  WiFi STA fallback — used when no saved network config in flash    */
/* ------------------------------------------------------------------ */
#ifndef NERTC_WIFI_SSID
#define NERTC_WIFI_SSID      "xxxxxxxx"
#endif

#ifndef NERTC_WIFI_PASSWORD
#define NERTC_WIFI_PASSWORD  "xxxxxxxx"
#endif
