/**
 * BLE BluFi 配网模块
 *
 * 从 blufi_app.cc 拆出的 BLE BluFi 配网逻辑。
 * 不直接操作任何硬件（LCD、按键、SPI），通过 blufi_hal 回调与上层交互。
 */

#ifndef _BLUFI_PROVISIONING_H_
#define _BLUFI_PROVISIONING_H_

#include "type.h"
#include "blufi_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 启动 BLE BluFi 配网流程 */
void RunBlufi();

#ifdef __cplusplus
}
#endif

#endif /* _BLUFI_PROVISIONING_H_ */
