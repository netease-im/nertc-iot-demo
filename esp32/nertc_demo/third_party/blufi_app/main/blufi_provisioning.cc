/**
 * BLE BluFi 配网实现
 *
 * 从 blufi_app.cc 拆出。通过 blufi_hal 回调与上层交互，
 * 不直接操作 LCD、按键、SPI 等硬件。
 */

#include "blufi_provisioning.h"
#include "blufi_hal.h"
#include "blufi_wifi.h"
#include "settings.h"
#include "ssid_manager.h"

#include <esp_log.h>
#include <esp_mac.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <wifi_configuration_ap.h>

#include <string>

static const char *TAG = "BlufiProv";

/* 共享状态（在 blufi_app.cc 中声明为 extern） */
extern AppType_t g_app;
extern blufi_custom_event_callback_t g_custom_event_callback;
extern TaskHandle_t g_BlufiTask_handle;
extern TaskHandle_t g_wifi_ap_task_handle;

/* 内部结构体 */
struct TaskParam {
    std::string broad_name;
    std::string broad_info;
    std::string broad_type;
};

/* 前向声明 */
static void BlufiTask(const std::string& broad_name, const std::string& broad_info);
static void RunWifiApMode(const std::string&& name);
static void OnBlufiCustomEventCallBack(BlufiCustomEvent_t event, void* data, int data_len);

/* ============================================================
 * RunBlufi — BLE BluFi 配网主流程
 * ============================================================ */

void RunBlufi() {
    Settings settings("board", true);

    int blufi_mode = settings.GetInt("blufi", 1);
    int test_mode = settings.GetInt("test", 0);
    ESP_LOGI(TAG, "Read board blufi mode: %d, test_mode: %d", blufi_mode, test_mode);

    /* 读取设备名 */
    std::string broad_name = settings.GetString("name");
    ESP_LOGI(TAG, "Read board name from nvs: %s", broad_name.c_str());

    std::string id = settings.GetString("id");
    std::string type = settings.GetString("type");
    std::string version = settings.GetString("version");
    int enable_4g = settings.GetInt("4g", 1);

    std::string appkey = settings.GetString("appkey");
    // 脱敏：前4位 + 中间* + 后4位
    std::string masked;
    if (appkey.length() > 8) {
        masked = appkey.substr(0, 4) 
            + std::string(appkey.length() - 8, '*') 
            + appkey.substr(appkey.length() - 4);
    } else {
        masked = appkey;  // 长度不足时不打码，避免信息全丢
    }

    ESP_LOGI(TAG, "Read board id: %s, type: %s, version: %s, 4g: %d appkey: %s",
             id.c_str(), type.c_str(), version.c_str(), enable_4g, masked.c_str());

    /* 清理一次性 activation 信息 */
    std::string code = settings.GetString("act_code");
    std::string activation_msg = settings.GetString("act_msg");
    size_t pos = activation_msg.find('\n');
    if (pos != std::string::npos) {
        activation_msg = activation_msg.substr(0, pos);
    }
    ESP_LOGI(TAG, "Activation code: %s, msg: %s", code.c_str(), activation_msg.c_str());
    if (!code.empty()) {
        settings.EraseKey("act_code");
        settings.EraseKey("act_msg");
    }

    /* 清理一次性 extra_msg */
    std::string extra_msg = settings.GetString("extra_msg");
    if (!extra_msg.empty()) {
        ESP_LOGI(TAG, "Extra msg: %s", extra_msg.c_str());
        settings.EraseKey("extra_msg");
    }

    /* 如果设备名为空，根据 MAC 地址生成 */
    if (broad_name.empty()) {
        uint8_t mac[6];
#if CONFIG_IDF_TARGET_ESP32P4
        esp_wifi_get_mac(WIFI_IF_AP, mac);
#else
        ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP));
