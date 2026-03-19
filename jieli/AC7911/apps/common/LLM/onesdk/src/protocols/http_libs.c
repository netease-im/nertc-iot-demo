//#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "libwebsockets.h"
#include "protocols/private_http_libs.h"
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
// 获取平台信息
const char *get_platform()
{
#ifdef _WIN32
    return "Windows";
#elif __APPLE__
    return "macOS";
#elif __linux__
    return "Linux";
#else
    return "Unknown";
#endif
}

// 获取编译器信息
const char *get_compiler()
{
#ifdef __clang__
    return "Clang";
#elif __GNUC__
    return "GCC";
#elif _MSC_VER
    return "MSVC";
#else
    return "Unknown";
#endif
}
// 获取架构信息
const char *get_architecture()
{
#ifdef __x86_64__
    return "x86_64";
#elif __arm__
    return "arm";
#elif __aarch64__
    return "arm64";
#else
    return "Unknown";
#endif
}

// 获取构建时间
const char *get_build_time()
{
    static char build_time[32];
    time_t now = time(NULL);
    strftime(build_time, sizeof(build_time), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return build_time;
}

// 生成 User-Agent 字符串
void generate_user_agent(char *user_agent, size_t size)
{
    const char *sdk_name = "OneSDK";
    const char *sdk_version = "1.2.3";
    const char *platform = get_platform();
    const char *arch = get_architecture();
    const char *compiler = get_compiler();
    const char *build_time = get_build_time();

    snprintf(user_agent, size, "%s/%s (%s; %s; %s; Build: %s)",
             sdk_name, sdk_version, platform, arch, compiler, build_time);
}


// 解析单个事件块
void parse_sse_message(sse_context_t *ctx, const char *message, size_t len)
{
    char key[128] = {0}, value[2048] = {0};
    const char *line = message;
    const char *end = NULL;

    // 按行解析
    while ((end = strchr(line, '\n')) && (end - line) < len) {
        size_t line_len = end - line;

        // 解析 key:value 格式
        if (line_len > 0) {
            //
            // 更健壮的解析方式
            const char *colon = strchr(line, ':');
            if (colon && colon < line + line_len) {
                // 提取key (去除尾部空格)
                size_t key_len = colon - line;
                while (key_len > 0 && isspace(line[key_len - 1])) {
                    key_len--;
                }
                strncpy(key, line, min(key_len, sizeof(key) - 1));
                key[key_len] = '\0';

                // 提取value (跳过冒号和前导空格)
                const char *val_start = colon + 1;
                while (*val_start && isspace(*val_start)) {
                    val_start++;
                }
                size_t val_len = line + line_len - val_start;
                strncpy(value, val_start, min(val_len, sizeof(value) - 1));
                value[val_len] = '\0';
            }
            //
            if (strcmp(key, "event") == 0) {
//                printf("  Event: %s\n", value);
            } else if (strcmp(key, "data") == 0) {
                // 为data分配新内存并拷贝内容
                if (ctx->data) {
                    free(ctx->data);
                    ctx->data = NULL;
                    ctx->data_len = 0;
                }
                ctx->data = malloc(strlen(value) + 1);
                if (ctx->data) {
                    strcpy(ctx->data, value);
                    ctx->data_len = strlen(value);
                }
                // printf("  Data: %s\n", value);
            } else if (strcmp(key, "id") == 0) {
//                printf("  ID: %s\n", value);
            }
        }

        // 跳过当前行
        line = end + 1;

        // 碰到空行，认为这是一个完整事件，请求返回
        if (line[0] == '\0' || (line - message) >= len) {
            break;
        }
    }
    // printf("\n");
}

/*
 * @brief 从字符串中查找子串(实现平台无关的memmem/strnstr)
 * 说明：Mac平台用来替换strnstr, windows/Linux平台用来替换memmem
 * @param haystack 待查找的字符串
 * @param haystacklen 待查找的字符串长度
 * @param needle 要查找的子串
 * @param needlelen 要查找的子串长度
 * @return 找到子串时返回子串的指针，否则返回NULL
 */
void *onesdk_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen)
{
    if (haystack == NULL || needle == NULL || needlelen == 0 || haystacklen < needlelen) {
        return NULL;
    }

    const char *hay = (const char *)haystack;
    const char *ndl = (const char *)needle;

    for (size_t i = 0; i <= haystacklen - needlelen; i++) {
        if (memcmp(&hay[i], ndl, needlelen) == 0) {
            return (void *)(&hay[i]);
        }
    }

    return NULL;
}
