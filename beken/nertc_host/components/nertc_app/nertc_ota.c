/**
 * @file    nertc_ota.c
 * @brief   OTA 版本检查 — 轻量 HTTP POST via lwip sockets
 *
 * 同构旧 nertc_ota.c, 发送设备信息到 NERTC OTA 端点,
 * 解析 JSON 响应获取 MQTT broker 配置 (用于 lite_mode)。
 */

/* lwip sockets — 必须最先引入, 在系统 errno.h 之前 */
#include "nertc_lwip_compat.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "nertc_ota.h"
#include "nertc_config.h"

/* WiFi MAC */
#include "modules/wifi_types.h"
#include "modules/wifi.h"

/* cJSON */
#include "cJSON.h"

/* BK logging */
#include <common/bk_err.h>
#include <os/os.h>

#define TAG "nertc_ota"
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)

/* OTA 端点 */
#define OTA_PATH         "/v1/ota"
#define OTA_TIMEOUT_MS   15000
#define OTA_RESP_BUF_SIZE 4096

/* ------------------------------------------------------------------ */
/*  HTTP 请求构建                                                      */
/* ------------------------------------------------------------------ */

static char *build_http_request(const char *host, const char *path,
                                 const char *app_key,
                                 const char **extra_headers, int extra_header_count,
                                 const char *body, int body_len,
                                 int *req_len_out)
{
    int max_size = 2048 + body_len;
    char *req = (char *)malloc(max_size);
    if (!req) return NULL;

    int pos = 0;

    if (app_key && app_key[0]) {
        pos += snprintf(req + pos, max_size - pos,
                        "POST %s?appkey=%s HTTP/1.1\r\n", path, app_key);
    } else {
        pos += snprintf(req + pos, max_size - pos,
                        "POST %s HTTP/1.1\r\n", path);
    }

    pos += snprintf(req + pos, max_size - pos, "Host: %s\r\n", host);

    for (int i = 0; i < extra_header_count; i += 2) {
        if (extra_headers[i] && extra_headers[i + 1]) {
            pos += snprintf(req + pos, max_size - pos,
                           "%s: %s\r\n", extra_headers[i], extra_headers[i + 1]);
        }
    }

    pos += snprintf(req + pos, max_size - pos,
                    "Content-Length: %d\r\n", body_len);
    pos += snprintf(req + pos, max_size - pos, "\r\n");

    if (body && body_len > 0) {
        memcpy(req + pos, body, body_len);
        pos += body_len;
    }

    *req_len_out = pos;
    return req;
}

/* ------------------------------------------------------------------ */
/*  HTTP POST                                                          */
/* ------------------------------------------------------------------ */

static int http_post_simple(const char *host, int port, const char *path,
                             const char *app_key,
                             const char **headers, int header_count,
                             const char *body,
                             char *resp_buf, int resp_buf_len)
{
    int sock = -1;
    int status = -1;

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char service[8];
    snprintf(service, sizeof(service), "%u", port);

    int rc = getaddrinfo(host, service, &hints, &res);
    if (rc != 0 || !res) {
        LOGE("DNS resolve failed: %s", host);
        return -1;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        LOGE("socket() failed, errno=%d", errno);
        freeaddrinfo(res);
        return -1;
    }

    struct timeval tv = { .tv_sec = OTA_TIMEOUT_MS / 1000,
                          .tv_usec = (OTA_TIMEOUT_MS % 1000) * 1000 };
    lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    lwip_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    rc = lwip_connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (rc < 0) {
        LOGE("connect() failed, errno=%d", errno);
        goto cleanup;
    }

    LOGI("Connected to %s:%d", host, port);

    int req_len = 0;
    char *req = build_http_request(host, path, app_key,
                                    headers, header_count,
                                    body, (int)strlen(body), &req_len);
    if (!req) {
        LOGE("build_http_request failed");
        goto cleanup;
    }

    int sent = 0;
    while (sent < req_len) {
        rc = lwip_send(sock, req + sent, req_len - sent, 0);
        if (rc < 0) {
            free(req);
            goto cleanup;
        }
        sent += rc;
    }
    free(req);

    /* 读取响应 */
    int total = 0;
    int header_done = 0;
    char *body_start = NULL;
    int content_length = -1;

    while (total < resp_buf_len - 1) {
        rc = lwip_recv(sock, resp_buf + total, resp_buf_len - 1 - total, 0);
        if (rc <= 0) break;
        total += rc;
        resp_buf[total] = '\0';

        if (!header_done) {
            body_start = strstr(resp_buf, "\r\n\r\n");
            if (body_start) {
                header_done = 1;
                *body_start = '\0';
                body_start += 4;

                char *space = strchr(resp_buf, ' ');
                if (space) {
                    status = (int)strtol(space + 1, NULL, 10);
                }

                char *cl = strstr(resp_buf, "Content-Length:");
                if (!cl) cl = strstr(resp_buf, "content-length:");
                if (cl) {
                    content_length = (int)strtol(cl + 15, NULL, 10);
                }

                int body_already = (int)((resp_buf + total) - body_start);
                if (body_start != resp_buf) {
                    memmove(resp_buf, body_start, body_already);
                }
                total = body_already;
                resp_buf[total] = '\0';
            }
        }

        if (header_done && content_length > 0 && total >= content_length) {
            break;
        }
    }

    if (total < resp_buf_len) {
        resp_buf[total] = '\0';
    } else {
        resp_buf[resp_buf_len - 1] = '\0';
    }

cleanup:
    if (sock >= 0) lwip_close(sock);
    return status;
}

