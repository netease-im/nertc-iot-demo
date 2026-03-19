#include "system/includes.h"
#include "app_power_manage.h"
#include "asm/rf_coexistence_config.h"

#include "app_config.h"
#include "wifi_prov.h"
#include "le_gizwits_profile.h"
#include "ai_device_manager.h"

#define LOG_TAG_CONST   WIFI_PROV
#define LOG_TAG_VAR     "[WIFI_PROV]"
#define log_info          printf
#define log_debug          printf
#define log_error          printf

typedef struct {
    struct {
        uint8_t msg_id: 5;      // Bit 0-4: Msg ID (1-31)
        uint8_t ver: 2;         // Bit 5-6: Version (0-3)
        uint8_t reserved: 1;    // Bit 7: Reserved (0)
    } byte0;
    uint8_t cmd;                // 指令字
    struct {
        uint8_t seq: 4;         // Bit 0-3: 帧序号 (0-15)
        uint8_t frames: 4;      // Bit 4-7: 总拆包帧数 (实际帧数=值+1)
    } byte2;
    uint8_t frame_len;          // 帧数据长度
} protocol_header_t;

static prov_status_t wifi_prov_status = PROV_STATUS_STOPPED;

static prov_result_t prov_result = {0};

prov_result_t *get_wifi_prov_result(void)
{
    return &prov_result;
}

BOOL is_wifi_prov_running(void)
{
    return wifi_prov_status == PROV_STATUS_STARTED;
}

void wifi_prov_start(void)
{
    log_info("wifi_prov_start");

    wifi_prov_status = PROV_STATUS_STARTED;

#if TCFG_USER_BLE_ENABLE && BT_NET_CFG_EN
    memset(&prov_result, 0, sizeof(prov_result_t));
#if !CONFIG_POWER_ON_ENABLE_BLE
    giz_ble_init();
#endif
#endif
}

void wifi_prov_connect(void)
{
    log_info("wifi_prov_connect");

    wifi_sta_connect(prov_result.ssid, prov_result.pwd, 1);
}

void wifi_prov_stop(void)
{
    log_info("wifi_prov_stop");

    giz_ble_exit();

    wifi_prov_status = PROV_STATUS_STOPPED;
}

static void parse_protocol_header(const uint8_t *data, protocol_header_t *header)
{
    // 解析第一个字节
    header->byte0.msg_id = data[0] & 0x1F;          // 取低5位
    header->byte0.ver = (data[0] >> 5) & 0x03;      // 取第5-6位
    header->byte0.reserved = (data[0] >> 7) & 0x01; // 取第7位

    // 解析第二个字节 (CMD)
    header->cmd = data[1];

    // 解析第三个字节
    header->byte2.seq = data[2] & 0x0F;             // 取低4位
    header->byte2.frames = (data[2] >> 4) & 0x0F;   // 取高4位

    // 解析第四个字节 (帧长度)
    header->frame_len = data[3];

    log_debug("Protocol Header:\n\tmsg_id: %d\n\tver: %d\n\treserved: %d\n\tcmd: %d\n\tseq: %d\n\tframes: %d\n\tframe_len: %d",
        header->byte0.msg_id, header->byte0.ver, header->byte0.reserved, header->cmd,
        header->byte2.seq, header->byte2.frames, header->frame_len);
}

static bool parse_field(const uint8_t *data, size_t data_len, size_t *offset,
        uint8_t *out_len, void *out_data, size_t max_len, const char *field_name)
{
    // 检查长度字段
    if (*offset + 1 > data_len) {
        log_error("Error: Not enough data for %s length. Required: %zu, Available: %zu",
               field_name, 1, data_len - *offset);
        return false;
    }

    // 获取字段长度
    *out_len = data[(*offset)++];
    log_debug("%s Length: %d", field_name, *out_len);

    // 检查字段数据
    if (*out_len > 0) {
        if (*offset + *out_len > data_len || *out_len > max_len) {
            log_error("Error: %s length exceeds limits. Length: %d, Available: %zu, Max: %zu",
                   field_name, *out_len, data_len - *offset, max_len);
            return false;
        }
        memcpy(out_data, &data[*offset], *out_len);
        if (max_len > *out_len) {  // 如果是字符串，添加结束符
            ((uint8_t*)out_data)[*out_len] = '\0';
        }
        *offset += *out_len;

        // 打印解析结果
        if (field_name[0] != '_') {  // 如果字段名不以_开头就打印
            log_debug("Parsed %s: %s", field_name, (char*)out_data);
        }
    }

    return true;
}

