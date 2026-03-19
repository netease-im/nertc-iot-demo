#ifndef _LE_GIZWITS_PROFILE_H_
#define _LE_GIZWITS_PROFILE_H_
#include "system/includes.h"



#define GIZ_BLE_SERVICE_UUID        0xABD0
// 配网模式枚举
typedef enum {
    PROV_MODE_SOUND_WAVE = 0,  // 声波配网
    PROV_MODE_BLE_BEACON,      // BLE广播配网
    PROV_MODE_AP_DIRECT        // AP直连配网
} prov_mode_t;

typedef enum {
    PROV_STATUS_STARTED = 0,
    PROV_STATUS_STOPPED,
    PROV_STATUS_SUCCESS,
    PROV_STATUS_FAILED
} prov_status_t;

// 响应状态码
#define RESP_STATUS_OK       0x00
#define RESP_STATUS_ERROR    0x80

// BLE Function Mask Bit Definitions
#define BLE_VERSION_MASK          0x07    // Bits 0-2
#define BLE_OTA_MASK              0x08    // Bit 3
#define BLE_SECURITY_MASK         0x10    // Bit 4
#define BLE_UNIQUE_KEY_MASK       0x20    // Bit 5
#define BLE_PROVISIONED_MASK      0x40    // Bit 6
#define BLE_RESERVED_MASK         0x80    // Bit 7

// BLE Version Values (Bits 0-2)
#define BLE_VERSION_4_0           0x00
#define BLE_VERSION_4_2           0x01
#define BLE_VERSION_5_0           0x02

// Helper macros for function mask composition
#define BLE_FUNC_SET_VERSION(mask, ver)   (((mask) & ~BLE_VERSION_MASK) | ((ver) & BLE_VERSION_MASK))
#define BLE_FUNC_ENABLE_OTA(mask)         ((mask) | BLE_OTA_MASK)
#define BLE_FUNC_ENABLE_SECURITY(mask)    ((mask) | BLE_SECURITY_MASK)
#define BLE_FUNC_SET_UNIQUE_KEY(mask)     ((mask) | BLE_UNIQUE_KEY_MASK)
#define BLE_FUNC_SET_PROVISIONED(mask)    ((mask) | BLE_PROVISIONED_MASK)

// Version & Type Bit Definitions
#define BLE_VER_TYPE_VERSION_MASK     0x0F    // Bits 0-3 for version
#define BLE_VER_TYPE_DEVICE_MASK      0xF0    // Bits 4-7 for device type

// Version Values (Bits 0-3)
#define BLE_VERSION_5                 0x05    // Version 5

// Device Type Values (Bits 4-7)
#define BLE_DEVICE_TYPE_MESH         (0x08 << 4)  // 0x80: Mesh device
#define BLE_DEVICE_TYPE_BEACON       (0x09 << 4)  // 0x90: Beacon device
#define BLE_DEVICE_TYPE_VOICE        (0x0A << 4)  // 0xA0: Voice device
#define BLE_DEVICE_TYPE_GATT         (0x0B << 4)  // 0xB0: GATT device

// Helper macro for version & type composition
#define BLE_MAKE_VER_TYPE(ver, type) (((ver) & BLE_VER_TYPE_VERSION_MASK) | ((type) & BLE_VER_TYPE_DEVICE_MASK))

// @TODO: 通过对长 PK 进行 CRC32 的运算得来
static const u8 pk[4] = {0x13, 0xF6, 0x2E, 0x11};
static const u8 manu_id[2] = {0x3D, 0x00};