/* ------------------------------------------------------------------ */
/*  JSON 构建 & 解析                                                 */
/* ------------------------------------------------------------------ */

static char *build_system_info_json(const char *mac_address, int *len_out)
{
    char buf[1024];
    int len = snprintf(buf, sizeof(buf),
        "{"
          "\"version\":2,"
          "\"language\":\"zh-CN\","
          "\"flash_size\":0,"
          "\"minimum_free_heap_size\":\"0\","
          "\"mac_address\":\"%s\","
          "\"uuid\":\"\","
          "\"chip_model_name\":\"bk7258\","
          "\"chip_info\":{\"model\":0,\"cores\":1,\"revision\":0,\"features\":0},"
          "\"application\":{"
            "\"capabilities\":{\"netease_cloud_music\":{\"support_play\":false}},"
            "\"name\":\"bk7258\","
            "\"version\":\"1.0.3\","
            "\"board_name\":\"bk7258\","
            "\"compile_time\":\"\","
            "\"idf_version\":\"\","
            "\"elf_sha256\":\"\""
          "},"
          "\"partition_table\":[],"
          "\"ota\":{\"label\":\"\"},"
          "\"board\":{}"
        "}",
        mac_address);

    if (len < 0 || len >= (int)sizeof(buf)) return NULL;

    char *result = (char *)malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, buf, len + 1);
    *len_out = len;
    return result;
}

static void json_get_str(cJSON *obj, const char *key, char *out, int out_len)
{
    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (cJSON_IsString(item) && item->valuestring) {
        strncpy(out, item->valuestring, out_len - 1);
        out[out_len - 1] = '\0';
    }
}

/* ------------------------------------------------------------------ */
/*  公共 API                                                          */
/* ------------------------------------------------------------------ */

int nertc_ota_check_version(const char *app_key, const char *device_id,
                             nertc_ota_result_t *result)
{
    if (!device_id || !result) return BK_FAIL;

    memset(result, 0, sizeof(*result));

    char mac_str[18] = {0};
    uint8_t mac[6];
    if (bk_wifi_sta_get_mac(mac) == BK_OK) {
        snprintf(mac_str, sizeof(mac_str),
                 "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    const char *mac_for_body = mac_str[0] ? mac_str : device_id;

    LOGI("OTA check: app_key=%s, device_id=%s",
         app_key ? app_key : "", device_id);

    int body_len = 0;
    char *body = build_system_info_json(mac_for_body, &body_len);
    if (!body) {
        LOGE("Failed to build system info JSON");
        return BK_FAIL;
    }

    const char *headers[] = {
        "Device-Id",          device_id,
        "Content-Type",       "application/json",
        "User-Agent",         "bk7258/1.0.3",
        "Activation-Version", "1",
        "Accept-Language",    "zh-CN",
    };
    int header_count = sizeof(headers) / sizeof(headers[0]);

    char *resp_buf = (char *)malloc(OTA_RESP_BUF_SIZE);
    if (!resp_buf) {
        free(body);
        return BK_FAIL;
    }

    BK_LOGW("OTA", "ota request host:%s port:%d path:%s body:%s", OTA_HOST, OTA_PORT, OTA_PATH, body);
    int status = http_post_simple(OTA_HOST, OTA_PORT, OTA_PATH,
                                   app_key, headers, header_count,
                                   body, resp_buf, OTA_RESP_BUF_SIZE);
    free(body);
    BK_LOGW("OTA", "ota response resp_buf:%s", resp_buf);
    if (status != 200) {
        LOGE("OTA HTTP failed, status=%d", status);
        free(resp_buf);
        return BK_FAIL;
    }

    cJSON *root = cJSON_Parse(resp_buf);
    if (!root) {
        LOGE("Failed to parse OTA JSON");
        free(resp_buf);
        return BK_FAIL;
    }

    /* MQTT 配置 */
    cJSON *mqtt = cJSON_GetObjectItem(root, "mqtt");
    if (cJSON_IsObject(mqtt)) {
        json_get_str(mqtt, "endpoint", result->mqtt_endpoint,
                     sizeof(result->mqtt_endpoint));
        json_get_str(mqtt, "client_id", result->mqtt_client_id,
                     sizeof(result->mqtt_client_id));
        json_get_str(mqtt, "username", result->mqtt_username,
                     sizeof(result->mqtt_username));
        json_get_str(mqtt, "password", result->mqtt_password,
                     sizeof(result->mqtt_password));
        json_get_str(mqtt, "publish_topic", result->mqtt_publish_topic,
                     sizeof(result->mqtt_publish_topic));

        result->valid = (result->mqtt_endpoint[0] != '\0');
        LOGI("MQTT config: endpoint=%s", result->mqtt_endpoint);
    }

    /* device_sdk_config (可选) */
    cJSON *agent = cJSON_GetObjectItem(root, "agent");
    if (cJSON_IsObject(agent)) {
        cJSON *sdk_cfg = cJSON_GetObjectItem(agent, "device_sdk_config");
        if (cJSON_IsObject(sdk_cfg)) {
            char *cfg_str = cJSON_PrintUnformatted(sdk_cfg);
            if (cfg_str) {
                strncpy(result->device_sdk_config, cfg_str,
                        sizeof(result->device_sdk_config) - 1);
                result->has_device_sdk_config = true;
                cJSON_free(cfg_str);
            }
        }
    }

    cJSON_Delete(root);
    free(resp_buf);

    LOGI("OTA check complete, mqtt_valid=%d", result->valid);
    return result->valid ? BK_OK : BK_FAIL;
}
