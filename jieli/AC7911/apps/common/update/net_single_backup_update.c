#include "system/includes.h"
#include "app_config.h"
#include "update/update.h"
#include "fs/fs.h"
#include "syscfg/syscfg_id.h"
#include "update/update_loader_download.h"
#include "generic/errno-base.h"
#include "lwip/ip4_addr.h"
#include "wifi/wifi_connect.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

void sdfile_save_test(char *buf, int len, char one_file, char close);
void sdfile_read(void *buf, int offset, int len);
//========================网络单备份系统升级例子================================
#if (CONFIG_DOUBLE_BANK_ENABLE == 0)
#if 0
enum {
    SEEK_SET = 0x01,
    SEEK_CUR = 0x02
};
#endif

#ifdef CONFIG_WIFI_STA_MODE
#define HTTP_UPDATE_ENABLE	1 //1，STA模式下http断点续传OTA升级
#else
#define HTTP_UPDATE_ENABLE	0
#endif

#if HTTP_UPDATE_ENABLE
#define HTTP_UPDATE_PORT	80			//http端口
#define HTTP_RECV_BUF		(20*1024)	//http升级缓存大小

//HTTP升级的OTA升级文件网址
/*#define HTTP_UPDATE_URL 	"http://test02.jieliapp.com/file/2022/update-ota.ufw"*/
#define HTTP_UPDATE_URL 	"http://172.16.109.110:18888/update-ota.ufw"

#define HTTP_GET_HEAD		"GET %s HTTP/1.1\r\n"\
							"Host: %s\r\n"\
							"User-Agent: User\r\n"\
							"Accept: */*\r\n"\
							"Connection: Close\r\n"\
							"Range: bytes=%d-%d\r\n\r\n"
struct http_tcp {
    struct ip4_addr ip_addr;
    struct tcp_pcb *pcb;
    unsigned char conneted: 1;
    unsigned char lock: 7;
};
struct http_range_ota {
    struct http_tcp tcp;
    unsigned int file_size;//文件大小
    unsigned int content_length;
    unsigned int range_start;//断点续传起始
    unsigned int range_end;//断点续传结束
    unsigned int recv_offset;
    unsigned int read_offset;
    unsigned char *host;
    unsigned char *file;
    unsigned char *send_buf;
    unsigned int buf_size;
    unsigned char buf[0];
};
static struct http_range_ota *http_ota = NULL;
#endif

//升级端口
#define NET_UPDATE_PORT     3335


//升级状态
#define NET_UPDATE_STATUS_DOING     (0 << 0)//设备端主动发送
#define NET_UPDATE_STATUS_OK        (1 << 0)//设备端主动发送
#define NET_UPDATE_STATUS_ERR       (1 << 1)//设备端主动发送
#define NET_UPDATE_STATUS_READ      (1 << 2)//设备端主动发送
#define NET_UPDATE_STATUS_STOP      (1 << 3)//APP主动的停止升级
#define NET_UPDATE_STATUS_SAME      (1 << 4)//升级的是同一个固件

#define NET_OTA_FILE_RESV       0 //文件夹预留读取：0文件往后读取， 1往前读取

static struct tcp_pcb *client_pcb = 0;
static char client_accept = 0;
static char net_ota_stop = 0;
static char net_recv_err = 0;

#define OS_SEM_TIME_MAX	500 //5s
static OS_SEM read_sem;

//协议保持和APP和ota_loard一致
//协议：头部20字节
struct net_ota {
    unsigned int sum_check;//设备端：头部16字节和校验，APP端：头部16字节+读取字节的和校验，（sum_check不加上和校验计算）
    unsigned int seek_addr;//文件偏移地址
    unsigned int read_size;//读取文件长度
    unsigned int all_size;//升级过程中需要的文件总字节数（用在进度条），整个升级过程中会有一次的all_size > 0, 其他次all_size = 0
    unsigned int status: 8; //升级转态，NET_UPDATE_STATUS_NONE正在升级，NET_UPDATE_STATUS_OK升级成功，NET_UPDATE_STATUS_ERR升级失败
    unsigned int download_percent: 8; //升级过程中设备的百分比
    unsigned int resv: 16; //预留，最后收到这4个字节代表成功和失败
    unsigned char buf[0];
};
struct net_local { //除了buf以外，其他成员保持和struct net_ota一致
    unsigned int sum_check;
    unsigned int seek_addr;
    unsigned int read_size;
    unsigned int all_size;
    unsigned int status: 8; //预留，最后发送这4个字节代表成功和失败
    unsigned int download_percent: 8; //升级过程中设备的百分比
    unsigned int resv: 16; //预留，最后发送这4个字节代表成功和失败
    unsigned char *buf;//存留升级fread的地址
};

