/**
 * BluFi 硬件抽象层实现
 *
 * 包含：HAL 回调存储、RestartApp（与按键触发一致）、超时定时器
 */

#include "blufi_hal.h"

#include <string.h>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

static const char *TAG = "BlufiHAL";

/* 全局 HAL 实例 */
static blufi_hal_t s_hal = {0};

/* OTA 安装状态查询 */
static bool (*s_ota_installing_check)(void) = NULL;

/* 超时定时器 */
static TimerHandle_t s_timeout_timer = NULL;

/* ============================================================
 * HAL 回调管理
 * ============================================================ */

void blufi_hal_set(const blufi_hal_t *hal) {
    if (hal == NULL) {
        memset(&s_hal, 0, sizeof(s_hal));
        return;
    }
    memcpy(&s_hal, hal, sizeof(blufi_hal_t));
    ESP_LOGI(TAG, "HAL callbacks registered");
}

const blufi_hal_t* blufi_hal_get(void) {
    return &s_hal;
}

void blufi_hal_set_ota_installing_check(bool (*check_fn)(void)) {
    s_ota_installing_check = check_fn;
}

/* ============================================================
 * RestartApp — 与原有按键触发逻辑一致
 * ============================================================ */

void blufi_restart_app(void) {
    /* 设置下一次启动分区为 ota_0（主工程的应用在 ota_0） */
    const esp_partition_t *ota0 = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP,
            ESP_PARTITION_SUBTYPE_APP_OTA_0,
            NULL);
    if (ota0 != NULL) {
        ESP_ERROR_CHECK(esp_ota_set_boot_partition(ota0));
        ESP_LOGI(TAG, "Switching to ota_0 partition: %s at offset 0x%lx, restarting...",
                 ota0->label, ota0->address);
    } else {
        ESP_LOGE(TAG, "ota_0 partition not found!");
    }

    vTaskDelay(pdMS_TO_TICKS(200));
    esp_restart();
}

/* ============================================================
 * 超时定时器
 * ============================================================ */

/* 重启任务：在独立 task 中执行重启，避免定时器回调栈溢出 */
static void restart_task(void *arg) {
    ESP_LOGW(TAG, "BluFi timeout reached, restarting to main app...");
    vTaskDelay(pdMS_TO_TICKS(200));
    blufi_restart_app();
}

static void timeout_timer_callback(TimerHandle_t timer) {
    /* OTA 安装期间不触发超时重启 */
    if (s_ota_installing_check && s_ota_installing_check()) {
        xTimerChangePeriod(s_timeout_timer, pdMS_TO_TICKS(30000), 0);
        return;
    }

    /* 创建独立任务执行重启，栈空间充足，不依赖主循环 */
    xTaskCreate(restart_task, "blufi_restart", 4096, NULL, 5, NULL);
}

void blufi_start_timeout_timer(uint32_t timeout_seconds) {
    if (s_timeout_timer != NULL) {
        xTimerStop(s_timeout_timer, 0);
        xTimerDelete(s_timeout_timer, 0);
        s_timeout_timer = NULL;
    }

    s_timeout_timer = xTimerCreate(
        "blufi_timeout",
        pdMS_TO_TICKS(timeout_seconds * 1000),
        pdFALSE,  /* 单次触发，不自动重载 */
        NULL,
        timeout_timer_callback
    );

    if (s_timeout_timer != NULL) {
        xTimerStart(s_timeout_timer, 0);
        ESP_LOGI(TAG, "Timeout timer started: %lu seconds", (unsigned long)timeout_seconds);
    } else {
        ESP_LOGE(TAG, "Failed to create timeout timer");
    }
}

void blufi_stop_timeout_timer(void) {
    if (s_timeout_timer != NULL) {
        xTimerStop(s_timeout_timer, 0);
        xTimerDelete(s_timeout_timer, 0);
        s_timeout_timer = NULL;
        ESP_LOGI(TAG, "Timeout timer stopped");
    }
}
