# BluFi 配网模块

独立可烧录的 BLE + WiFi AP 配网工程，通过 HAL 抽象层与主程序解耦，可作为 OTA 入口集成到任意 ESP32 项目。

## 架构

```
blufi_app (ota_1)                    main app (ota_0)
┌─────────────────────┐              ┌─────────────────────┐
│  blufi_app.cc       │              │  board.cc           │
│  ├─ NVS init        │   restart    │  ├─ blufi_hal_set() │
│  ├─ 90s timeout     │──────────▶   │  └─ 注册 HAL 回调   │
│  └─ dispatch        │              └─────────────────────┘
│                     │
│  blufi_provisioning │   HAL callbacks
│  ├─ BLE BluFi       │◀──────────── display_device_info
│  └─ WiFi AP         │              on_provision_start/success/fail
│                     │              on_ota_progress/complete
│  ota_upgrade.cc     │
│  └─ RunOta()        │
│                     │
│  blufi_hal.c        │
│  ├─ 回调存储        │
│  ├─ 超时定时器      │
│  └─ restart_app()   │
└─────────────────────┘
```

## 文件结构

```
main/
├── blufi_app.cc            # 入口：NVS → 90s 超时 → 分发配网/OTA
├── blufi_hal.c/h           # HAL 接口：回调注册、超时定时器、重启
├── blufi_provisioning.cc/h # BLE BluFi + WiFi AP 配网
├── ota_upgrade.cc          # OTA 升级流程（连接 WiFi → 下载 → 重启）
├── ota.cc/h                # OTA 核心逻辑（版本检查、固件下载、激活）
├── blufi_wifi.c/h          # WiFi/BLE BluFi 协议实现
├── blufi_init.c            # BluFi 初始化
├── blufi_impl.h            # BluFi 内部结构定义
├── blufi_security.c        # 加密/安全模块
├── blufi_http_client.cc/h  # HTTP 客户端（OTA 服务器通信）
├── settings.cc/h           # NVS 设置封装
├── type.h                  # 共享类型定义（AppType_t）
├── partitions.csv          # 分区表（须与主工程一致）
└── idf_component.yml       # 组件依赖
```

## 分区表

blufi_app 必须与主工程使用**相同的分区表**。当前默认使用 16MB 分区表：

| 分区 | 类型 | 偏移 | 大小 | 说明 |
|------|------|------|------|------|
| ota_0 | app | 0x30000 | 8MB | 主程序 |
| blufi | app (ota_1) | 0x830000 | 2.5MB | 配网工程 |

如果主工程使用其他分区表，将对应文件复制为 `partitions.csv`：

```bash
cp ../../partitions/v1/16m.csv partitions.csv
```

## 集成方式

### 1. 主工程注册 HAL 回调

```cpp
#include "blufi_hal.h"

blufi_hal_t hal = {};
hal.display_device_info = [](const char *name, const char *info, void *ctx) {
    // 上层自行决定展示方式：串口打印、LED、或其他外设
    printf("Device: %s, Info: %s\n", name, info);
};
hal.on_provision_start = [](const char *device_name, void *ctx) {
    // 配网开始
};
hal.on_provision_success = [](const char *ssid, void *ctx) {
    // 配网成功
};
hal.on_provision_fail = [](int error_code, void *ctx) {
    // 配网失败
};
hal.on_ota_progress = [](int progress, void *ctx) {
    // OTA 进度 0-100
};
hal.on_ota_complete = [](bool success, void *ctx) {
    // OTA 完成
};

blufi_hal_set(&hal);
```

所有回调均为可选，不需要的置 `NULL` 即可。

### 2. 生成 blufi_app.bin

```bash
cd third_party/blufi_app
idf.py build
cd ../..
```

编译成功后，固件位于 `third_party/blufi_app/build/blufi_app.bin`。

### 3. 烧录 blufi_app

```bash
esptool.py --chip esp32s3 \
           --port /dev/cu.usbmodem1434201 \
           write_flash 0x520000 \
           third_party/blufi_app/build/blufi_app.bin
```

> 注意：`0x520000` 须与你的分区表中 blufi/ota_1 分区的偏移地址一致。

### 4. 进入配网模式

主工程通过设置 NVS 并重启进入 blufi_app：

```cpp
// 设置下次启动分区为 blufi
const esp_partition_t *blufi = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
esp_ota_set_boot_partition(blufi);

// 设置 app type（1=配网，2=OTA）
Settings settings("board", true);
settings.SetInt("app", 1);  // 配网模式
// settings.SetInt("app", 2);  // OTA 模式

esp_restart();
```

## 工作流程

### 配网模式（app=1）

1. blufi_app 启动，启动 90 秒超时定时器
2. 启动 BLE BluFi 广播 + WiFi AP 热点
3. 用户通过手机 App（BLE）或浏览器（WiFi AP 页面）配置 WiFi
4. 配网成功 → `blufi_restart_app()` → 切换 boot 分区到 ota_0 → 重启回主程序
5. 90 秒无操作 → 超时自动重启回主程序

### OTA 模式（app=2）

1. blufi_app 启动，启动 90 秒超时定时器
2. 连接已保存的 WiFi
3. 从 NVS 读取 OTA URL/版本/MD5，下载固件
4. OTA 成功 → 重启回主程序
5. 超时定时器在 OTA 下载期间自动延长，不会中断升级

## 超时机制

- 默认 90 秒，可通过 `blufi_start_timeout_timer(seconds)` 自定义
- 超时后创建独立 FreeRTOS 任务执行重启（避免定时器回调栈溢出）
- OTA 下载期间自动延长超时，不会中断固件下载

## NVS 配置项

blufi_app 从 `board` 命名空间读取以下配置（由主工程写入）：

| Key | 类型 | 说明 |
|-----|------|------|
| app | int | 1=配网模式，2=OTA 模式 |
| name | string | 设备名（空则根据 MAC 自动生成） |
| id | string | 设备 ID |
| type | string | 设备类型 |
| version | string | 固件版本 |
| 4g | int | 是否支持 4G |
| appkey | string | 应用密钥 |
| ota_url | string | OTA 服务器地址（OTA 模式） |
| ota_v | string | OTA 目标版本（OTA 模式） |
| ota_md5 | string | OTA 固件 MD5（OTA 模式） |
| act_code | string | 激活码（一次性，读取后清除） |
| act_msg | string | 激活消息（一次性，读取后清除） |

## 构建

```bash
cd third_party/blufi_app
idf.py set-target esp32s3
idf.py build
```
