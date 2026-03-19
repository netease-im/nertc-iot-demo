#ifndef  PRIVATE_HTTP_LIBS_H
#define  PRIVATE_HTTP_LIBS_H
// 每个 SSE 块的用户数据结构
struct sse_data {
    char id[128];        // 事件 ID
    char event[128];     // 事件类型
    char data[1024];     // 数据部分
};

// 全局缓冲区保存部分消息流，解析时用
#define SSE_BUFFER_SIZE 4096
typedef struct sse_context {
    char buffer[SSE_BUFFER_SIZE]; // 临时缓冲区
    size_t used_len;          // 当前缓冲区使用长度
    char *data; // point to the buffer
    size_t data_len;
    char *event; // not yet supported
    size_t event_len;
    char *id; // not yet supported
    size_t id_len;
} sse_context_t;

void parse_sse_message(sse_context_t *ctx, const char *message, size_t len);
void generate_user_agent(char *user_agent, size_t size);
void *onesdk_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);

#define HEADER_AIGW_HARDWARE_ID "X-Hardware-Id"

#endif // PRIVATE_HTTP_LIBS_H