static u8 *ota_ram_addr = NULL; //OTA缓存地址，长度42*1024,42K
static struct net_local *net_local_addr = NULL;
static u32 ota_loard_size = 0;//OTA升级需要字节大小
static u32 ota_read_size = 0;//OTA升级代码后已读取的字节大小

#define NET_OTA_BUF_SIZE	(20*1024) //务必<=40K，和APP相关
#define RECV_OFFSET_SIZE 	(sizeof(struct net_local))
#define RECV_BUF_MAX_SIZE 	(NET_OTA_BUF_SIZE - sizeof(struct net_local) - sizeof(struct net_ota))
#define NET_MAX_SIZE 		(RECV_BUF_MAX_SIZE)
#define NET_TCP_OTA			(ota_ram_addr + sizeof(struct net_local))

#define UPDATE_IDLE		0
#define UPDATE_ERROR	1
#define UPDATE_DOING	2
#define UPDATE_SUCCES	3
#define UPDATE_STOP		4
static u8 update_succes_state = 0;

static unsigned int sumget(void *buf, int len)//和校验生成
{
    unsigned char *pbuf = (unsigned char *)buf;
    unsigned int sum = 0;
    int i = 0;
    while (i < len) {
        sum += pbuf[i++];
    }
    return sum;
}
static int sumcheck(void *buf, int len, unsigned int sum)//和校验检测
{
    unsigned char *pbuf = (unsigned char *)buf;
    unsigned int sum_check = 0;
    int i = 0;
    while (i < len) {
        sum_check += pbuf[i++];
    }
    if (sum_check != sum) {
        printf("err sumcheck = 0x%x, 0x%x\n", sum_check, sum);
        return -1;
    }
    return 0;
}
static int send_msg(struct tcp_pcb *pcb, u8 *buf, u32 len)//往TCP发送数据包
{
    err_t err;
    int size = tcp_sndbuf(pcb);
    if (size < len) {
        printf("err tcp no buf to send , %d , %d\n", size, len);
        return 0;
    }
    size = MIN(size, len);
    err = tcp_write(pcb, buf, size, 1);//往TCP发送数据包
    if (err != ERR_OK) {
        puts("tcp_write err \n");
        return 0;
    }
    return size;
}
int msgsenddata(u8 *buf, u32 len, u8 no_wait)//读取缓冲区的指定地址和长度
{
    int err = 0;
    struct net_local *net_local = (struct net_local *)(buf);
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    if (len > RECV_OFFSET_SIZE) {
        return 0;
    }
    if (client_pcb) {
        int tcp_size = RECV_BUF_MAX_SIZE;
        u32 read_size = 0;
        if (net_local_addr->status == NET_UPDATE_STATUS_READ) {
            if (!net_local->all_size) {
                if (!net_ota->seek_addr && !net_ota->read_size) {
                    net_ota->seek_addr = net_local->seek_addr;
                }
                if (!net_ota->read_size || net_local->seek_addr < net_ota->seek_addr ||
                    ((net_local->seek_addr + net_local->read_size) > (net_ota->seek_addr + net_ota->read_size))) {//没有数据则请求tcp
                    net_ota->all_size = 0;
                    net_ota->read_size = tcp_size;
                    net_ota->seek_addr = net_local->seek_addr;
                } else {//二级缓存有数据则复制即可
                    memcpy(net_local->buf, net_ota->buf + (net_local->seek_addr - net_ota->seek_addr), net_local->read_size);
                    return net_local->read_size;
                }
#if NET_OTA_FILE_RESV
                if (net_ota->seek_addr > (RECV_BUF_MAX_SIZE - net_local->read_size)) {
                    net_ota->seek_addr -= (RECV_BUF_MAX_SIZE - net_local->read_size);
                }
#endif
            } else {
                net_ota->all_size = net_local->all_size;
            }
        } else {
            read_size = net_ota->read_size;
            net_ota->read_size = 0;
            net_ota->all_size = net_local->all_size ? net_local->all_size : net_ota->all_size;
        }
        os_sem_set(&read_sem, 0);
        net_ota->download_percent = net_local_addr->download_percent;
        net_ota->status = net_local_addr->status;//发送结果
        net_ota->sum_check = sumget((char *)net_ota + sizeof(net_ota->sum_check), sizeof(struct net_ota) - sizeof(net_ota->sum_check));
        err = send_msg(client_pcb, (u8 *)net_ota, len);
        if (read_size) {
            net_ota->read_size = read_size;
        }
        if (no_wait) {
            return err;
        }
        if (os_sem_pend(&read_sem, OS_SEM_TIME_MAX)) {
            return 0;
        }
    }
    return err;
}
static err_t msgsend_buf(void *arg, struct tcp_pcb *pcb)
{
    return ERR_OK;
}
static void msgerr(void *arg, err_t err)
{
}
static err_t msgpoll(void *arg, struct tcp_pcb *pcb)
{
    return msgsend_buf(arg, pcb);
}
static err_t msgsent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    return msgsend_buf(arg, pcb);
}
static int msgclose(struct tcp_pcb *pcb)
{
    if (pcb) {
        if (client_accept > 0) {
            client_accept--;
        }
        if (client_accept == 0 || client_pcb == pcb) {
            client_pcb = NULL;
        }
#if HTTP_UPDATE_ENABLE
        http_ota->tcp.lock = 1;
#endif
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_abort(pcb);//关闭tcp就发送RST复位断开tcp并释放tcp占用的内存池
        /*tcp_close(pcb);*/
#if HTTP_UPDATE_ENABLE
        if (http_ota && http_ota->tcp.pcb == pcb) {
            http_ota->tcp.pcb = NULL;
            http_ota->tcp.conneted = 0;
        }
        http_ota->tcp.lock = 0;
#endif
        return ERR_ABRT;
    }
    return ERR_OK;
}
static err_t msgrecvmemcpy(void *arg, int len)
{
    struct net_local *net_local = (struct net_local *)(ota_ram_addr);
    struct net_ota *net_ota = (struct net_ota *)arg;

    if (net_ota->status == NET_UPDATE_STATUS_STOP) {
        net_local->status = net_ota->status;
        net_ota_stop = 1;
    } else if (net_local->read_size) {//复制当前请求的地址和大小到一级缓存
        memcpy(net_local->buf, net_ota->buf + (net_local->seek_addr - net_ota->seek_addr), net_local->read_size);
    }
    os_sem_post(&read_sem);
    return ERR_OK;
}
static err_t msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
#if HTTP_UPDATE_ENABLE
    struct http_range_ota *httpota = (struct http_range_ota *)arg;
    if (!httpota) {
        return ERR_OK;
    }
    if (err == ERR_OK && p != NULL) {
        struct pbuf *q;
        char Find = 0;
        int tot_len = 0;
        char *pt = (char *)httpota->buf + httpota->recv_offset;
        int len = p->tot_len;
        if ((httpota->recv_offset + len) <= httpota->buf_size) {
            for (q = p; q != NULL; q = q->next) {
                memcpy(pt, q->payload, q->len);
                pt += q->len;
                tot_len += q->len;
            }
            tcp_recved(pcb, tot_len);
            /*printf("recv size : %d \n%s\n", tot_len, httpota->buf + httpota->recv_offset);*/
            if (!httpota->recv_offset) {
                char *p, *f, *c;
                p = (char *)httpota->buf + httpota->recv_offset;
                f = strstr(p, "HTTP/1.1 206");
                /*sdfile_save_test(p, tot_len, 0, 1);*/
                if (f) {
                    net_recv_err = 0;
                    c = strstr(f, "Content-Length:");
                    if (!c) {
                        c = strstr(f, "content-length:");
                    }
                    if (c) {
                        c += strlen("Content-Length:");
                        httpota->content_length = atoi(c);
                    }
                    c = strstr(f, "Content-Range:");
                    if (!c) {
                        c = strstr(f, "content-range:");
                    }
                    if (c) {
                        c += strlen("Content-Range:") + 1;
                        f = c;
                        while (c < (p + tot_len)) {
                            if (c[0] == '\r' && c[1] == '\n' && c[2] == '\r' && c[3] == '\n') {
                                c += 4;
                                Find = 1;
                                goto find;
                            }
                            if (c[0] == '\r' && c[1] == '\n') {
                                c += 2;
                                break;
                            }
                            if (c[0] == '/') {
                                httpota->file_size = atoi(++c);
                                /*printf("---> http ota file_size = %d \n", httpota->file_size);*/
                            }
                            c++;
                        }
                        f = strstr(f, "/");
                        if (f && f < c && !httpota->file_size) {
                            f += 1;
                            httpota->file_size = atoi(f);
                            /*printf("---> http ota file_size = %d \n",httpota->file_size);*/
                        }
                    } else {
                        c = f;
                    }
                    while (c < (p + tot_len)) {
                        if (c[0] == '\r' && c[1] == '\n' && c[2] == '\r' && c[3] == '\n') {
                            Find = 1;
                            c += 4;
                            break;
                        }
                        c++;
                    }
find:
                    if (Find) {
                        len = tot_len - (c - p);
                        memcpy(httpota->buf, c, len);
                        httpota->buf[len] = 0;
                        httpota->recv_offset += len;
                    } else {
                        printf("recv head data err\n");
                        goto err;
                    }
                } else {
err:
                    printf("recv err \n");
                    net_recv_err++;
                    if (net_recv_err >= 3) {
                        net_recv_err = 0;
                        net_ota_stop = 1;
                        httpota->recv_offset = 0;
                    }
                    os_sem_post(&read_sem);
                }
            } else {
                httpota->recv_offset += tot_len;
            }
        } else {
            tcp_recved(pcb, p->tot_len);
        }
        pbuf_free(p);
        /*printf("recv size : %d, request : %d \n", httpota->recv_offset, httpota->range_end - httpota->range_start);*/
        //接收数据>=所请求字节，则接收数据完成
        if (httpota->recv_offset && (httpota->recv_offset >= (httpota->range_end - httpota->range_start) || (http_ota->recv_offset >= http_ota->file_size && http_ota->file_size))) {
            /*printf("---> recv size : %d, request : %d \n", httpota->recv_offset, httpota->range_end - httpota->range_start);*/

            if (net_local_addr->buf && net_local_addr->read_size) {//复制到二级缓存20K里
                memcpy(net_local_addr->buf, httpota->buf + (net_local_addr->seek_addr - httpota->range_start), net_local_addr->read_size);
            }
            os_sem_post(&read_sem);
        }
    }
