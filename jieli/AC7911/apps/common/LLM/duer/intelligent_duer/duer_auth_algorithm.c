#include "duer_common.h"

#if INTELLIGENT_DUER
#define CST_OFFSET_TIME			(8*60*60)	// 北京时间时差

// 生成随机字节函数
void duer_get_random_bytes(unsigned char *buf, int nbytes)
{
    while (nbytes--) {
        *buf = random32(0);
        ++buf;
    }
}

// 生成UUID字符串函数 (生成32字符的不带连字符UUID)
static void generate_uuid_string_without_hyphens(char *uuid_buffer)
{
    unsigned char rand_bytes[16]; // 16 bytes for UUID
    duer_get_random_bytes(rand_bytes, 16);
    snprintf(uuid_buffer, 33, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             rand_bytes[0], rand_bytes[1], rand_bytes[2], rand_bytes[3],
             rand_bytes[4], rand_bytes[5], rand_bytes[6], rand_bytes[7],
             rand_bytes[8], rand_bytes[9], rand_bytes[10], rand_bytes[11],
             rand_bytes[12], rand_bytes[13], rand_bytes[14], rand_bytes[15]);
}
// 生成对话请求ID函数
void duer_generate_dialog_request_id(char *request_id)
{
    // 获取系统时间
    /* int ntp = ntp_client_get_time("s2c.time.edu.cn"); */
    /* if (ntp == -1) { */
    /* printf(">>>>>error !!!\n"); */
    /* return; */
    /* } */
    /* struct sys_time curtime; */
    /* my_get_sys_time(&curtime); */
    /* MY_LOG_I("Current Time Parameters:\n"); */
    /* MY_LOG_I("Year:  %d\n", curtime.year); */
    /* MY_LOG_I("Month: %d\n", curtime.month); */
    /* MY_LOG_I("Day:   %d\n", curtime.day); */
    /* MY_LOG_I("Hour:  %d\n", curtime.hour); */
    /* MY_LOG_I("Minute:%d\n", curtime.min); */
    /* MY_LOG_I("Second:%d\n", curtime.sec); */
    /* 转换为UTC时间戳（秒） */
    /* int utc_seconds = my_mytime_2_utc_sec(&curtime) - CST_OFFSET_TIME; */
    time_t now_time;
    time(&now_time);

    // 转换为毫秒精度
    /* long long milliseconds = (long long)utc_seconds * 1000; */
    long long milliseconds = (long long)now_time * 1000;

    // 生成UUID（不带连字符）
    char stripped_uuid[33]; // 32字符 + 结束符
    generate_uuid_string_without_hyphens(stripped_uuid);

    // 取UUID的前8个字符作为随机部分
    char random_part[9] = {0};
    strncpy(random_part, stripped_uuid, 8);

    // 组合成最终的请求ID: <毫秒时间戳>_<8字符UUID>
    snprintf(request_id, 60, "%lld_%s", milliseconds, random_part);
}


// 生成6个字符的随机字符串
void duer_generate_6char_random_string(char *output)
{
    char charset[] = "0123456789"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                     "abcdefghijklmnopqrstuvwxyz";
    const int charset_size = sizeof(charset) - 1; // 62个字符

    // 生成6字节随机数据
    unsigned char rand_bytes[6];
    duer_get_random_bytes(rand_bytes, 6);

    // 将每个随机字节映射到字符集
    for (int i = 0; i < 6; i++) {
        output[i] = charset[rand_bytes[i] % charset_size];
    }
    output[6] = '\0'; // 确保以空字符结尾
}


#if 0
// 测试函数
int duer_main()
{
    char dialog_id[60];
    duer_generate_dialog_request_id(dialog_id);
    MY_LOG_I("Generated Dialog Request ID: %s\n", dialog_id);
    return 0;
}


int test_random_string()
{
    char random_str[7]; // 6个字符 + 结束符

    // 生成10个随机字符串进行测试
    for (int i = 0; i < 10; i++) {
        duer_generate_6char_random_string(random_str);
        MY_LOG_I("Random String %d: %s\n", i + 1, random_str);
    }
    return 0;
}
#endif

#endif