static const uint8_t giz_profile_data[] = {
    // -----------------------------------
    // PRIMARY_SERVICE 1800
    0x0a, 0x00,         // record length - 10 bytes
    0x02, 0x00,         // flags
    0x01, 0x00,         // handle 0x0001
    0x00, 0x28,         // attribute type: PRIMARY_SERVICE (0x2800)
    0x00, 0x18,         // service uuid: 1800

    // CHARACTERISTIC 2a00 READ | DYNAMIC
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x02, 0x00,         // handle 0x0002
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x02,               // properties: READ | WRITE
    0x03, 0x00,         // handle 0x0003
    0x00, 0x2a,         // characteristic uuid: 2a00
    // VALUE 2a00 READ | DYNAMIC
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x03, 0x00,         // handle 0x0003
    0x00, 0x2a,         // characteristic uuid: 2a00

    // CHARACTERISTIC 2a01 READ | DYNAMIC
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x04, 0x00,         // handle 0x0004
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x02,               // properties: READ
    0x05, 0x00,         // handle 0x0005
    0x01, 0x2a,         // characteristic uuid: 2a01
    // VALUE 2a01 READ | DYNAMIC
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x05, 0x00,         // handle 0x0005
    0x01, 0x2a,         // characteristic uuid: 2a01

    // -----------------------------------
    // PRIMARY_SERVICE 1801
    0x0a, 0x00,         // record length - 10 bytes
    0x02, 0x00,         // flags
    0x06, 0x00,         // handle 0x0006
    0x00, 0x28,         // attribute type: PRIMARY_SERVICE (0x2800)
    0x01, 0x18,         // service uuid: 1801

    // CHARACTERISTIC 2a05 INDICATE
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x07, 0x00,         // handle 0x0007
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x20,               // properties: INDICATE
    0x08, 0x00,         // handle 0x0008
    0x05, 0x2a,         // characteristic uuid: 2a05
    // VALUE 2a05 INDICATE
    0x08, 0x00,         // record length - 8 bytes
    0x20, 0x00,         // permissions: INDICATE
    0x08, 0x00,         // handle 0x0008
    0x05, 0x2a,         // characteristic uuid: 2a05
    // CCCD 2a05
    0x0a, 0x00,         // record length - 10 bytes
    0x0a, 0x01,         // permissions: INDICATE | +DYNAMIC
    0x09, 0x00,         // handle 0x0009
    0x02, 0x29,         // attribute type: CLIENT_CHARACTERISTIC_CONFIGURATION (0x2902)
    0x00, 0x00,         // default value: 0x0000

    // CHARACTERISTIC 2b3a READ
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x0a, 0x00,         // handle 0x000a
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x02,               // properties: READ
    0x0b, 0x00,         // handle 0x000b
    0x3a, 0x2b,         // characteristic uuid: 2b3a
    // VALUE 2b3a READ
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x0b, 0x00,         // handle 0x000b
    0x3a, 0x2b,         // characteristic uuid: 2b3a

    // CHARACTERISTIC 2b29 READ | WRITE
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x0c, 0x00,         // handle 0x000c
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x0a,               // properties: READ | WRITE
    0x0d, 0x00,         // handle 0x000d
    0x29, 0x2b,         // characteristic uuid: 2b29
    // VALUE 2b29 READ | WRITE
    0x08, 0x00,         // record length - 8 bytes
    0x0a, 0x01,         // permissions: READ | WRITE | +DYNAMIC
    0x0d, 0x00,         // handle 0x000d
    0x29, 0x2b,         // characteristic uuid: 2b29

    // CHARACTERISTIC 2b2a READ
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x0e, 0x00,         // handle 0x000e
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x02,               // properties: READ
    0x0f, 0x00,         // handle 0x000f
    0x2a, 0x2b,         // characteristic uuid: 2b2a
    // VALUE 2b2a READ
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x0f, 0x00,         // handle 0x000f
    0x2a, 0x2b,         // characteristic uuid: 2b2a

    // -----------------------------------
    // PRIMARY_SERVICE 0xABD0
    0x0a, 0x00,         // record length - 10 bytes
    0x02, 0x00,         // flags
    0x10, 0x00,         // handle 0x0010
    0x00, 0x28,         // attribute type: PRIMARY_SERVICE (0x2800)
    0xD0, 0xAB,         // service uuid: 0xABD0

    // CHARACTERISTIC ABD4 READ
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x11, 0x00,         // handle 0x0011
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x02,               // properties: READ
    0x12, 0x00,         // handle 0x0012
    0xD4, 0xAB,         // characteristic uuid: 0xABD4
    // VALUE ABD4 READ
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x12, 0x00,         // handle 0x0012
    0xD4, 0xAB,         // characteristic uuid: 0xABD4

    // CHARACTERISTIC ABD5 READ | WRITE
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x13, 0x00,         // handle 0x0013
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x0a,               // properties: READ | WRITE
    0x14, 0x00,         // handle 0x0014
    0xD5, 0xAB,         // characteristic uuid: 0xABD5
    // VALUE ABD5 READ | WRITE
    0x08, 0x00,         // record length - 8 bytes
    0x0a, 0x01,         // permissions: READ | WRITE | +DYNAMIC
    0x14, 0x00,         // handle 0x0014
    0xD5, 0xAB,         // characteristic uuid: 0xABD5

    // CHARACTERISTIC ABD6 READ | INDICATE
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x15, 0x00,         // handle 0x0015
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x22,               // properties: READ | INDICATE
    0x16, 0x00,         // handle 0x0016
    0xD6, 0xAB,         // characteristic uuid: 0xABD6
    // VALUE ABD6 READ | INDICATE
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x16, 0x00,         // handle 0x0016
    0xD6, 0xAB,         // characteristic uuid: 0xABD6
    // CCCD ABD6
    0x0a, 0x00,         // record length - 10 bytes
    0x0a, 0x01,         // permissions: READ | WRITE | +DYNAMIC
    0x17, 0x00,         // handle 0x0017
    0x02, 0x29,         // attribute type: CLIENT_CHARACTERISTIC_CONFIGURATION (0x2902)
    0x00, 0x00,         // default value: 0x0000

    // CHARACTERISTIC ABD7 READ | WRITE_NO_RESPONSE
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x18, 0x00,         // handle 0x0018
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x06,               // properties: READ | WRITE_NO_RESPONSE
    0x19, 0x00,         // handle 0x0019
    0xD7, 0xAB,         // characteristic uuid: 0xABD7
    // VALUE ABD7 READ | WRITE_NO_RESPONSE
    0x08, 0x00,         // record length - 8 bytes
    0x06, 0x01,         // permissions: READ | WRITE_NO_RESPONSE | +DYNAMIC
    0x19, 0x00,         // handle 0x0019
    0xD7, 0xAB,         // characteristic uuid: 0xABD7

    // CHARACTERISTIC ABD8 READ | NOTIFY
    0x0d, 0x00,         // record length - 13 bytes
    0x02, 0x00,         // flags
    0x1a, 0x00,         // handle 0x001a
    0x03, 0x28,         // attribute type: CHARACTERISTIC (0x2803)
    0x12,               // properties: READ ｜ NOTIFY
    0x1b, 0x00,         // handle 0x001b
    0xD8, 0xAB,         // characteristic uuid: 0xABD8
    // VALUE ABD8 READ | NOTIFY
    0x08, 0x00,         // record length - 8 bytes
    0x02, 0x01,         // permissions: READ | +DYNAMIC
    0x1b, 0x00,         // handle 0x001b
    0xD8, 0xAB,         // characteristic uuid: 0xABD8
    // CCCD ABD8
    0x0a, 0x00,         // record length - 10 bytes
    0x0a, 0x01,         // permissions: READ | WRITE | +DYNAMIC
    0x1c, 0x00,         // handle 0x001c
    0x02, 0x29,         // attribute type: CLIENT_CHARACTERISTIC_CONFIGURATION (0x2902)
    0x00, 0x00,         // default value: 0x0000

    // END
    0x00, 0x00,
};

