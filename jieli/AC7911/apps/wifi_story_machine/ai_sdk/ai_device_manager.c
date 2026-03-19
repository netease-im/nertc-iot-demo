#include "app_config.h"

#include "system/includes.h"
#include "asm/gpio.h"
#include "cJSON_common/cJSON.h"
#include "syscfg_id.h"
#include "ai_device_manager.h"


u16 timer_device_status_id = 0;
device_status_t device_status = {0};
device_param_t device_param = {0};
device_mqtt_config_t device_mqtt_config = {0};
coze_login_t coze_login = {0};


void print_sys_status(void *priv){
    malloc_stats();
    printf("is_in_config_network_state: %d\n", is_in_config_network_state());
    printf("device_status.chat_start %d device_status.chat_end: %d\n", device_status.chat_start, device_status.chat_end);
    printf("get_decoder_len(): %d\n", get_decoder_len());
    printf("device_status.dac_play_en: %d, device_status.mic_open_en: %d", device_status.dac_play_en, device_status.mic_open_en);
}
void sys_info_timer_int(void){
    // void init_opus_cache_t(void);
    // init_opus_cache_t();
    timer_device_status_id = usr_timer_add(NULL, print_sys_status, 1000, 1);
}
// static void session_app_main_task(void *priv)
// void app_task_init(void)
// {
//     // sys_info_timer_int();
//     if (thread_fork("session_app_main_task", 12, 2048, 0, NULL, session_app_main_task, NULL) != OS_NO_ERR) {
//         printf("session_app_main_task thread fork fail\n");
//     }
// }


//luy
unsigned char hex_char_to_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;  // 非法字符处理
}

//mac地址字符串转hex数据
void convert_macstr2machex(const char *mac_str, u8 *mac_hex)
{
    for (int i = 0; i < 6; i++) {
        mac_hex[i] = (hex_char_to_value(mac_str[2 * i]) << 4) | hex_char_to_value(mac_str[2 * i + 1]);
    }
}

// 获取第 N 个逗号之间的字符
char* getFieldByComma(char *str, int N, char *result) {
    // 创建一个字符串副本
    char str_copy[DEVICE_TYPE_MAX_LEN * 5];
    strncpy(str_copy, str, sizeof(str_copy) - 1);
    str_copy[sizeof(str_copy) - 1] = '\0';  // 确保字符串以 '\0' 结尾

    char *token;
    int count = 1;  // 逗号计数，从1开始
    token = strtok(str_copy, ",");  // 使用逗号分割字符串

    while (token != NULL) {
        if (count == N) {
            // 找到第 N 个逗号之间的字段，复制到 result 中
            size_t len = strlen(token);  // 获取 token 的长度
            size_t copy_len = (len < DEVICE_TYPE_MAX_LEN - 1) ? len : (DEVICE_TYPE_MAX_LEN - 1);  // 确保不溢出
            strncpy(result, token, copy_len);
            result[copy_len] = '\0';  // 确保字符串结尾为 '\0'
            return result;
        }
        count++;
        token = strtok(NULL, ",");  // 获取下一个字段
    }

    // 如果没有找到第 N 个字段，返回 NULL
    return NULL;
}


void device_param_factory(void)
{
    memcpy(device_param.device_mac, DEVICE_MAC, strlen(DEVICE_MAC));
    device_param_save(CFG_USER_DEVICE_MAC);

    memcpy(device_param.device_id, DEVICE_ID, strlen(DEVICE_ID));
    device_param_save(CFG_USER_DEVICE_ID);

    memcpy(device_param.szAuthKey, AUTH_KEY, strlen(AUTH_KEY));
    device_param_save(CFG_USER_AUTH_KEY);

    memcpy(device_param.szPK, PRODUCT_KEY, strlen(PRODUCT_KEY));
    device_param_save(CFG_USER_PRODUCT_KEY);

    memcpy(device_param.secret, DEVICE_SECRET, strlen(DEVICE_SECRET));
    device_param_save(CFG_USER_DEVICE_SECRET);

}

//luy
// u8 product_read_license(u8 *idx, u8 *buf, u32 *len)
// {
//     u8 *data;
//     u32 ret = 0, flag;

//     *len = 0;

//     if (!(data = zalloc(PRODUCT_LICENSE_INFO_SIZE))) {
//         return ERR_MALLOC_FAIL;
//     }

