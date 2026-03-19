#ifndef PRIVATE_LWS_HTTP_DOWNLOAD_H
#define PRIVATE_LWS_HTTP_DOWNLOAD_H

// #include "http.h"
#include "asm/cpu.h"
typedef enum {
    HTTP_DOWNLOAD_METHOD_ONCE = 0, // 一次下载
    HTTP_DOWNLOAD_METHOD_RANGE = 1, // 分段下载
} HTTP_DOWNLOAD_METHOD;

typedef void(http_download_data_cb)(const char *in, size_t in_len, bool is_last_chunk, void *user_data);
typedef void(http_download_complete_cb)(void *user_data);
typedef void(http_download_error_cb)(int err_code, void *user_data);

typedef struct http_download_context {
    void *parent_http_ctx;
    HTTP_DOWNLOAD_METHOD method;
    char *file_path; // 下载文件路径
    void *file_fd; // 文件句柄
    int timeout_ms; // 下载超时时间
    // 下载专用字段
    size_t total_size; // total size of the file如没有回自动获取
    // 单次下载
    size_t downloaded_size; // offset which has been downloaded
    // 多次下载
    size_t chunk_size; // chunk size of each download
    size_t chunk_offset; // offset of each chunk
    size_t chunk_index; // index of each chunk
    size_t chunk_count; // total chunk count
    http_download_data_cb *on_download_data;
    http_download_complete_cb *on_download_complete;
    http_download_error_cb *on_download_error;
    void *user_data;
} http_download_context_t;

// 初始化下载上下文
http_download_context_t *lws_http_download_init(void *parent_http_ctx, char *file_path, int file_size);

// 设置下载路径
int lws_http_download_set_url(http_download_context_t *dl_ctx, const char *url);
// 设置下载回调
int lws_http_download_set_callback(http_download_context_t *dl_ctx, http_download_data_cb *on_download_data, http_download_complete_cb *on_download_complete, http_download_error_cb *on_download_error, void *user_data);

// 设置下载文件路径
int lws_http_download_set_file_path(http_download_context_t *dl_ctx, const char *file_path);
/*
设置下载文件大小
@param file_size 文件大小
@return 0 成功，-1 失败
可选，如未配置，将在下载时自动获取
*/
int lws_http_download_set_file_size(http_download_context_t *dl_ctx, size_t file_size);

// 设置下载超时时间
int lws_http_download_set_timeout(http_download_context_t *dl_ctx, int timeout_ms);
// 设置下载CA证书
int lws_http_download_set_ca_crt(http_download_context_t *dl_ctx, const char *ca_crt);


// 发送HEAD请求获取文件信息
int lws_http_download_get_size(http_download_context_t *dl_ctx);

// 启动分段下载
int lws_http_download_start(http_download_context_t *dl_ctx);

// 取消下载
int lws_http_download_cancel(http_download_context_t *dl_ctx);

// 释放下载上下文
int lws_http_download_deinit(http_download_context_t *dl_ctx);

void _internal_download_handle(const char *in, size_t in_len, bool is_last_chunk, void *user_data);

#endif //PRIVATE_LWS_HTTP_DOWNLOAD_H