#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_2a05_01_CCC_HANDLE   0x0009
#define ATT_CHARACTERISTIC_2b3a_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_ABD4_VALUE_HANDLE    0x0012
#define ATT_CHARACTERISTIC_ABD5_VALUE_HANDLE    0x0014
#define ATT_CHARACTERISTIC_ABD6_VALUE_HANDLE    0x0016
#define ATT_CHARACTERISTIC_ABD6_CCC_HANDLE      0x0017
#define ATT_CHARACTERISTIC_ABD7_VALUE_HANDLE    0x0019
#define ATT_CHARACTERISTIC_ABD8_VALUE_HANDLE    0x001b
#define ATT_CHARACTERISTIC_ABD8_CCC_HANDLE      0x001c

typedef struct {
    char ssid[33];
    char pwd[65];
    char uid[33];
} prov_result_t;

prov_result_t *get_wifi_prov_result(void);

// CMD 指令定义
#define CMD_WIFI_CONFIG         0x40    // WiFi 配置指令
#define CMD_WIFI_CONFIG_RESP    0x41    // WiFi 配置响应指令

// WiFi配置结构体
typedef struct {
    uint32_t ntp;           // 时间戳
    u8 ssid[33];            // 热点名称（最大 32 字节 + 1 字节结束符）
    uint8_t ssid_len;       // 热点名称长度
    u8 bssid[7];            // 热点 BSSID (6 字节 + 1 字节结束符)
    uint8_t bssid_len;      // 热点 BSSID 长度
    u8 password[65];        // 热点密码（最大 64 字节 + 1 字节结束符）
    uint8_t password_len;   // 热点密码长度
    u8 uid[33];             // UID（最大 32 字节 + 1 字节结束符）
    uint8_t uid_len;        // UID 长度
} wifi_config_t;

