# BK7258 NERTC Demo — README

- 日期：2026-06-25
- 芯片平台：Beken BK7258（三核 Cortex-M33）
- 引擎模式：LITE（MQTT 信令，替代 AI 模式的 WebSocket）
- 仓库路径：`demo/bk7258/`

---

## 1. 工程结构

```
demo/bk7258/
├── bk_aidk/                          # Beken 官方 SDK（Git 子模块，只读）
│   ├── bk_avdk/                      #   音视频 SDK（嵌套子模块）
│   │   └── bk_idk/                   #     IoT SDK 核心（嵌套子模块）
│   ├── components/                   #   官方组件（多媒体、WiFi、音频、蓝牙等）
│   ├── properties/                     #   属性配置
│   └── tools/                        #   构建工具链
│
├── nertc_host/                       # 自有 NERTC Demo 工程
│   ├── main/                           # 应用主入口与平台初始化
│   ├── components/                     # 自有组件
│   │   ├── nertc_app/                  #   NERTC SDK 封装 + MQTT 信令 + OTA
│   │   └── bk_smart_config/            #   配网与云端适配（NERTC 适配器）
│   ├── config/                         # 板级配置（三核）
│   │   ├── bk7258/                     #   CPU0 主配置
│   │   ├── bk7258_cp1/                 #   CPU1 配置
│   │   └── bk7258_cp2/                 #   CPU2 配置
│   ├── CMakeLists.txt                  # 工程构建入口
│   ├── Makefile                        # 构建包装脚本
│   └── pj_config.mk                    # 项目级构建配置
│
├── docs/                               # 工程文档
    ├── prd-bk7258-clean-demo.md        #   PRD 设计文档
    ├── bk7258-demo-status-20260623.md    #   开发状态记录
    └── handover-20260623.md            #   交接文档
```

---

## 2. 初始化方法

### 2.1 拉取子模块（首次 clone 后必须执行）

```bash
# 1. 进入工程根目录
cd demo/bk7258

# 2. 初始化并递归拉取子模块（bk_aidk → bk_avdk → bk_idk）
git submodule update --init --recursive

# 3. 验证子模块状态
git submodule status
# 期望输出（无 "+" 前缀）：
#  6a8000534fd21a4b0c8ddff9d648ac25ccf52a3e demo/bk7258/bk_aidk (v2.0.1.34)
```

> **注意**：`bk_aidk` 是 Git 子模块，指向 GitHub `bekencorp/bk_aidk` 的 `ai_release/v2.0.1` 分支。其子模块 `bk_avdk` 和嵌套子模块 `bk_idk` 会自动递归拉取。

---

## 3. 主要流程

### 3.1 系统启动流程

```
上电 → main() → bk_init()
                    ↓
              user_app_main()          [CPU0]
                    ↓
         ┌──────────────────────┐
         │ 1. 设置 CPU 频率 240MHz│
         │ 2. audio_engine_init() │
         │ 3. video_engine_init() │
         │ 4. network_transfer_init()│
         │ 5. nertc_cli_init()     │
         └──────────────────────┘
                    ↓
         ┌──────────────────────┐
         │ 6. media_service_init()│
         │ 7. app_event_init()    │
         │ 8. nertc_app_event_init()│
         │ 9. volume_init()       │
         │ 10. audio_turn_on()     │
         └──────────────────────┘
                    ↓
         ┌──────────────────────┐
         │ 11. 启动 CP1 音频核    │
         │ 12. bk_smart_config_init()│  ← WiFi 配网 + 云端接入
         │ 13. bk_key_service_init()  │  ← 按键服务
         └──────────────────────┘
```

### 3.2 NERTC 连接主流程

```
WiFi 连接成功
    ↓
MQTT 配置获取（可选，通过 OTA）
    ↓
nertc_app_init()  →  bk_nertc_create()
                         ↓
                    __nertc_init()
                         ↓
                    ┌─────────────────────────┐
                    │ 1. 配置 nertc_sdk_config │
                    │    - app_key, device_id  │
                    │    - license            │
                    │    - audio: OPUS, 16kHz │
                    │    - server AEC         │
                    │ 2. 创建引擎             │
                    │ 3. 配置引擎模式 (LITE)   │
                    │ 4. 注入 ext_net_handle   │  ← MQTT 桥接
                    │ 5. 注册事件回调         │
                    │ 6. 初始化引擎           │
                    └─────────────────────────┘
                         ↓
                    bk_nertc_start()
                         ↓
                    nertc_join(channel_name, uid)
                         ↓
                    ┌─────────────────────────┐
                    │ __on_join() 回调触发      │
                    │   → NERTC_MSG_JOIN_CHANNEL_RESULT
                    │   → 进房成功              │
                    │ __on_user_joined()       │
                    │   → 远端用户加入          │
                    │   → bk_nertc_start_listen() ← 开启麦克风监听
                    └─────────────────────────┘
```

