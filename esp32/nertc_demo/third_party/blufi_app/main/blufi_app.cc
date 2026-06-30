/**
 * BluFi App 入口
 *
 * 精简后的入口文件，只负责：
 * 1. NVS 初始化
 * 2. 启动 90 秒超时定时器
 * 3. 读取 app type 并分发到配网或 OTA 模块
 *
 * 不直接操作任何硬件（按键、LCD、SPI）。
 * 硬件交互通过 blufi_hal 回调由上层注入。
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "blufi_hal.h"
#include "blufi_provisioning.h"
#include "blufi_wifi.h"
#include "settings.h"
#include "type.h"

static const char *TAG = "BlufiApp";

/* 全局共享状态 */
AppType_t g_app;
blufi_custom_event_callback_t g_custom_event_callback = nullptr;
TaskHandle_t g_BlufiTask_handle = nullptr;
TaskHandle_t g_wifi_ap_task_handle = nullptr;

/* OTA 模块函数（在 ota_upgrade.cc 中实现） */
extern void RunOta();

/* 读取 NVS 中的 app type */
static AppType_t GetAppType() {
    Settings settings("board", false);
    int32_t app = settings.GetInt("app", 1);
    AppType_t app_type = static_cast<AppType_t>(app);
    ESP_LOGI(TAG, "Board app type from NVS: %d", (int)app_type);
    return app_type;
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* 1. 初始化 NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* 2. 启动 90 秒超时定时器 */
    blufi_start_timeout_timer(BLUFI_DEFAULT_TIMEOUT_SEC);

    /* 3. 读取 app type */
    g_app = GetAppType();
    ESP_LOGI(TAG, "Starting BluFi app... type: %d", (int)g_app);

    /* 4. 根据类型分发（RunBlufi/RunOta 内部有循环，不会返回） */
    if (g_app != APP_TYPE_OTA) {
        ESP_LOGI(TAG, "App type is BLUFI, starting provisioning...");
        RunBlufi();
    } else {
        ESP_LOGI(TAG, "App type is OTA, starting upgrade...");
        RunOta();
    }
}
