#include "system/includes.h"
#include "os/os_api.h"
#include "http/http_cli.h"
#include "fs/fs.h"
#include <stdlib.h>
#include "storage_device.h"
#include "net_update.h"
#include "app_config.h"
#include "wifi/wifi_connect.h"
#include "http/http_cli.h"
#include "json_c/json_object.h"
#include "json_c/json_tokener.h"
#include "syscfg_id.h"

#define JLFILE_VERSION_LEN 32
typedef struct {
    unsigned short headCrc;
    unsigned short dataCrc;
    unsigned int   address;
    unsigned int   length;

    unsigned char u8Attribute;
    unsigned char res;
    unsigned short fileNum;
    char fileName[16];
    char version[JLFILE_VERSION_LEN];
} JLExtUpateHead;

void switch_upgrade_dev(u8 dev_type);
int get_current_version(char *version);

/** 固件URL*/
#define OTA_VERSION_URL "https://profile.jieliapp.com/license/v1/fileupdate/download/0537cd6f-c354-4fd5-ac2b-8e9843eb08c9"


#define DEFAULT_RECV_BUF_SIZE   (4*1024)   //数据缓存BUF大小
#define MAX_RECONNECT_CNT 10               //最大重连次数

struct download_hdl {
    char *url;                    //下载链接
    u32 file_size;                //需要下载的文件大小
    u8 *recv_buf;                 //数据缓存BUF
    u32 recv_buf_size;            //数据缓存BUF大小
    u32 download_len;             //已下载的数据长度
    u32 reconnect_cnt;             //重连次数
    httpcli_ctx ctx;              //http请求参数
    const struct net_download_ops *download_ops; //http操作集
};

struct download_parm {
    char *url;  //下载链接
};

//设备版本号长度
#define VERSION_LEN 16

/* 升级固件个数*/
#define UPDATE_FILE_NUM 2

/* 设备类型*/
#define FLASH_DEV_TYPE 0    //flash
#define SD_DEV_TYPE    1    //sd

/* 设备当前版本号 */
static char g_currentVersion[VERSION_LEN] = "v1.0.0-001";  //应当填写烧录时的版本号

/* 保存需要升级的固件信息*/
static JLExtUpateHead gupdateTable[UPDATE_FILE_NUM];

/* 保存从服务器上获取到的最新版本信息 */
static char g_latestVersion[VERSION_LEN] = {0};

int ext_head_vaild_check(JLExtUpateHead *head)
{
    if (NULL == head) {
        return -1;
    }

    unsigned short calc_crc = CRC16((unsigned char *) & (head->dataCrc), sizeof(JLExtUpateHead) - 2);
    if (head->headCrc == calc_crc) {
        return 0;
    }

    return -1;
}

/* 底层会调用获取固件最新的版本号, 版本号最多长度为16字节*/
char *get_latest_version(void)
{
    return g_latestVersion;
}

//http回调函数
static int http_user_cb(void *ctx, void *buf, unsigned int size, void *priv, httpin_status status)
{
    struct download_hdl *hdl = (struct download_hdl *)priv;

    if (status == HTTPIN_HEADER) {
        //第一次连接时，记录文件总长度
        if (hdl->ctx.lowRange == 0) {
            hdl->file_size = hdl->ctx.content_length;
        }

        log_e("http_need_download_file=%d KB", hdl->file_size / 1024);
    }
    return 0;
}