#else
    struct net_ota *net_ota = (struct net_ota *)arg;
    if (!net_ota || !ota_ram_addr) {
        return ERR_OK;
    }
    if (err == ERR_OK && p != NULL) {
        struct pbuf *q;
        char First = 0;
        int tot_len = 0;
        char *pt = (char *)net_ota + net_ota->all_size;
        for (q = p; q != NULL; q = q->next) {
            memcpy(pt, q->payload, q->len);
            pt += q->len;
            tot_len += q->len;
        }
        /*tcp_recved(pcb, p->tot_len);*/
        tcp_recved(pcb, tot_len);
        pbuf_free(p);
        net_ota->all_size += tot_len;
        if (net_ota->all_size >= (net_ota->read_size + sizeof(struct net_ota))) {//接收数据>=所请求字节，则接收数据完成
            net_ota->all_size = 0;
            //和校验检测，检测过才能复制到二级20K缓存
            if (sumcheck((char *)net_ota + sizeof(net_ota->sum_check), net_ota->read_size + sizeof(struct net_ota) - sizeof(net_ota->sum_check), net_ota->sum_check)) {
            } else { //和校验成功
                msgrecvmemcpy(net_ota, tot_len);
            }
        }
    }
#endif
    else if (err == ERR_OK && p == NULL) {
        puts("msgrecv close tcp\n");
        return msgclose(pcb);
    }
    return ERR_OK;
}
static err_t msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    client_accept++;
    if (client_accept > 1) {
        printf("client_accept = %d \n", client_accept);
        return msgclose(pcb);
    } else {
        client_pcb = pcb;
        tcp_arg(pcb, NET_TCP_OTA);
        tcp_recv(pcb, msgrecv);
        tcp_sent(pcb, msgsent);
        tcp_err(pcb, msgerr);
        tcp_poll(pcb, msgpoll, 1);
        int net_ota_single_back_update(void);//100ms后初始化单备份升级
        sys_timeout_add_to_task("sys_timer", NULL, net_ota_single_back_update, 100);
        puts("client accept ok\n");
    }
    return ERR_OK;
}
#if HTTP_UPDATE_ENABLE
static err_t tcp_dataconnected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct http_range_ota *httpota = (struct http_range_ota *)arg;
    tcp_recv(pcb, msgrecv);
    tcp_sent(pcb, msgsent);
    tcp_err(pcb, msgerr);
    httpota->tcp.conneted = 1;
    puts("connet server ok\n");
    return ERR_OK;
}
static int http_get(const char *url, char reset, int size)
{
    int port = 0;
    if (!url || !http_ota) {
        goto exit;
    }
    char *p = strstr(url, "http://");
    if (!p) {
        goto exit;
    }
    p += strlen("http://");
    char *e = strstr(p, ":");
    int len = 0;
    if (e) {
        len = e - p;
        if (len >= http_ota->buf_size) {
            goto exit;
        }
        e++;
        port = atoi(e);
        e = strstr(p, "/");
    } else {
        e = strstr(p, "/");
        if (e) {
            len = e - p;
            if (len >= http_ota->buf_size) {
                goto exit;
            }
        } else {
            http_ota->host = NULL;
            http_ota->file = NULL;
            http_ota->send_buf = NULL;
            http_ota->recv_offset = 0;
            http_ota->file_size = 0;
            goto exit;
        }
    }
    http_ota->host = http_ota->buf;
    memcpy(http_ota->host, p, len);
    http_ota->host[len] = 0;
    http_ota->send_buf = http_ota->host + len + 1;
    http_ota->file = NULL;
    http_ota->recv_offset = 0;
    if ((strlen(e) + len + 2) < http_ota->buf_size) {
        http_ota->file = http_ota->host + len + 1;
        len = strlen(e);
        memcpy(http_ota->file, e, len);
        http_ota->file[len] = 0;
        http_ota->send_buf = http_ota->file + len + 1;
    }
    if (!http_ota->tcp.ip_addr.addr || reset) {
        if (!atoi(http_ota->host)) {
            printf("http gethostbyname\n");
            struct hostent *host_ent = gethostbyname(http_ota->host);//域名解析
            if (!host_ent) {
                printf("  . ! lwip analayse ip err !!!\r\n");
                goto exit;
            }
            http_ota->tcp.ip_addr.addr = *((u32_t *)host_ent->h_addr);
        } else {
            http_ota->tcp.ip_addr.addr = inet_addr(http_ota->host);
        }
        printf("http update DNS remoute ip : %s  \n", inet_ntoa(http_ota->tcp.ip_addr.addr));
        http_ota->file_size = 0;
    }
    while (http_ota->tcp.lock) {
        os_time_dly(1);
    }
    http_ota->tcp.lock = 1;
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        goto exit;
    }
    ip_addr_t ip;
    tcp_arg(pcb, http_ota);
    IP_SET_TYPE_VAL(ip, IPADDR_TYPE_V4);
    ip4_addr_copy(*ip_2_ip4(&ip), http_ota->tcp.ip_addr);
    http_ota->tcp.pcb = pcb;
    port = port ? port : HTTP_UPDATE_PORT;
    if (tcp_connect(pcb, &ip, port, tcp_dataconnected) != ERR_OK) {
        tcp_connect(pcb, &ip, 8080, tcp_dataconnected);
    }
    int cnt = 10 * 100;
    while (!http_ota->tcp.conneted && --cnt) {
        os_time_dly(10);
    }
    if (!cnt || net_ota_stop) {
        tcp_arg(pcb, NULL);
        tcp_sent(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_close(pcb);
        http_ota->tcp.pcb = NULL;
        http_ota->tcp.ip_addr.addr = 0;
        goto exit;
    }
    size = size ? size : http_ota->buf_size;
    if (http_ota->file_size && (http_ota->range_start + http_ota->buf_size) > http_ota->file_size) {
        size = http_ota->file_size - http_ota->range_start;
    }
    http_ota->range_end = http_ota->range_start + size;
    if (http_ota->send_buf) {
        if (!http_ota->file) {
            http_ota->file = "/";
        }
        sprintf(http_ota->send_buf, HTTP_GET_HEAD, http_ota->file, http_ota->host, http_ota->range_start, http_ota->range_end - 1);
        /*printf("HTTP_GET_HEAD : \n%s\n", http_ota->send_buf);*/
        len = strlen(http_ota->send_buf);
        if (send_msg(pcb, http_ota->send_buf, len) != len) {//发送http数据头
            goto exit;
        }
    }
    http_ota->tcp.lock = 0;
    return 0;

exit:
    http_ota->tcp.lock = 0;
    puts("http update err !!! \n");
    return -EINVAL;
}
static int http_read(void *priv, char reset)
{
    struct net_local *net_local = (struct net_local *)priv;
    if (!http_ota || !net_local) {
        return 0;
    }
    if (!http_ota->range_start && !http_ota->recv_offset) {
        http_ota->range_start = net_local->seek_addr;
    }
    if (!http_ota->recv_offset || net_local->seek_addr < http_ota->range_start ||
        ((net_local->seek_addr + net_local->read_size) > (http_ota->range_start + http_ota->recv_offset))) {//没有数据或第一次则请求tcp
        http_ota->recv_offset = 0;
        http_ota->range_start = net_local->seek_addr;
    } else {//二级缓存有数据则复制即可
        memcpy(net_local->buf, http_ota->buf + (net_local->seek_addr - http_ota->range_start), net_local->read_size);
        return net_local->read_size;
    }
#if NET_OTA_FILE_RESV
    if (http_ota->range_start > (http_ota->buf_size - net_local->read_size)) {
        http_ota->range_start -= (http_ota->buf_size - net_local->read_size);
    }
#endif
    u8 cnt = 3;
redo:
    os_sem_set(&read_sem, 0);
    if (http_get(HTTP_UPDATE_URL, reset, 0)) {
        return 0;
    }
    if (os_sem_pend(&read_sem, OS_SEM_TIME_MAX)) {
        return 0;
    }
    if (net_recv_err && --cnt && !net_ota_stop) {
        goto redo;
    }
    return net_local->read_size ? net_local->read_size : http_ota->buf_size;
}
static int http_stop(int err)
{
    if (http_ota) {
        http_ota->tcp.lock = 1;
        if (http_ota->tcp.pcb) {
            tcp_arg(http_ota->tcp.pcb, NULL);
            tcp_sent(http_ota->tcp.pcb, NULL);
            tcp_recv(http_ota->tcp.pcb, NULL);
            tcp_err(http_ota->tcp.pcb, NULL);
            tcp_close(http_ota->tcp.pcb);
        }
        http_ota->tcp.lock = 0;
        http_ota->tcp.pcb = NULL;
        http_ota->tcp.ip_addr.addr = 0;
        http_ota->tcp.conneted = 0;
        memset(http_ota->buf, 0, http_ota->buf_size);
        memset(http_ota, 0, sizeof(struct http_range_ota));
    }
    return 0;
}
#endif
int net_ota_single_back_init(void)
{
#if HTTP_UPDATE_ENABLE
    int size = HTTP_RECV_BUF;
    if (!http_ota) {
        http_ota = malloc(size);
    }
    if (http_ota) {
        memset(http_ota, 0, size);
        http_ota->buf_size = size - sizeof(struct http_range_ota);
        puts("http update init ok \n");
    } else {
        return -1;
    }
    os_sem_create(&read_sem, 0);
    syscfg_write(VM_HTTP_SINGLE_UPDATE_URL, (char *)HTTP_UPDATE_URL, strlen(HTTP_UPDATE_URL) + 1);
    printf("---> HTTP_UPDATE_URL = %s \n", HTTP_UPDATE_URL);
#else
    int err = 0;
    int cnt = 0;
    if (ota_ram_addr) {
        printf("net_ota_single_back_init already\n");
        return 0;
    }
    if (!ota_ram_addr) {
        ota_ram_addr = malloc(NET_OTA_BUF_SIZE);
        if (!ota_ram_addr) {
            printf("net_ota_single_back_init err , no mem\n");
            return -1;
        }
    }
    memset(ota_ram_addr, 0, NET_OTA_BUF_SIZE);
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("tcp_new err\n");
        return -1;
    }
    tcp_bind(pcb, IP_ADDR_ANY, NET_UPDATE_PORT);
    pcb = tcp_listen(pcb);
    if (!pcb) {
        tcp_close(pcb);
        printf("tcp_new err\n");
        return -1;
    }
    tcp_accept(pcb, msgaccept);
    os_sem_create(&read_sem, 0);
    puts("net_ota_open, wait APP accept .....\n");