#endif
        char ssid[32];
        if (!type.empty()) {
            snprintf(ssid, sizeof(ssid), "%s-%02X%02X", type.c_str(), mac[4], mac[5]);
        } else {
            snprintf(ssid, sizeof(ssid), "%s-%02X%02X", "小派", mac[4], mac[5]);
        }
        broad_name = std::string(ssid);
    }

    /* 构造广播信息 JSON */
    std::string broad_info = "{";
    broad_info += "\"id\":\"" + id + "\",";
    broad_info += "\"type\":\"" + type + "\",";
    broad_info += "\"version\":\"" + version + "\",";
    broad_info += "\"name\":\"" + broad_name + "\",";
    broad_info += "\"appkey\":\"" + appkey + "\",";
    broad_info += "\"4g\":" + std::to_string(enable_4g) + "";
    broad_info += "}";

    ESP_LOGW(TAG, "Broad name: %s, type: %s, id: %s, version: %s, 4g: %d",
             broad_name.c_str(), type.c_str(), id.c_str(), version.c_str(), enable_4g);

    /* 通过 HAL 回调通知上层展示设备信息 */
    const blufi_hal_t *hal = blufi_hal_get();
    if (hal->display_device_info) {
        hal->display_device_info(broad_name.c_str(), broad_info.c_str(), hal->user_ctx);
    }
    if (hal->on_provision_start) {
        hal->on_provision_start(broad_name.c_str(), hal->user_ctx);
    }

    /* 注册自定义事件回调 */
    g_custom_event_callback = OnBlufiCustomEventCallBack;

    /* 创建 BlufiTask */
    TaskParam *param1 = new TaskParam{broad_name, broad_info, type};
    xTaskCreate([](void* args) {
        TaskParam* param = static_cast<TaskParam*>(args);
        BlufiTask(param->broad_name, param->broad_info);
        delete param;
        vTaskDelete(NULL);
    }, "BlufiTask", 1024 * 8, param1, 5, &g_BlufiTask_handle);

    /* 创建 WiFi AP Task */
    TaskParam *param2 = new TaskParam{broad_name, broad_info, type};
    xTaskCreate([](void* args) {
        TaskParam* param = static_cast<TaskParam*>(args);
        RunWifiApMode(std::move(param->broad_type));
        delete param;
        vTaskDelete(NULL);
    }, "wifi_ap_task", 1024 * 8, param2, 5, &g_wifi_ap_task_handle);

    /* 打印内存信息 */
    int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    int min_free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "free sram: %u, minimal sram: %u", free_sram, min_free_sram);
    int free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    int min_free_psram = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "free psram: %u, minimal psram: %u", free_psram, min_free_psram);
}

/* ============================================================
 * BlufiTask — BLE BluFi 配网任务
 * ============================================================ */

static void BlufiTask(const std::string& broad_name, const std::string& broad_info) {
    ESP_LOGI(TAG, "run BlufiTask");

    wifi_credential_t wifi_cred = initialise_wifi_and_blufi(broad_name.c_str(), broad_info.c_str(), broad_info.size());
    if (wifi_cred.succ == 1) {
        ESP_LOGI(TAG, "BLUFI WiFi connected! SSID: %s", wifi_cred.ssid);
        SsidManager::GetInstance().AddSsid(wifi_cred.ssid, wifi_cred.password);

        /* 通知网络类型 */
        OnBlufiCustomEventCallBack(BLUFI_CUSTOM_WIFI_START_EVENT, NULL, 0);

        /* 通过 HAL 回调通知配网成功 */
        const blufi_hal_t *hal = blufi_hal_get();
        if (hal->on_provision_success) {
            hal->on_provision_success(wifi_cred.ssid, hal->user_ctx);
        }
    } else {
        ESP_LOGE(TAG, "BLUFI WiFi connection failed");

        const blufi_hal_t *hal = blufi_hal_get();
        if (hal->on_provision_fail) {
            hal->on_provision_fail(-1, hal->user_ctx);
        }
    }

    ESP_LOGI(TAG, "BlufiTask done");
    blufi_restart_app();
}

/* ============================================================
 * RunWifiApMode — WiFi AP 配网模式（内嵌在 provisioning 中）
 * ============================================================ */

static void RunWifiApMode(const std::string&& name) {
    ESP_LOGI(TAG, "run RunWifiApMode");
    auto& wifi_ap = WifiConfigurationAp::GetInstance();
    wifi_ap.SetLanguage("zh-CN");
    wifi_ap.SetSsidPrefix(std::move(name));
    wifi_ap.SetJoinCallBack([](){
        ESP_LOGI(TAG, "wifi_ap join");
        uninitialise_blufi();
        vTaskDelay(pdMS_TO_TICKS(50));
    });
    wifi_ap.SetDoneCallBack([](){
        ESP_LOGI(TAG, "wifi_ap done, restart app");

        const blufi_hal_t *hal = blufi_hal_get();
        if (hal->on_provision_success) {
            hal->on_provision_success("ap_mode", hal->user_ctx);
        }

        blufi_restart_app();
    });
    wifi_ap.Start();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/* ============================================================
 * OnBlufiCustomEventCallBack — 自定义事件处理
 * ============================================================ */

static void OnBlufiCustomEventCallBack(BlufiCustomEvent_t event, void* data, int data_len) {
    ESP_LOGI(TAG, "Custom Event: %d, Data Length: %d", (int)event, data_len);

    if (event == BLUFI_CUSTOM_READY_EVENT)
        return;

    {
        Settings settings("network", true);
        std::string type = "type";
        settings.EraseKey(type.c_str());

        switch (event) {
        case BLUFI_CUSTOM_CONNECTED_EVENT: {
            auto& wifi_ap = WifiConfigurationAp::GetInstance();
            if (wifi_ap.IsStarted())
                wifi_ap.Stop();
            break;
        }
        case BLUFI_CUSTOM_4G_START_EVENT: {
            settings.SetInt(type.c_str(), 1);
            ESP_LOGI(TAG, "Set network type 1 to 4G in NVS");
            break;
        }
        case BLUFI_CUSTOM_WIFI_START_EVENT: {
            settings.SetInt(type.c_str(), 0);
            ESP_LOGI(TAG, "Set network type 0 to wifi in NVS");
            break;
        }
        default:
            break;
        }
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    if (event == BLUFI_CUSTOM_4G_START_EVENT) {
        blufi_restart_app();
    }
}