static void download_task(void *priv)
{
    struct download_hdl *hdl = priv;
    void *update_fd = NULL;
    int ret = 0;
    int err = 0;
    char sock_err = 0;
    int total = 0;
    JLExtUpateHead extHead;


_reconnect_:
    //发起连接请求，建立socket连接
    ret = hdl->download_ops->init(&hdl->ctx);
    if (ret != HERROR_OK) {
        if (hdl->ctx.req_exit_flag == 0) {
            if (hdl->reconnect_cnt < MAX_RECONNECT_CNT) {
                hdl->reconnect_cnt++;
                goto _reconnect_;
            } else {
                log_e("download reconnect upto max count.\n");
                goto _out_;
            }
        }
    }

    /* 获取目录头部，根据头部信息获取升级固件信息 */
    ret = hdl->download_ops->read(&hdl->ctx, (char *)&extHead, sizeof(JLExtUpateHead));
    if (ret !=  sizeof(JLExtUpateHead)) {
        printf("get extHead err!");
        sock_err = 1;
        goto _out_;
    }

    put_buf(&extHead, sizeof(JLExtUpateHead));
    if (ext_head_vaild_check(&extHead)) {
        printf("check extHead err!\n");
        sock_err = 1;
        goto _out_;
    } else {
        if (0 == memcmp(extHead.fileName, "EXT_UPDATE", strlen("EXT_UPDATE"))) {
            printf("get EXT_UPDATE head\n");
        } else {
            printf("EXT_UPDATE information not found\n");
            sock_err = 1;
            goto _out_;
        }
    }

    /* 获取需要升级的固件头部信息 */
    char *r_buf = (char *)&gupdateTable;
    int r_len = extHead.fileNum * sizeof(JLExtUpateHead);
    int len = 0;
    while (1) {
        ret = hdl->download_ops->read(&hdl->ctx, r_buf + len, r_len - len);
        if (ret < 0) {
            sock_err = 1;
            goto _out_;
        } else {
            len += ret;
        }

        if (len >= r_len) {
            break;
        }
    }

    /* 对需要升级的设备进行升级 */
    int cnt = 0;
    while (cnt < extHead.fileNum) {
        if (ext_head_vaild_check(&gupdateTable[cnt])) {
            printf("ext_head_vaild_check gupdateTable head err!\n");
            sock_err = 1;
            goto _out_;
        } else {
            if (0 == memcmp(gupdateTable[cnt].fileName, "appUpdate.ufw", strlen("appUpdate.ufw"))) {
                printf("find %s\n", gupdateTable[cnt].fileName);

                /* 升级flash时需要转换到flash设备 */
                switch_upgrade_dev(FLASH_DEV_TYPE);//flash
            } else if (0 == memcmp(gupdateTable[cnt].fileName, "exAppUpdate.ufw", strlen("exAppUpdate.ufw"))) {
                printf("find %s\n", gupdateTable[cnt].fileName);

                /* 升级SD时需要转换到SD设备 */
                switch_upgrade_dev(SD_DEV_TYPE);//sd
            } else {
                printf("unable to find the upgrade firmware\n");
                sock_err = 1;
                goto _out_;
            }

            /* 获取当前设备的固件版本号 */
            get_current_version(g_currentVersion);
            if (0 == strcmp(gupdateTable[cnt].version, g_currentVersion)) {
                printf("the upgrade firmware version information is the same\n");
                strcpy(get_latest_version(), gupdateTable[cnt].version);
                cnt++;
                continue;
            } else {
                strcpy(get_latest_version(), gupdateTable[cnt].version);
            }

            update_fd = net_fopen(CONFIG_UPGRADE_OTA_FILE_NAME, "w");
            if (!update_fd) {
                log_e("open update_fd error\n");
                goto _out_;
            }

            int dsize = 0;
            hdl->file_size = gupdateTable[cnt].length;
            hdl->download_len = 0;
            while (hdl->ctx.req_exit_flag == 0) {
                dsize = (hdl->file_size - hdl->download_len) > hdl->recv_buf_size ? hdl->recv_buf_size : (hdl->file_size - hdl->download_len);
                ret = hdl->download_ops->read(&hdl->ctx, (char *)hdl->recv_buf, dsize);//最大接收为recv_buf_size
                if (ret <  0) {  //读取数据失败
                    sock_err = 1;
                    goto _out_;
                } else {
                    hdl->download_len += ret;
                    err = net_fwrite(update_fd, hdl->recv_buf, ret, 0);
                    if (err != ret) {
                        goto _out_;
                    }
                }

                if (hdl->download_len >= hdl->file_size) {
                    printf("download %s success!\n", gupdateTable[cnt].fileName);
                    total++;
                    sock_err = 0;
                    net_fclose(update_fd, sock_err);
                    update_fd = NULL;
                    break;
                }
            }

            hdl->download_len = 0;
            cnt++;
        }
    }

_out_:

    if (total) {
        system_reset();
    }

    //关闭网络连接
    hdl->download_ops->close(&hdl->ctx);

    if (update_fd) {
        net_fclose(update_fd, sock_err);
        update_fd = NULL;
    }
    free(hdl->url);
    free(hdl->recv_buf);
    free(hdl);

}

static int http_create_download_task(void)
{
    struct download_parm parm = {0};
    struct download_hdl *hdl = NULL;

    parm.url = OTA_VERSION_URL;

    //申请堆内存
    hdl = (struct download_hdl *)calloc(1, sizeof(struct download_hdl));

    if (hdl == NULL) {
        return -ENOMEM;
    }

    hdl->download_ops = &http_ops;
    hdl->recv_buf_size = DEFAULT_RECV_BUF_SIZE;

    hdl->url = (char *)calloc(1, strlen(parm.url) + 1);

    if (hdl->url == NULL) {
        return -ENOMEM;
    }

    hdl->recv_buf = (u8 *)calloc(1, hdl->recv_buf_size);

    if (hdl->recv_buf == NULL) {
        return -ENOMEM;
    }

    strcpy(hdl->url, parm.url);

    //http请求参数赋值
    hdl->ctx.url = hdl->url;
    hdl->ctx.cb = http_user_cb;
    hdl->ctx.priv = hdl;
    hdl->ctx.connection = "close";

    //创建下载线程
    return thread_fork("download_task", 20,  2 * 1024, 0, NULL, download_task, hdl);
}

static void http_upgrade_start(void *priv)
{
    enum wifi_sta_connect_state state;

    while (1) {
        printf("Connecting to the network...\n");
        state = wifi_get_sta_connect_state();
        if (WIFI_STA_NETWORK_STACK_DHCP_SUCC == state) {
            printf("Network connection is successful!\n");
            break;
        }

        os_time_dly(1000);
    }

    http_create_download_task();
}

//应用程序入口,需要运行在STA模式下
void c_main(void *priv)
{
    if (thread_fork("http_upgrade_start", 10, 4 * 1024, 0, NULL, http_upgrade_start, NULL) != OS_NO_ERR) {
        printf("thread fork fail\n");
    }
}

late_initcall(c_main);