#endif
    return 0;
}
static u16 net_ota_single_back_fs_open(void)
{
#if HTTP_UPDATE_ENABLE
    int size = HTTP_RECV_BUF;
    if (!net_local_addr) {
        net_local_addr = malloc(sizeof(struct net_local));
        if (!net_local_addr) {
            return false;
        }
    }
    if (!http_ota) {
        http_ota = malloc(size);
        if (!http_ota) {
            return false;
        }
    }
    memset(http_ota, 0, size);
    memset(net_local_addr, 0, sizeof(struct net_local));
    http_ota->buf_size = size - sizeof(struct http_range_ota);
    net_local_addr->download_percent = 1;
    net_ota_stop = 0;
    if (!http_read((void *)net_local_addr, 1)) {
        puts("net_f_open err\n");
        return false;
    }
#else
    int cnt = 500;//5秒
    if (!ota_ram_addr) {
        return false;
    }
    while (!client_pcb && --cnt) {
        os_time_dly(10);
    }
    if (!cnt) { //等待APP连接超时
        return false;
    }
    os_sem_set(&read_sem, 0);
    //清空数据
    net_local_addr = (struct net_local *)ota_ram_addr;
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    memset(net_ota, 0, sizeof(struct net_ota));
    memset(net_local_addr, 0, sizeof(struct net_local));
    net_ota_stop = 0;
    net_local_addr->download_percent = 1;
#endif
    printf("net_ota open ok\n");
    return true;
}