//     if ((ret = syscfg_read(CFG_PRODUCT_LICENSE_INDEX, data, PRODUCT_LICENSE_INFO_SIZE))) {
//         memcpy(&flag, data, sizeof(flag));
//         product_info("%s, license_len = %d\n", __FUNCTION__, flag);
//         if (flag) {
//             *idx = 0;
//             *len = flag;
//             memcpy(buf, data + sizeof(flag), *len);
//             free(data);
//             return ERR_NULL;
//         }
//     }

//     free(data);
//     return ERR_FILE_WR;
// }

void device_param_init(void){
    int ret = 0;
    memset(&device_param, 0, sizeof(device_param_t));

    int get_auth_code(char *buf, int *len);
    char license_buf[128] = {0};
    int len = 0;
    // get_auth_code(test_buf, &len);
    u8 idx = 0;
    //u8 product_read_license(u8 *idx, u8 *buf, u32 *len);
    //product_read_license(&idx, license_buf, &len);
    printf("idx: %d, len: %d, test_buf: %s\n", idx, len, license_buf);

    char buf[128] = {0};
    getFieldByComma(license_buf, 1, buf);
    printf("buf1: %s\n", buf);
    getFieldByComma(license_buf, 2, buf);
    printf("buf2: %s\n", buf);
    getFieldByComma(license_buf, 3, buf);
    printf("buf3: %s\n", buf);
    getFieldByComma(license_buf, 4, buf);
    printf("buf4: %s\n", buf);
    getFieldByComma(license_buf, 5, buf);
    printf("buf5: %s\n", buf);
    if(license_buf){
        printf("get license_buf mac\n");
        memset(buf, sizeof(buf), 0);
        getFieldByComma(license_buf, 1, buf);

    }

    //mac地址
    ret = syscfg_read(CFG_USER_DEVICE_MAC, device_param.device_mac, DEVICE_TYPE_MAX_LEN);
    // printf("");
    put_buf(buf, 6);
    put_buf(device_param.device_mac, 6);
    if (ret <= 0 || memcmp(device_param.device_mac, buf, 12) != 0) {
        printf("license_buf buf1: %s\n", buf);
        if(strlen(buf)){
            memcpy(device_param.device_mac, buf, strlen(buf));
        }else{
            memcpy(device_param.device_mac, DEVICE_MAC, strlen(DEVICE_MAC));
        }

        device_param_save(CFG_USER_DEVICE_MAC);
    }
    //设备id
    ret = syscfg_read(CFG_USER_DEVICE_ID, device_param.device_id, DEVICE_TYPE_MAX_LEN);
    memset(buf, sizeof(buf), 0);
    getFieldByComma(license_buf, 2, buf);
    if (ret <= 0 || memcmp(device_param.device_id, buf, 8) != 0) {

        printf("buf2: %s\n", buf);
        if(strlen(buf)){
            memcpy(device_param.device_id, buf, strlen(buf));
        }else{
            memcpy(device_param.device_id, DEVICE_ID, strlen(DEVICE_ID));
        }
        device_param_save(CFG_USER_DEVICE_ID);
    }
    //认证码
    ret = syscfg_read(CFG_USER_AUTH_KEY, device_param.szAuthKey, DEVICE_TYPE_MAX_LEN);
    memset(buf, sizeof(buf), 0);
    getFieldByComma(license_buf, 3, buf);
    if (ret <= 0 || memcmp(device_param.szAuthKey, buf, 32) != 0) {
        printf("buf2: %s\n", buf);
        if(strlen(buf)){
            memcpy(device_param.szAuthKey, buf, strlen(buf));
        }else{
            memcpy(device_param.szAuthKey, AUTH_KEY, strlen(AUTH_KEY));
        }
        device_param_save(CFG_USER_AUTH_KEY);
    }
    //厂商数据
    ret = syscfg_read(CFG_USER_PRODUCT_KEY, device_param.szPK, DEVICE_TYPE_MAX_LEN);
    memset(buf, sizeof(buf), 0);
    getFieldByComma(license_buf, 4, buf);
    if (ret <= 0  || memcmp(device_param.szPK, buf, 32) != 0) {

        printf("buf4: %s\n", buf);
        if(strlen(buf)){
            memcpy(device_param.szPK, buf, strlen(buf));
        }else{
            memcpy(device_param.szPK, PRODUCT_KEY, strlen(PRODUCT_KEY));
        }
        device_param_save(CFG_USER_PRODUCT_KEY);
    }
    //秘钥
    ret = syscfg_read(CFG_USER_DEVICE_SECRET, device_param.secret, DEVICE_TYPE_MAX_LEN);
    memset(buf, sizeof(buf), 0);
    getFieldByComma(license_buf, 5, buf);
    if (ret <= 0 || memcmp(device_param.secret, buf, 32) != 0) {

        printf("buf5: %s\n", buf);
        if(strlen(buf)){
            memcpy(device_param.secret, buf, strlen(buf));
        }else{
            memcpy(device_param.secret, DEVICE_SECRET, strlen(DEVICE_SECRET));
        }
        device_param_save(CFG_USER_DEVICE_SECRET);
    }
    //用户参数
    ret = syscfg_read(CFG_USER_DEVICE_INFO, &device_param.user_info, sizeof(user_param_t));
    if (ret <= 0) {
        device_param.user_info.is_reset = 1;
        device_param_save(CFG_USER_DEVICE_INFO);
    }

    if(ret > 0){
        printf("\n\n***********************device_param*************************** %d\n\n", ret);
        printf("device_param.szPK: %s\n", device_param.szPK);
        printf("device_param.device_mac: %s", device_param.device_mac);
        printf("device_param.szAuthKey: %s\n", device_param.szAuthKey);
        printf("device_param.device_id: %s", device_param.device_id);
        printf("device_param.secret: %s", device_param.secret);
        printf("device_param.user_info.is_reset: %d", device_param.user_info.is_reset);

        printf("\n\n*************************************************************\n\n");
    }

}

