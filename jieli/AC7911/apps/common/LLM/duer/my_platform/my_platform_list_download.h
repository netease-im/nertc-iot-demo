#ifndef _MY_LIST_DOWNLOAD_H_
#define _MY_LIST_DOWNLOAD_H_

#include "system/includes.h"
#include "circular_buf.h"
#include "net_download.h"
#include "http/http_cli.h"
#include <stdlib.h>
#include "my_platform_common.h"
#include "os/os_api.h"

extern int net_download_open(void **priv, struct net_download_parm *parm);
extern u32 net_download_get_tmp_data_size(void *priv);
extern int net_download_read(void *priv, void *buf, u32 len);
extern void net_download_get_status(void *priv, int *download_status, int *http_err_status);
extern int net_download_close(void *priv);

typedef struct DownloadState {
    char *current_url;             // 当前下载的URL
    char file_path[128];           // 文件保存路径
    u32 total_downloaded_bytes;    // 当前URL下载字节数
    u32 total_read_bytes;          // 已读取字节数
    u8 finished;                   // 下载完成标志
    int file_counter;              // 文件计数器
    u8 is_last_url;                // 新增：标记是否是最后一个URL
} DownloadState;

// 初始化下载状态
void download_state_init(DownloadState *state);

// 设置下一个下载URL
void download_state_set_next_url(DownloadState *state);

// 准备下一个下载任务
int download_state_prepare_next(DownloadState *state);

// 清理状态资源
void download_state_cleanup(DownloadState *state);



#define NET_BUF_SIZE (10 * 1024)
#define DOWNLOAD_COMPLETE_STATUS 2

typedef struct FileDownloader {
    void *net_file;           // 下载文件句柄
    char *net_buf;            // 网络缓冲区
    struct net_download_parm parm; // 下载参数
} FileDownloader;

typedef void (*DownloadCompleteCallback)(DownloadState *state);

// 初始化下载器
int file_downloader_init(FileDownloader *downloader, DownloadState *state);

// 执行下载
int file_downloader_execute(FileDownloader *downloader,
                            DownloadState *state,
                            DownloadCompleteCallback callback);

// 清理下载器资源
void file_downloader_cleanup(FileDownloader *downloader);



typedef struct {
    int run;          // 任务运行标志
} TaskStatus;

// 启动下载任务
void task_start_download(void);

// 终止下载任务
void task_stop_download(void);

// 获取任务状态
TaskStatus task_get_status(void);



typedef struct {
    char *buf;
    cbuffer_t cbuf;
} net_buf_t;

extern void task_stop_download(void);

extern TaskStatus task_get_status(void);

static void on_download_complete(DownloadState *state);
extern void download_music_task(void *p) ;
extern void task_start_download(void) ;
extern void my_list_download_task() ;
// extern unsigned long os_get_time() ;
static void my_net_download_kill_cb(void) ;
static void download_task_exit() ;
#endif