static int net_ota_single_back_fs_seek(void *fp, u8 type, u32 offset)
{
#if HTTP_UPDATE_ENABLE
    if (!net_local_addr) {
        return -1;
    }
#else
    if (!net_local_addr || !ota_ram_addr) {
        return (int) - 1;
    }
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        return 0;
    }
#endif
    if (type == SEEK_SET) {
        net_local_addr->seek_addr = offset;
    } else {
        net_local_addr->seek_addr += offset;
    }
    //printf("---> seek_addr = 0x%X \n",net_local_addr->seek_addr);
    return 0;
}

static int net_ota_single_back_fs_need_size(void *p, u32 len)//第一次发送本次升级的总字节
{
#if HTTP_UPDATE_ENABLE
    if (!net_local_addr || net_ota_stop || !len) {
        return -1;
    }
    net_local_addr->all_size = len;
    ota_loard_size = len ? len : 0;
    ota_read_size = 0;
#else
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop || !len) {
        return -1;
    }
    net_local_addr->seek_addr = 0;
    net_local_addr->read_size = 0;
    net_local_addr->all_size = len;
    net_local_addr->status = len ? 0 : NET_UPDATE_STATUS_SAME;
    net_local_addr->resv = 0;
    ota_loard_size = len ? len : 0;
    ota_read_size = 0;
    msgsenddata(net_local_addr, sizeof(struct net_ota), 0);