// 协议解析结果结构体
typedef struct {
    uint8_t cmd;            // 命令字
    uint8_t msg_id;         // 消息 ID
    bool success;           // 解析是否成功
    union {
        wifi_config_t wifi_config;
    } data;
} protocol_data_t;

// WiFi配置响应包结构
typedef struct {
    uint8_t msg_id;             // 消息 ID (0x04)
    uint8_t cmd;                // 命令字 (0x41)
    uint8_t frame_seq;          // 帧序号
    uint8_t frame_len;          // 帧长度
    uint8_t status;             // 状态 (0x00: 成功, 0x80: 失败)
    char hw_ver[8];             // 硬件版本
    char sw_ver[8];             // 软件版本
} wifi_config_response_t;

/**
 * 启动配网
 */
void wifi_prov_start(void);

/**
 * 停止配网
 */
void wifi_prov_stop(void);

/**
 * 收到配网结果，连接网络
 */
void wifi_prov_connect(void);

/**
 * 启动蓝牙配网的 BLE 广播
 */
void giz_ble_init(void);

/**
 * 停止蓝牙配网
 */
void giz_ble_exit(void);

/**
 * 蓝牙配网结果通知
 * 
 * @param event 事件
 */
void ble_cfg_net_result_notify(int event);

/**
 * @brief 构建WiFi配置响应包
 * @param frame_seq 原请求的帧序号
 * @param status 响应状态 (0=成功, 0x80=失败)
 * @param out_buf 输出缓冲区
 * @param buf_size 缓冲区大小
 * @return 响应包长度，失败返回0
 */
size_t pack_wifi_config_response(uint8_t frame_seq, uint8_t msg_id,uint8_t status,
        uint8_t *out_buf, size_t buf_size);

/**
 * @brief 解析协议数据
 * @param data 数据
 * @param len 数据长度
 * @return 解析结果
 */
protocol_data_t protocol_parse_data(const uint8_t *data, size_t len);

/**
 * 设置配网结果
 * 
 * @param ssid 热点名称
 * @param pwd 热点密码
 */
void wifi_prov_set_result(const u8 *ssid, const u8 ssid_len, const u8 *pwd, const u8 pwd_len, const u8 *uid, const u8 uid_len);

#endif