### 3.3 音频数据流

```
上行（麦克风 → NERTC）
    audio_engine 采集线程
        ↓
    audio_transfer 环形缓冲区
        ↓
    bk_nertc_audio_data_send()
        ↓
    nertc_push_audio_encoded_frame() → SDK → 网络

下行（NERTC → 扬声器）
    网络 → SDK
        ↓
    __on_audio_data() 回调
        ↓
    nertc_audio_rx_data_handle() → audio_engine → 扬声器
```

### 3.4 TTS 半双工控制

```
服务端下发 AI 数据（type="tts", state="start"）
    ↓
__on_ai_data() 回调
    ↓
__ai_data_state_is("start") → bk_nertc_stop_listen()
    ↓
停止麦克风采集（AI 正在说话，避免打断）

服务端下发 AI 数据（type="tts", state="stop"）
    ↓
__on_ai_data() 回调
    ↓
__ai_data_state_is("stop") → bk_nertc_start_listen()
    ↓
恢复麦克风采集（AI 说完，等待用户说话）
```

### 3.5 OTA 配置获取流程

```
nertc_ota_check_version()
    ↓
HTTP POST webtest.netease.im:80/v1/ota
    ↓
JSON 响应解析
    ↓
提取 MQTT 配置（endpoint, client_id, username, password, publish_topic）
    ↓
填充到 nertc_sdk_config.mqtt_config
    ↓
NERtc SDK 初始化时使用这些配置连接 MQTT broker
```

---

## 4. 代码结构

### 4.1 目录总览

```
nertc_host/
├── main/                             # 应用主入口
│   ├── app_main.c                    #   系统启动、初始化顺序
│   ├── app_main.h                    #   公共声明
│   ├── audio_para.c                  #   CPU0 音频参数（EQ/AEC）
│   ├── vendor_flash.c                #   自定义 flash 分区表
│   └── vendor_flash_partition.h      #   分区表头文件
│
├── components/
│   ├── nertc_app/                    # NERTC SDK 封装层
│   │   ├── nertc.c                   #   SDK 生命周期、回调、音频收发
│   │   ├── nertc.h                   #   公共接口头文件
│   │   ├── nertc_main.c              #   NERTC 主线程、CLI 命令
│   │   ├── nertc_config.h            #   编译时配置（appkey、license、MQTT）
│   │   ├── nertc_ext_mqtt_bk.c       #   MQTT ext_net_handle 桥接
│   │   ├── nertc_ext_mqtt_bk.h       #   MQTT 桥接头文件
│   │   ├── nertc_ota.c               #   OTA 版本检查与配置获取
│   │   ├── nertc_ota.h               #   OTA 接口头文件
│   │   ├── generic_mqtt.c          #   通用 MQTT 客户端（封装 ali_mqtt）
│   │   ├── generic_mqtt.h            #   MQTT 客户端头文件
│   │   └── nertc_lwip_compat.h       #   lwip 兼容性头文件
│   │
│   └── bk_smart_config/              # 配网与云端适配（NERTC 适配器）
│       ├── bk_smart_config.h
│       ├── bk_smart_config_nertc_adapter.h
│       ├── src/core/bk_smart_config_core.c
│       └── src/adapter/nertc/bk_smart_config_nertc_adapter.c
│
├── config/                           # 板级配置
│   ├── bk7258/                       #   CPU0 配置
│   │   ├── config                    #     sdkconfig（Kconfig 生成）
│   │   ├── configuration.json       #     分区配置
│   │   ├── partitions.csv           #     分区表
│   │   ├── bk7258_partitions.csv    #     分区表（构建工具用）
│   │   └── usr_gpio_cfg.h           #     GPIO 配置
│   ├── bk7258_cp1/                   #   CPU1 配置
│   └── bk7258_cp2/                   #   CPU2 配置
│
├── CMakeLists.txt                    # 工程 CMake 入口
├── Makefile                          # 构建包装脚本
└── pj_config.mk                      # 项目级构建配置
```

### 4.2 核心文件职责

#### `main/app_main.c`（~318 行）

- `main()`：系统入口，判断复位原因，初始化平台
- `user_app_main()`：应用主入口，按顺序初始化各模块
- `bk_enter_deepsleep()` / `bk_wait_power_on()`：低功耗管理
- 初始化顺序：audio_engine → video_engine → network_transfer → media_service → app_event → smart_config → key_service

#### `components/nertc_app/nertc.c`（~797 行）