#endif
    return 0;
}
static u16 net_ota_single_back_fs_read(void *fp, u8 *buff, u16 len)
{
    int err;
#if HTTP_UPDATE_ENABLE
    int cnt = 3;
    if (!net_local_addr || net_ota_stop) {
        printf("err ota read\n");
        return 0;
    }
    if (ota_loard_size) {
        u8 percent = (unsigned long long)ota_read_size * 100 / ota_loard_size;
        u8 last_percent = net_local_addr->download_percent;
        net_local_addr->download_percent = percent > net_local_addr->download_percent ? percent : net_local_addr->download_percent;
        net_local_addr->download_percent = net_local_addr->download_percent < 100 ? net_local_addr->download_percent : 99;
        if (net_local_addr->download_percent != last_percent) {
            /*printf("update >> %d%%\n", net_local_addr->download_percent);*/
        }
    }
    net_local_addr->read_size = len;
    net_local_addr->resv = 0;
    net_local_addr->buf = buff;
    /*os_sem_set(&read_sem, 0);*/
    /*printf("---> read = %d, seek = %d \n", len, net_local_addr->seek_addr);*/
redo:
    cnt--;
    if (!http_read((void *)net_local_addr, 0)) {
        if (cnt) {
            goto redo;
        }
        puts("http_read err\n");
        return 0;
    }
#else
    if (!net_local_addr || !ota_ram_addr) {
        printf("err ota read\n");
        return 0;
    }
    if (len > NET_MAX_SIZE) {
        printf("err ota read size\n");
        return 0;
    }
    net_local_addr->read_size = len;
    net_local_addr->status = NET_UPDATE_STATUS_READ;
    net_local_addr->resv = 0;
    net_local_addr->buf = buff;
    net_local_addr->all_size = 0;
    if (ota_loard_size) {
        u8 percent = (unsigned long long)ota_read_size * 100 / ota_loard_size;
        net_local_addr->download_percent = percent > net_local_addr->download_percent ? percent : net_local_addr->download_percent;
        net_local_addr->download_percent = net_local_addr->download_percent < 100 ? net_local_addr->download_percent : 99;
    }
    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        printf("err ota read net_ota_stop\n");
        return 0;
    }
    if (!msgsenddata(net_local_addr, sizeof(struct net_ota), 0)) {
        puts("net_f_read err\n");
    }

    if (net_local_addr->status == NET_UPDATE_STATUS_STOP || net_ota_stop) {
        printf("err ota read net_ota_stop\n");
        return 0;
    }
