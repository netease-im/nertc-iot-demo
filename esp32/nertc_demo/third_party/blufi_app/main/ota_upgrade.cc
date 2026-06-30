/**
 * OTA 升级模块
 *
 * 从 blufi_app.cc 拆出的 OTA 升级逻辑。
 * 通过 blufi_hal 回调通知上层 OTA 进度和状态，
 * 不直接操作 LCD、按键、SPI 等硬件。
 */

#include "blufi_hal.h"
#include "ota.h"
#include "settings.h"
#include "ssid_manager.h"
#include "type.h"

#include <esp_log.h>
#include <wifi_station.h>

#include <string>

static const char *TAG = "OtaUpgrade";

/* 全局 OTA 实例 */
static Ota g_ota;

/* 共享状态 */
extern AppType_t g_app;

/* ============================================================
 * OTA 安装状态查询（供超时定时器使用）
 * ============================================================ */

static bool IsOtaInstalling(void) {
    return g_ota.GetInstalling();
}

/* ============================================================
 * StartNetwork — 连接已保存的 WiFi
 * ============================================================ */

bool StartNetwork() {
    auto& ssid_manager = SsidManager::GetInstance();
    auto ssid_list = ssid_manager.GetSsidList();
    if (ssid_list.empty()) {
        ESP_LOGE(TAG, "No WiFi SSID configured, cannot start network");
        return false;
    }

    auto& wifi_station = WifiStation::GetInstance();
    wifi_station.OnScanBegin([]() {
        ESP_LOGI(TAG, "Scanning WiFi networks...");
    });
    wifi_station.OnConnect([](const std::string& ssid) {
        ESP_LOGI(TAG, "Connecting to WiFi SSID: %s...", ssid.c_str());
    });
    wifi_station.OnConnected([](const std::string& ssid) {
        ESP_LOGI(TAG, "Connected to WiFi SSID: %s", ssid.c_str());
    });
    wifi_station.Start();

    if (!wifi_station.WaitForConnected(60 * 1000)) {
        ESP_LOGE(TAG, "Failed to connect to WiFi within timeout");
        return false;
    }

    ESP_LOGI(TAG, "WiFi connected, IP: %s", wifi_station.GetIpAddress().c_str());
    return true;
}

/* ============================================================
 * RunOta — OTA 升级主流程
 * ============================================================ */

void RunOta() {
    /* 注册 OTA 安装状态查询（防止超时定时器在 OTA 期间触发重启） */
    blufi_hal_set_ota_installing_check(IsOtaInstalling);

    /* 通过 HAL 通知上层 OTA 开始 */
    const blufi_hal_t *hal = blufi_hal_get();

    if (!StartNetwork()) {
        ESP_LOGE(TAG, "Failed to start network, cannot proceed with OTA");
        if (hal->on_ota_complete) {
            hal->on_ota_complete(false, hal->user_ctx);
        }
        return;
    }

    /* 读取 OTA 参数 */
    std::string url;
    std::string version;
    std::string md5;
    {
        Settings settings("board", false);
        url = settings.GetString("ota_url");
        version = settings.GetString("ota_v");
        if (version.empty()) {
            version = settings.GetString("version");
        }
        md5 = settings.GetString("ota_md5");
    }

    ESP_LOGI(TAG, "Starting OTA: URL=%s, Version=%s, MD5=%s",
             url.c_str(), version.c_str(), md5.c_str());

    if (!url.empty()) {
        g_ota.SetFirmwareUrl(url);

        for (int count = 0; count < 3; ++count) {
            bool res = g_ota.StartUpgrade(md5, [](const std::string& status, int progress, size_t speed) {
                ESP_LOGI(TAG, "OTA: %s %d%%, %u KB/s", status.c_str(), progress, (unsigned int)(speed / 1024));

                const blufi_hal_t *h = blufi_hal_get();
                if (h->on_ota_progress) {
                    h->on_ota_progress(progress, h->user_ctx);
                }
            });

            if (!res) {
                ESP_LOGW(TAG, "OTA attempt %d failed, retrying...", count + 1);
                vTaskDelay(pdMS_TO_TICKS(1000));
            } else {
                /* OTA 成功，通知上层并重启 */
                if (hal->on_ota_complete) {
                    hal->on_ota_complete(true, hal->user_ctx);
                }
                blufi_restart_app();
                return;
            }
        }
    }

    /* OTA 失败 */
    ESP_LOGW(TAG, "OTA failed after 3 attempts");
    if (hal->on_ota_complete) {
        hal->on_ota_complete(false, hal->user_ctx);
    }

    {
        Settings settings("board", true);
        settings.SetInt("ota_fail", 1);
        ESP_LOGW(TAG, "Set ota_fail flag in NVS");
    }

    blufi_restart_app();
}