- `bk_nertc_create()` / `bk_nertc_destroy()`：SDK 生命周期管理
- `bk_nertc_start()` / `bk_nertc_stop()`：进房/退房
- `bk_nertc_start_ai()` / `bk_nertc_stop_ai()`：AI 会话控制
- `bk_nertc_start_listen()` / `bk_nertc_stop_listen()`：麦克风监听控制
- `bk_nertc_audio_data_send()`：上行音频数据发送
- `bk_nertc_register_audio_rx_handle()`：注册下行音频回调
- 内部函数：
  - `__nertc_init()`：SDK 初始化（配置、创建引擎、注入 ext_net_handle）
  - `__on_join()` / `__on_disconnect()`：连接状态回调
  - `__on_user_joined()` / `__on_user_left()`：远端用户事件
  - `__on_ai_data()`：AI 数据回调（TTS 状态解析）
  - `__on_audio_data()`：音频数据回调（下行）

#### `components/nertc_app/nertc_main.c`（~611 行）

- `nertc_app_init()`：NERTC 应用初始化（从 MAC 地址生成 device_id）
- `nertc_thread()`：NERTC 主线程（消息循环）
- `nertc_user_notify_msg_handle()`：用户消息分发（JOIN、LEAVE、CONNECTION_LOST 等）
- `nertc_audio_rx_data_handle()`：音频接收处理（写入 audio_engine）
- CLI 命令：`nertc_test`（启动/停止 NERTC）
- `nertc_get_sta_mac_string()`：从 WiFi STA MAC 生成设备 ID

#### `components/nertc_app/nertc_ext_mqtt_bk.c`（~314 行）

- `nertc_ext_mqtt_fill_handle()`：填充 `nertc_sdk_ext_net_handle_t` 结构体
- `create_mqtt()` / `destroy_mqtt()`：MQTT 实例生命周期
- `mqtt_connect()` / `mqtt_disconnect()`：连接/断开 MQTT broker
- `mqtt_publish()` / `mqtt_subscribe()` / `mqtt_unsubscribe()`：消息发布/订阅
- `set_mqtt_on_*()`：注册 SDK 回调
- 内部使用 `generic_mqtt.c` 作为 MQTT 传输层

#### `components/nertc_app/generic_mqtt.c`（~604 行）

- `gen_mqtt_new()` / `gen_mqtt_free()`：MQTT 客户端生命周期
- `gen_mqtt_connect()` / `gen_mqtt_disconnect()`：连接管理
- `gen_mqtt_publish()` / `gen_mqtt_subscribe()`：消息操作
- `gen_mqtt_yield()`：消息轮询（内部调用 `IOT_MQTT_Yield()`）
- 基于 `ali_mqtt`（阿里云 IoT MQTT 客户端库）

#### `components/nertc_app/nertc_ota.c`（~360 行）

- `nertc_ota_check_version()`：检查 OTA 版本并获取 MQTT 配置
- `build_http_request()`：构造 HTTP POST 请求
- `parse_ota_response()`：解析 JSON 响应
- 使用 lwip socket API 直接进行 HTTP 通信

### 4.3 关键数据结构

```c
// nertc.h — NERTC 应用层封装

typedef enum {
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

typedef enum {
    NERTC_STATE_NULL = 0,
    NERTC_STATE_IDLE,
    NERTC_STATE_WORKING,
} nertc_state_t;

typedef struct {
    nertc_sdk_engine_t engine;
    nertc_state_t state;
    nertc_msg_notify_cb user_message_callback;
    bool has_channel_joined;
    bool has_ai_started;
    nertc_audio_rx_data_handler audio_rx_data_handler;
} nertc_t;
```

---

## 5. 配置说明

### 5.1 Kconfig 配置（`main/Kconfig.projbuild`）

| 配置项 | 说明 | 默认值 |
|--------|------|--------|
| `NERTC_WIFI_SSID` | WiFi STA SSID | "" |
| `NERTC_WIFI_PASSWORD` | WiFi STA 密码 | "" |
| `HARDWARE_SPEAKER_VER` | 硬件扬声器版本 | 0 |
| `G722_CODEC_RUN_CPU` | G722 编解码器运行核 | CPU1 |

### 5.2 编译时配置（`nertc_config.h`）