#endif
    net_local_addr->seek_addr += net_local_addr->read_size;
    if (ota_loard_size) {
        ota_read_size += net_local_addr->read_size;
    }
    return net_local_addr->read_size;
}
static u16 net_ota_single_back_fs_stop(u8 err)
{
    u8 same = err & ~UPDATE_RESULT_FLAG_BITMAP;
    switch (same) {
    case UPDATE_RESULT_UPDATE_FILE_SAME:
        update_succes_state = UPDATE_STOP;
        break;
    default:
        break;
    }
#if HTTP_UPDATE_ENABLE
    if (!net_local_addr) {
        return -1;
    }
    http_stop(err);
    memset(net_local_addr, 0, sizeof(struct net_local));
#else
    if (!net_local_addr || !ota_ram_addr) {
        return (int) - 1;
    }
    memset(net_local_addr, 0, sizeof(struct net_local));
    if (client_pcb) {
        net_local_addr->read_size = 0;
        if (!err) { //先发送100%
            net_local_addr->status = 0;//发送结果
            if (ota_loard_size) {
                net_local_addr->download_percent = 100;
            }
            msgsenddata(net_local_addr, sizeof(struct net_ota), 0);
            os_time_dly(50);
        }
        net_local_addr->status = err ? NET_UPDATE_STATUS_ERR : NET_UPDATE_STATUS_OK;//发送结果
        msgsenddata(net_local_addr, sizeof(struct net_ota), 1);
        net_local_addr->status = 0;
        net_local_addr->download_percent = 0;
    }
    os_time_dly(50);
    net_local_addr = NULL;
    if (client_pcb) {
        puts("client_pcb close\n");
        msgclose(client_pcb);
    }
    //清空数据
    struct net_ota *net_ota = (struct net_ota *)(NET_TCP_OTA);
    memset(net_ota, 0, sizeof(struct net_ota));
#endif
    net_ota_stop = 0;
    ota_loard_size = 0;
    ota_read_size = 0;
    puts("net_ota_fs_stop ok\n");
    return true;
}
static const update_op_api_t net_ota_single_back_fs_update_opt = {
    .f_open = net_ota_single_back_fs_open,
    .f_read = net_ota_single_back_fs_read,
    .f_seek = net_ota_single_back_fs_seek,
    .f_stop = net_ota_single_back_fs_stop,
    .notify_update_content_size = net_ota_single_back_fs_need_size,
};

