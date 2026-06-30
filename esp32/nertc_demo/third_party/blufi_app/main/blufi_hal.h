/**
 * BluFi 硬件抽象层接口
 *
 * 所有回调均为可选（可传 NULL）。
 * 核心模块通过此接口与上层硬件交互，不直接操作任何硬件。
 */

#ifndef _BLUFI_HAL_H_
#define _BLUFI_HAL_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * BluFi HAL 回调结构体
 *
 * 使用方式：
 * - 上层板级代码填充需要的回调，不需要的置 NULL
 * - 在进入配网/OTA 流程前调用 blufi_hal_set() 注册
 * - 核心模块通过 blufi_hal_get() 获取并触发回调
 */
typedef struct {
    /** 配网开始回调 */
    void (*on_provision_start)(const char *device_name, void *ctx);

    /** 配网成功回调 */
    void (*on_provision_success)(const char *ssid, void *ctx);

    /** 配网失败回调 */
    void (*on_provision_fail)(int error_code, void *ctx);

    /** 展示设备信息（由上层决定展示方式：LCD、LED、串口、或不展示） */
    void (*display_device_info)(const char *device_name, const char *info, void *ctx);

    /** OTA 进度回调（progress: 0-100） */
    void (*on_ota_progress)(int progress, void *ctx);

    /** OTA 完成回调 */
    void (*on_ota_complete)(bool success, void *ctx);

    /** 用户上下文指针 */
    void *user_ctx;
} blufi_hal_t;

/**
 * 设置 HAL 回调（结构体拷贝，调用后原结构体可释放）
 * 必须在进入配网/OTA 流程前调用
 */
void blufi_hal_set(const blufi_hal_t *hal);

/**
 * 获取当前 HAL 接口（内部使用）
 */
const blufi_hal_t* blufi_hal_get(void);

/**
 * 注册 OTA 安装状态查询回调
 * 超时定时器在 OTA 安装期间不会触发重启
 */
void blufi_hal_set_ota_installing_check(bool (*check_fn)(void));

/**
 * 重启回主程序
 * 设置 boot partition 为 ota_0，然后 esp_restart()
 * 行为与原有按键触发逻辑一致
 */
void blufi_restart_app(void);

/** 默认超时秒数 */
#define BLUFI_DEFAULT_TIMEOUT_SEC 90

/**
 * 启动配网超时定时器
 * @param timeout_seconds 超时秒数（默认 90）
 * 超时后创建独立任务执行重启
 */
void blufi_start_timeout_timer(uint32_t timeout_seconds);

/**
 * 停止配网超时定时器
 */
void blufi_stop_timeout_timer(void);

#ifdef __cplusplus
}
#endif

#endif /* _BLUFI_HAL_H_ */