| 配置项 | 说明 | 默认值 |
|--------|------|--------|
| `CONFIG_NERTC_APPKEY` | NERTC App Key | `xxxxxxxx` |
| `CONFIG_NERTC_SERVER_AEC` | 服务端 AEC 开关 | false |
| `CONFIG_NERTC_ENGINE_MODE` | SDK 引擎模式 | `NERTC_SDK_ENGINE_MODE_LITE` |
| `CONFIG_NERTC_MQTT_ENDPOINT` | MQTT Broker 地址 | "" |
| `CONFIG_NERTC_MQTT_CLIENT_ID` | MQTT Client ID | "" |
| `CONFIG_NERTC_MQTT_USERNAME` | MQTT 用户名 | "" |
| `CONFIG_NERTC_MQTT_PASSWORD` | MQTT 密码 | "" |
| `CONFIG_NERTC_MQTT_PUBLISH_TOPIC` | MQTT 发布主题 | "" |
| `CONFIG_NERTC_TEST_UID` | 测试用户 UID | 6669 |

### 5.3 板级配置（`config/bk7258/config`）

| 配置项 | 说明 | 值 |
|--------|------|-----|
| `CONFIG_CPU_FREQ_HZ` | CPU 频率 | 240MHz |
| `CONFIG_AUDIO_ADC_SAMP_RATE` | ADC 采样率 | 16000 |
| `CONFIG_AUDIO_DAC_SAMP_RATE` | DAC 采样率 | 16000 |
| `CONFIG_AUDIO_FRAME_DURATION_MS` | 音频帧时长 | 60ms |
| `CONFIG_AUD_INTF_SUPPORT_OPUS` | OPUS 编解码支持 | y |
| `CONFIG_AEC_VERSION_V3` | AEC 版本 | V3 |
| `CONFIG_OVERRIDE_FLASH_PARTITION` | 自定义分区表 | y |

---

## 6. 依赖关系

### 6.1 nertc_app 组件依赖

```
nertc_app
├── nertc_sdk          (NERTC SDK 库)
├── ali_mqtt           (阿里云 IoT MQTT 客户端)
├── bk_common          (Beken 公共库)
├── bk_boarding_service (配网服务)
├── bk_smart_config    (智能配网)
├── bk_init            (初始化)
├── lwip_intf_v2_1     (lwip 网络栈)
├── mbedtls            (TLS/SSL)
├── psa_mbedtls        (Platform Security Architecture)
├── json               (cJSON)
├── multimedia         (多媒体框架)
├── avdk_utils         (AVDK 工具)
├── video_engine       (视频引擎)
├── audio_engine       (音频引擎)
├── bk_app_event       (应用事件)
├── bk_factory_config  (工厂配置)
└── bk_cli             (CLI 命令行)
```

### 6.2 main 组件依赖

```
main
├── nertc_app          (NERTC 应用)
├── nertc_sdk          (NERTC SDK)
├── media_service      (媒体服务)
├── multimedia         (多媒体)
├── avdk_utils         (AVDK 工具)
├── mbedtls            (TLS/SSL)
├── psa_mbedtls        (PSA)
├── json               (cJSON)
├── wanson             (离线语音识别)
├── bk_wanson_asr      (Wanson ASR)
├── bk_factory_config  (工厂配置)
├── bk_nfc             (NFC)
├── bk_app_event       (应用事件)
├── bk_countdown       (倒计时)
├── bk_led_blink       (LED 闪烁)
├── bk_motor           (马达)
├── bk_boarding_service (配网服务)
├── bk_smart_config    (智能配网)
├── bk_bt              (蓝牙)
├── bk_key_app         (按键应用)
├── asr                (语音识别)
├── audio_engine       (音频引擎)
├── video_engine       (视频引擎)
└── network_transfer   (网络传输)
```

---

## 7. 注意事项

1. **子模块只读**：`bk_aidk/` 是 Git 子模块，任何修改都应通过 `git submodule update` 恢复，不要在子模块内直接修改文件
2. **编译环境**：必须使用 Beken 官方工具链编译，本地交叉编译环境可能不匹配
3. **NERTC SDK 路径**：`CMakeLists.txt` 中通过 `$ENV{ARMINO_PATH}/../../../../../src/nertc_sdk` 引用 NERTC SDK，确保路径正确
4. **WiFi 配置**：首次烧录后需要通过 smart config 或 CLI 配置 WiFi SSID 和密码
5. **License 有效期**：测试 License 有过期时间，过期后需要重新获取
6. **分区表**：`vendor_flash.c` 定义了自定义分区表，与 SDK 默认分区不同，确保烧录时使用正确的分区表

---

## 8. 参考文档

| 文档 | 路径 |
|------|------|
| PRD 设计文档 | `docs/prd-bk7258-clean-demo.md` |
| 开发状态记录 | `docs/bk7258-demo-status-20260623.md` |
| 交接文档 | `docs/handover-20260623.md` |
| Beken 官方文档 | `bk_aidk/README.md` |
| Beken 中文文档 | `bk_aidk/README_CN.md` |