//单备份升级
static void net_ota_single_back_fs_update_param_private_handle(UPDATA_PARM *p)
{
}
static void net_ota_single_back_fs_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;
    switch (state) {
    case UPDATE_CH_EXIT:
        if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
            update_succes_state = UPDATE_SUCCES;
            UPDATA_PARM *update_ram = UPDATA_FLAG_ADDR;
            memset(update_ram, 0, 32);
            UPDATA_PARM *p = UPDATA_FLAG_ADDR + sizeof(UPDATA_PARM);
            if (support_dual_bank_update_en) {
                memset(update_ram, 0, sizeof(UPDATA_PARM));
                update_ram->magic = type;
                update_result_set(UPDATA_SUCC);
                printf(">>>>>>>>>>>>>>>>>>update ok , cpu reset ...\n");
            } else {
                //单备份
                update_mode_api_v2(type, net_ota_single_back_fs_update_param_private_handle, NULL);
            }
#if HTTP_UPDATE_ENABLE
            wifi_enter_sta_mode("#@!$%^&*()##$$", "#@!$%^&*()##$$");
            os_time_dly(200);
#else
            void wifi_all_sta_discon(void);
            wifi_all_sta_discon();//断开所有连接STA
            os_time_dly(200);
#endif
            system_reset();
        } else {
            if (update_succes_state == UPDATE_DOING) {
                update_succes_state = UPDATE_ERROR;
            }
            update_result_set(UPDATA_DEV_ERR);
            printf("\nupdate err !!! \n");
        }
        break;
    }
}
#if HTTP_UPDATE_ENABLE
int net_ota_single_back_update(void);
int net_ota_single_back_update_state(void)
{
    static u8 update_retry_cnt = 0;//升级失败尝试次数
    switch (update_succes_state) {
    case UPDATE_ERROR:
        if (++update_retry_cnt < 6) {
            sys_timeout_add(NULL, net_ota_single_back_update, 500);
        } else {
            update_retry_cnt = 0;
        }
        break;
    case UPDATE_SUCCES:
    case UPDATE_STOP:
        update_retry_cnt = 0;
        break;
    default:
        sys_timeout_add(NULL, net_ota_single_back_update_state, 500);
        break;
    }
    return (int)update_succes_state;
}
#endif

//单备份网络升级例子
int net_ota_single_back_update(void)
{
    printf("\n>>>>>>>>>>>>net_ota_single_back_update\n");
#if HTTP_UPDATE_ENABLE
    if (net_ota_single_back_init()) {//http下先初始化
        return -1;
    }
    sys_timeout_add(NULL, net_ota_single_back_update_state, 500);
#endif
    update_mode_info_t info = {
        .type = NET_UFW_UPDATA,
        .state_cbk = net_ota_single_back_fs_update_state_cbk,
        .p_op_api = &net_ota_single_back_fs_update_opt,
        .task_en = 1,
    };
    update_succes_state = UPDATE_DOING;
    app_active_update_task_init(&info);
    return 0;
}
#endif

//===============sta模式下的http单备份升级例子===========================//
static void net_single_http_update_doing(void)
{
#if HTTP_UPDATE_ENABLE
    int state = wifi_get_sta_connect_state();
    switch (state) {
    case WIFI_STA_NETWORK_STACK_DHCP_SUCC:
        net_ota_single_back_update();//获取IP后启动单备份升级流程
        return;
    case WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("NOT_FOUND_SSID\n");
        break;
    case WIFI_STA_CONNECT_ASSOCIAT_TIMEOUT:
        puts("CONNECT_ASSOCIAT_TIMEOUT\n");
        break;
    default:
        break;
    }
    sys_timeout_add(NULL, net_single_http_update_doing, 500);//定时500ms检测是否连上网络
#endif
}
//应用层调用函数例子
void net_single_http_update_test(void)
{
#if HTTP_UPDATE_ENABLE
    sys_timeout_add(NULL, net_single_http_update_doing, 500);//定时500ms检测是否连上网络
#endif
}