void device_param_save(u8 type){
    u32 len = 0;
    void *param;
    switch(type){
        case CFG_USER_PRODUCT_KEY:
            param = device_param.szPK;
            len = DEVICE_TYPE_MAX_LEN;
            break;
        case CFG_USER_DEVICE_MAC:
            param = device_param.device_mac;
            len = DEVICE_TYPE_MAX_LEN;
        break;
        case CFG_USER_AUTH_KEY:
            param = device_param.szAuthKey;
            len = DEVICE_TYPE_MAX_LEN;
            break;
        case CFG_USER_DEVICE_ID:
            param = device_param.device_id;
            len = DEVICE_TYPE_MAX_LEN;
            break;
        case CFG_USER_DEVICE_SECRET:
            param = device_param.secret;
            len = DEVICE_TYPE_MAX_LEN;
            break;
        case CFG_USER_DEVICE_INFO:
            param = &device_param.user_info;
            len = sizeof(user_param_t);
            break;

        default:
            break;

    }
    if(len > 0){
        int ret = syscfg_write(type, param, len);
        if (ret > 0) {
            printf("device_param_ %d save ok, len %d\n", type, ret);
        }else{
            printf("device_param_save %d fail %d\n", type, ret);
        }
        if(type == CFG_USER_DEVICE_MAC){
            static u8 mac_addr[6];
            if (strlen(device_param.device_mac) != 12) {
                u8 flash_uid[16];
                memcpy(flash_uid, get_norflash_uuid(), 16);
                do {
                    u32 crc32 = rand32()^CRC32(flash_uid, sizeof(flash_uid));
                    u16 crc16 = rand32()^CRC16(flash_uid, sizeof(flash_uid));
                    memcpy(mac_addr, &crc32, sizeof(crc32));
                    memcpy(&mac_addr[4], &crc16, sizeof(crc16));
                } while (!bytecmp(mac_addr, 0, 6));
                //此处用户可自行修改为本地生成mac地址的算法
                mac_addr[0] &= ~((1 << 0) | (1 << 1));
                printf("device_param first mac addr use sys random\n");

            }else{
                printf("device_param first mac addr use sn : %s", device_param.device_mac);
                void convert_macstr2machex(const char *mac_str, u8 *mac_hex);
                convert_macstr2machex(device_param.device_mac, mac_addr);
            }
            put_buf(mac_addr, 6);
            int ret2 = syscfg_write(CFG_BT_MAC_ADDR, mac_addr, 6);
            if (ret2 > 0) {
                printf("CFG_BT_MAC_ADDR %d save ok, len %d\n", type, ret2);
            }else{
                printf("CFG_BT_MAC_ADDR %d fail %d\n", type, ret2);
            }
        }

    }else{
        printf("no need update len is %d", len);
    }


    printf("\n\n***********************device_param***************************\n\n");
    printf("device_param.szPK: %s\n", device_param.szPK);
    printf("device_param.device_mac: %s", device_param.device_mac);
    printf("device_param.szAuthKey: %s\n", device_param.szAuthKey);
    printf("device_param.device_id: %s", device_param.device_id);
    printf("device_param.secret: %s", device_param.secret);
    printf("device_param.is_reset: %d", device_param.user_info.is_reset);

    printf("\n\n*************************************************************\n\n");
}