static bool parse_wifi_config(const uint8_t *data, size_t len, wifi_config_t *wifi_config)
{
    size_t offset = 4;

    // 1. 解析NTP时间戳
    if (offset + 4 > len) {
        log_error("Error: Not enough data for NTP timestamp");
        return false;
    }
    wifi_config->ntp = (data[offset] << 24) | (data[offset+1] << 16) | (data[offset+2] << 8) | data[offset+3];
    offset += 4;
    log_debug("Parsed NTP: %u", wifi_config->ntp);

    // 2. 解析SSID
    if (!parse_field(data, len, &offset, &wifi_config->ssid_len,
            wifi_config->ssid, sizeof(wifi_config->ssid) - 1, "SSID")) {
        return false;
    }

    // 3. 解析BSSID
    if (!parse_field(data, len, &offset, &wifi_config->bssid_len,
            wifi_config->bssid, sizeof(wifi_config->bssid) - 1, "BSSID")) {
        return false;
    }

    // 4. 解析密码
    if (!parse_field(data, len, &offset, &wifi_config->password_len,
            wifi_config->password, sizeof(wifi_config->password) - 1, "Password")) {
        return false;
    }

    // 5. 解析UID
    if (!parse_field(data, len, &offset, &wifi_config->uid_len,
            wifi_config->uid, sizeof(wifi_config->uid) - 1, "UID")) {
        return false;
    }

    // 打印最终配置摘要
    log_info("WiFi Configuration Summary:\n\tNTP: %u\n\tSSID (%d): %s\n\tBSSID (%d): %s\n\tPassword (%d): %s\n\tUID (%d): %s",
        wifi_config->ntp,
        wifi_config->ssid_len, wifi_config->ssid,
        wifi_config->bssid_len, (wifi_config->bssid_len > 0) ? wifi_config->bssid : "N/A",
        wifi_config->password_len, wifi_config->password,
        wifi_config->uid_len, (wifi_config->uid_len > 0) ? wifi_config->uid : "N/A");

    return true;
}

void wifi_prov_set_result(const u8 *ssid, const u8 ssid_len, const u8 *pwd, const u8 pwd_len, const u8 *uid, const u8 uid_len)
{
    memcpy(prov_result.ssid, ssid, ssid_len);
    memcpy(prov_result.pwd, pwd, pwd_len);
    memcpy(prov_result.uid, uid, uid_len);
    printf("wifi_prov_set_result: ssid:%s, pwd:%s, uid:%s\n");
}

size_t pack_wifi_config_response(uint8_t frame_seq, uint8_t msg_id,uint8_t status,
        uint8_t *out_buf, size_t buf_size)
{
    if (!out_buf || buf_size < sizeof(wifi_config_response_t)) {
        log_error("invalid parameter");
        return 0;
    }

    wifi_config_response_t *resp = (wifi_config_response_t *)out_buf;

    resp->msg_id = msg_id;
    resp->cmd = CMD_WIFI_CONFIG_RESP;
    resp->frame_seq = frame_seq;
    resp->frame_len = sizeof(wifi_config_response_t) - 4;
    resp->status = status;

    strncpy(resp->hw_ver, HARD_VERSION, sizeof(resp->hw_ver) - 1);
    strncpy(resp->sw_ver, SOFT_VERSION, sizeof(resp->sw_ver) - 1);

    return sizeof(wifi_config_response_t);
}

protocol_data_t protocol_parse_data(const uint8_t *data, size_t len)
{
    protocol_data_t result = {0};
    result.success = false;

    if (len < 4) {
        log_error("invalid data length: %d", len);
        return result;
    }

    // 解析协议头
    protocol_header_t header;
    parse_protocol_header(data, &header);

    result.cmd = header.cmd;
    result.msg_id = header.byte0.msg_id;

    // 根据CMD选择不同的解析方法
    switch (header.cmd) {
    case CMD_WIFI_CONFIG:
        result.success = parse_wifi_config(data, len, &result.data.wifi_config);
        break;
    default:
        // 如果有数据部分，打印原始数据
        if (len > 4 && header.frame_len > 0) {
            log_info("Unrecognized CMD, payload data: ");
            log_info_hexdump(data + 4, header.frame_len);
        }
        break;
    }

    return result;
}
