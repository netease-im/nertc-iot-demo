#include "product_main.h"

#ifdef PRODUCT_TEST_ENABLE

#include "sock_api/sock_api.h"
#include "app_config.h"
#include "system/includes.h"
#include "wifi/wifi_connect.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "asm/usb.h"
#include "uac_stream.h"
#include "board_config.h"
#include "server/video_server.h"


#ifdef PRODUCT_NET_SERVER_ENABLE

#define TXINFO_SIZE			  4
#define USB_DIR_OUT	          0
#define USB_DIR_IN	          0x80
#define WIFI_TXWI_HEAD_SIZE   20
#define WIFI_80211_FILL_SIZE  27 //保留一些字节不填充, 为了避开duration段和上层发送1个字节也支持

#define SPK_FRAME_SIZE		  (2 * 1024)


static struct product_net_type {
    u16 time_hdl;
    u8 breathe_idx;
    int camera_pid;
    void *camera_sock;
    int audio_ctl_pid;
    int audio_data_pid;
    void *audio_ctl_sock;
    void *audio_data_sock;
    struct sockaddr_in remote_addr;
    struct sockaddr_in host_dest;
    u8 conn_flag, wifi_conn;
    u8 conn_ssid[33], conn_pwd[64];
    void *host_sock, *breathe_sock;
    OS_SEM asem, bsem, bs_sem, br_sem, camera_sem, audio_data_sem;
    enum WIFI_MODE server_mode;
    __attribute__((aligned(4))) u8 *wifi_send_pkg;
    OS_SEM spk_sem;
    void *spk_buf, *mic_buf;
    u32 spk_buf_size, mic_buf_size;
    cbuffer_t spk_cbuf, mic_cbuf;
} *__THIS = NULL;


static u8 breathe_ctl(u8 on);
static void build_dev_conn(void);
static void delete_dev_conn(void);
static void build_data_conn(void);
static void delete_data_conn(void);


static void wifi_tx_data(u8 *pkg, int len, u32 rate, u8 bw, u8 short_gi)// 最大包1513
{
    u16 *PktLen = &__THIS->wifi_send_pkg[0];
    u16 *MPDUtotalByteCount = &__THIS->wifi_send_pkg[10];
    *PktLen = WIFI_80211_FILL_SIZE + len + WIFI_TXWI_HEAD_SIZE + 4 - 8;
    *MPDUtotalByteCount = WIFI_80211_FILL_SIZE + len;
    const  struct  {
        const char *string;
        u8 phy;
        u8 mcs;
    } tx_rate_tab[] = {
        {"1M",		0,		0},
        {"2M",		0,		1},
        {"5.5M",	0,		2},
        {"11M",		0,		3},

        {"6M",		1,		0},
        {"9M",		1,		1},
        {"12M",		1,		2},
        {"18M",		1,		3},
        {"24M",		1,		4},
        {"36M",		1,		5},
        {"48M",		1,		6},
        {"54M",		1,		7},

        {"HTMCS0",	2,		0},
        {"HTMCS1",	2,		1},
        {"HTMCS2",	2,		2},
        {"HTMCS3",	2,		3},
        {"HTMCS4",	2,		4},
        {"HTMCS5",	2,		5},
        {"HTMCS6",	2,		6},
        {"HTMCS7",	2,		7},
    };

    if (pkg) {
        //put_buf(pkg, len);
        memcpy(&__THIS->wifi_send_pkg[WIFI_TXWI_HEAD_SIZE + WIFI_80211_FILL_SIZE], pkg, len);
    } else {
        memset(&__THIS->wifi_send_pkg[WIFI_TXWI_HEAD_SIZE + WIFI_80211_FILL_SIZE], 0xaa, len);
    }

    struct urb urb;
    urb.pipe = USB_DIR_OUT;
    urb.complete = NULL;
    urb.transfer_buffer = __THIS->wifi_send_pkg;
    urb.transfer_buffer_length = len + WIFI_TXWI_HEAD_SIZE + WIFI_80211_FILL_SIZE + 4;

    PTXWI_STRUC pTxWI = TXINFO_SIZE + (u8 *)urb.transfer_buffer;
    pTxWI->PHYMODE    = tx_rate_tab[rate].phy;
    pTxWI->MCS        = tx_rate_tab[rate].mcs;
    pTxWI->BW         = bw;
    pTxWI->ShortGI    = !short_gi;
    usb_submit_urb(&urb, 0);
}


static void broadcast_task(void *priv)
{
    u8 idx = 0;
    struct wifi_store_info *info = get_cur_wifi_info();
    u8 ch[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

    struct product_conn conn = {
        .str = "product_ask",
        .reset = 1,
        .spk_value = ((SPK_CHANNEL << 24) | SPK_AUDIO_RATE),
        .mic_value = ((MIC_CHANNEL << 24) | MIC_AUDIO_RATE),
        .mic_res   = MIC_AUDIO_RES,
    };

    conn.mode = info->mode;
    if (info->mode == AP_MODE) {
        strcpy(conn.ssid, info->ssid[info->mode - STA_MODE]);
        strcpy(conn.pwd, info->pwd[info->mode - STA_MODE]);
    }

    for (;;) {
        putchar('C');
        wifi_set_channel(ch[(++idx) % ARRAY_SIZE(ch)]);
        for (u8 i = 0; i < 5; i++) {
            wifi_tx_data(&conn, sizeof(struct product_conn), 0, 0, 0);
            if (!os_sem_accept(&__THIS->asem)) {
                os_sem_post(&__THIS->bsem);
                if (__THIS->server_mode == AP_MODE) {
                    wifi_set_channel(PRODUCT_SER_AP_CH);
                } else {
                    build_dev_conn();
                }
                return;
            }
        }
    }
}


static u8 broadcast_ctl(u8 ctl)
{
    static int pid = 0;
    if (ctl & !pid) {
        __THIS->conn_flag = FALSE;
        thread_fork("broadcast_task", 30, 256, 0, &pid, broadcast_task, NULL);
        return TRUE;
    }

    if (!ctl && pid) {
        os_sem_post(&__THIS->asem);
        pid = 0;
        //os_sem_pend(&__THIS->bsem, 0);
        return TRUE;
    }

    return FALSE;
}


static void breathe_timeout_hdl(void *priv)
{
    __THIS->breathe_idx ++;
    product_info("%s, __THIS->breathe_idx = %d\n", __FUNCTION__, __THIS->breathe_idx);
    if (__THIS->breathe_idx < 5) {
        __THIS->time_hdl = sys_timeout_add(NULL, breathe_timeout_hdl, BREATHE_TIMEOUT);
    } else {
        __THIS->breathe_idx = 0;
        breathe_ctl(FALSE);
        delete_camera_data_conn();
#ifdef CONFIG_AUDIO_ENABLE
        delete_audio_data_conn();
#endif
        delete_data_conn();
        delete_dev_conn();
    }
}


static void breathe_recv_task(void)
{
    u8 recv_buf[32], recv_len, idx = 0;
    socklen_t len = sizeof(struct sockaddr_in);

    for (;;) {
        if (__THIS->breathe_sock) {
            recv_len = sock_recvfrom(__THIS->breathe_sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&__THIS->remote_addr, &len);
            //product_info("\n recv_len is %d, %s\n", recv_len, recv_buf);
            if (recv_len > 0 && !strcmp(BREATHE_STR, recv_buf)) {
                __THIS->breathe_idx = 0;
                sys_timer_modify(__THIS->time_hdl, BREATHE_TIMEOUT);
                if (!__THIS->host_sock) {
                    product_info("Received data from (ip : %s, port : %d)\r\n", inet_ntoa(__THIS->remote_addr.sin_addr), __THIS->remote_addr.sin_port);
                    build_data_conn();
                    void product_camera_reconnect(void);
                    product_camera_reconnect();
#ifdef CONFIG_AUDIO_ENABLE
                    build_audio_data_conn();
#endif
                }
            }
        }

        if (!os_sem_accept(&__THIS->br_sem)) {
            os_sem_del(&__THIS->br_sem, 0);
            return;
        }
    }
}


static void breathe_send_task(void *priv)
{
    struct sockaddr_in dest;

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest.sin_port = htons(BREATHE_PORT);

    __THIS->time_hdl = sys_timeout_add(NULL, breathe_timeout_hdl, BREATHE_TIMEOUT);
    for (;;) {
        if (__THIS->breathe_sock) {
            sock_sendto(__THIS->breathe_sock, BREATHE_STR, strlen(BREATHE_STR), 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
        }
        os_time_dly(20);

        if (!os_sem_accept(&__THIS->bs_sem)) {
            os_sem_del(&__THIS->bs_sem, 0);
            return;
        }
    }
}


static u8 breathe_ctl(u8 on)
{
    u32 opt = 1;

    if (on) {
        if (__THIS->breathe_sock) {
            return TRUE;
        }

        __THIS->breathe_sock = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL);
        if (__THIS->breathe_sock == NULL) {
            product_err("udp sock_reg fail.\n");
            return FALSE;
        }
        sock_setsockopt(__THIS->breathe_sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

        os_sem_create(&__THIS->bs_sem, 0);
        os_sem_create(&__THIS->br_sem, 0);
        thread_fork("breathe_send_task", 29, 256, 0, NULL, breathe_send_task, NULL);
        thread_fork("breathe_recv_task", 30, 256, 0, NULL, breathe_recv_task, NULL);
        return TRUE;
    } else {
        if (__THIS->breathe_sock) {
            sys_timer_del(__THIS->time_hdl);
            os_sem_post(&__THIS->bs_sem);
            os_sem_post(&__THIS->br_sem);
            sock_unreg(__THIS->breathe_sock);
            __THIS->breathe_sock = NULL;
            __THIS->time_hdl = 0;
            return TRUE;
        }
    }

    return FALSE;
}


static void dev_conn_task(void *priv)
{
    wifi_set_connect_sta_block(0);
    wifi_enter_sta_mode(__THIS->conn_ssid, __THIS->conn_pwd);
}


static void build_dev_conn(void)
{
    thread_fork("dev_conn_task", 10, 1024, 0, NULL, dev_conn_task, NULL);
}


static void delete_dev_conn(void)
{
    u8 mac[6];
    static u8 ssid[33] = {0}, pwd[64] = {0};

    if (__THIS->server_mode == AP_MODE) {
        wifi_get_mac(mac);
        sprintf((char *)ssid, PRODUCT_SER_AP_PREFIX"%02x%02x%02x%02x%02x%02x", \
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        sprintf((char *)pwd, PRODUCT_SER_AP_PWD);
        wifi_enter_ap_mode(ssid, pwd);
    } else {
        wifi_enter_sta_mode("jijij", "jlkhk");
    }
}


static void host_data_handle_task(void *priv)
{
    s32 len;
    u16 *crc_value;
    u8 rscorr, *data, *read_addr, *l_data, l_idx, *recv_buf, *msg, *str, rscorr_str[4] = {0};
    u32 crc_len, recv_size, total_size, l_len;
    struct comm_head *head;
    json_object *new_obj, *rscorr_obj;

    recv_buf = zalloc(RECV_BUFFER_SIZE);
    head = (struct comm_head *)recv_buf;
    data = recv_buf + sizeof(struct comm_head);

    for (;;) {
        if (!(comm_ops()->online())) {
            os_time_dly(10);
            continue;
        }

        len = host_data_read(recv_buf, MAXP_SIZE_CDC_BULKOUT);
        if (len < sizeof(struct comm_head)) {
            continue;
        }

        if (head->mark[0] != 'J' || head->mark[1] != 'L') {
            continue;
        }

        recv_size  = len;
        read_addr  = recv_buf + len;
        total_size = sizeof(struct comm_head) + head->len + sizeof(u16);
        while (recv_size < total_size) {
            if ((len = host_data_read(read_addr, MAXP_SIZE_CDC_BULKOUT)) <= 0) {
                break;
            }
            recv_size += len;
            read_addr += len;
        }

        if (recv_size < total_size) {
            continue;
        }

        //put_buf(recv_buf, total_size);

        crc_len = sizeof(struct comm_head) + head->len;
        crc_value = (u16 *)(recv_buf + crc_len);
        if (CRC16(head, crc_len) != *crc_value) {
            continue;
        }

        if (!__THIS->host_sock) {
            msg = zalloc(head->len);
            memcpy(msg, data, head->len);
            new_obj = json_tokener_parse(msg);
            rscorr_obj = json_object_object_get(new_obj, "rscorr");
            sprintf(rscorr_str, "%d", ERR_NET_DISCONNECT);
            json_object_string_set(rscorr_obj, rscorr_str, strlen(rscorr_str));
            str = json_object_get_string(new_obj);
            printf("str = %s\n", str);
            data_respond(0, DATA_TYPE_OPCODE, str, strlen(str));
            json_object_put(new_obj);
        } else {
            sock_sendto(__THIS->host_sock, recv_buf, total_size, 0, (struct sockaddr *)&__THIS->host_dest, sizeof(struct sockaddr_in));
        }
    }
}


static void client_data_handle_task(void *priv)
{
    int err, recv_len;
    char recv_buf[1024];

    if (!(__THIS->host_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("sock_reg fail.\n");
        return;
    }

    __THIS->host_dest.sin_family = AF_INET;
    __THIS->host_dest.sin_addr.s_addr = inet_addr(inet_ntoa(__THIS->remote_addr.sin_addr));
    __THIS->host_dest.sin_port = htons(HOST_PORT);

    if (sock_connect(__THIS->host_sock, (struct sockaddr *)&__THIS->host_dest, sizeof(struct sockaddr_in))) {
        product_err("sock_connect fail.\n");
        sock_unreg(__THIS->host_sock);
        __THIS->host_sock = NULL;
        delete_dev_conn();
        return;
    } else {
        product_info("sock_connect succ.\n");
        product_online_tips();
    }

    for (;;) {
        if ((recv_len = sock_recvfrom(__THIS->host_sock, recv_buf, sizeof(recv_buf), 0, NULL, NULL)) > 0) {
            //put_buf(recv_buf, recv_len);
            host_data_write(recv_buf, recv_len);
        } else {
            product_err("sock_recvfrom err!");
            break;
        }
    }
}


static void build_data_conn(void)
{
    if (thread_fork("client_data_handle_task", 20, 1024, 0, NULL, client_data_handle_task, NULL) != OS_NO_ERR) {
        product_err("thread fork fail\n");
    }
}


static void delete_data_conn(void)
{
    sock_unreg(__THIS->host_sock);
    __THIS->host_sock = NULL;
}


static void camera_data_handle_task(void *priv)
{
    int recv_len;
    char recv_buf[1024];
    struct sockaddr_in dest = {0};
    struct uvc_video_data {
        int pid;
        u8 *buf;
        u32 len;
        u8 *reqbuf;
        u16 reqlen;
        u16 send_len;
        OS_SEM wait_sem;
        OS_SEM req_sem;
        u8 cur_fmt_id;
        u8 cur_frame_id;
        u8 init;
        u8 end;
        u8 err;
        volatile u8 exit;
        volatile u8 complete;
        spinlock_t lock;
        u32 cur_fps;
        u32 offset;
        u16 width;
        u16 height;
    } *info;
    info = *((u32 *)priv);
    u32(*func)(void *, u8 *, u32) = *(((u32 *)priv) + 1);
    struct net_cmd_type cmd;
    u32 *params = (u32 *)cmd.params;
    char *camera_data = malloc(50 * 1024);
    u32 read_size, recv_size, total_size, remain_size;

    if (!(__THIS->camera_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("sock_reg fail.\n");
        return;
    }

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(inet_ntoa(__THIS->remote_addr.sin_addr));
    dest.sin_port = htons(CAMERA_PORT);

    if (sock_connect(__THIS->camera_sock, (struct sockaddr *)&dest, sizeof(struct sockaddr_in))) {
        product_err("camera sock_connect fail.\n");
        sock_unreg(__THIS->camera_sock);
        __THIS->camera_sock = NULL;
        return;
    } else {
        product_info("camera_sock_connect succ.\n");
    }

    strcpy(cmd.cmd, NET_CAMERA_OPEN);
    params[0] = info->width;
    params[1] = info->height;
    params[2] = PRODUCT_UVC_FPS;//info->cur_fps;

    if (sock_send(__THIS->camera_sock, &cmd, sizeof(struct net_cmd_type), 0) != sizeof(struct net_cmd_type)) {
        product_err("net camera open err\n");
        return;
    }

    for (;;) {
        if (sock_recvfrom(__THIS->camera_sock, &cmd, sizeof(struct net_cmd_type), 0, NULL, NULL) == sizeof(struct net_cmd_type)) {
            if (strcmp(cmd.cmd, NET_CAMERA_FRAME)) {
                continue;
            }
            recv_size = 0;
            total_size = *((u32 *)cmd.params);
            remain_size = total_size;
            while (remain_size) {
                read_size = remain_size > 1024 ? 1024 : remain_size;
                if ((recv_len = sock_recvfrom(__THIS->camera_sock, camera_data + recv_size, read_size, 0, NULL, NULL)) > 0) {
                    recv_size += recv_len;
                    remain_size = total_size - recv_size;
                } else {
                    break;
                }
            }

            if (!remain_size) {
#if 1
                func(info, camera_data, total_size);
#else
#include "fs/fs.h"
                FILE *fp;
                char *path[32];
                static u32 idx = 0;

                sprintf(path, "storage/sd0/C/myjpg_%d.JPG", idx++);
                if ((fp = fopen(path, "w+"))) {
                    fwrite(camera_data, 1, total_size, fp);
                    fclose(fp);
                }
#endif
            }
        }

        if (!os_sem_accept(&__THIS->camera_sem)) {
            os_sem_del(&__THIS->camera_sem, 0);
            free(camera_data);
            __THIS->camera_pid = 0;
            return;
        }
    }
}


u8 build_camera_data_conn(void *func, void *priv)
{
    static u32 msg[2];
    msg[0] = (u32)priv;
    msg[1] = (u32)func;

    if (__THIS->camera_pid) {
        return TRUE;
    }

    if (__THIS->wifi_conn == FALSE) {
        return TRUE;
    }

    os_sem_create(&__THIS->camera_sem, 0);
    if (thread_fork("camera_data_handle_task", 10, 1024, 0, &__THIS->camera_pid, camera_data_handle_task, msg) != OS_NO_ERR) {
        product_err("thread fork fail\n");
        return FALSE;
    }
    return TRUE;
}


void delete_camera_data_conn(void)
{
    struct net_cmd_type cmd;
    if (__THIS->camera_pid) {
        strcpy(cmd.cmd, NET_CAMERA_CLOSE);
        sock_send(__THIS->camera_sock, &cmd, sizeof(struct net_cmd_type), 0);
        sock_unreg(__THIS->camera_sock);
        __THIS->camera_sock = NULL;
        os_sem_post(&__THIS->camera_sem);
    }
}


#ifdef CONFIG_AUDIO_ENABLE

static void audio_ctl_handle_task(void *priv)
{
    struct sockaddr_in dest = {0};

    if (!(__THIS->audio_ctl_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("sock_reg fail.\n");
        return;
    }

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(inet_ntoa(__THIS->remote_addr.sin_addr));
    dest.sin_port = htons(AUDIO_CTL_PORT);

    if (sock_connect(__THIS->audio_ctl_sock, (struct sockaddr *)&dest, sizeof(struct sockaddr_in))) {
        product_err("audio sock_connect fail.\n");
        sock_unreg(__THIS->audio_ctl_sock);
        __THIS->audio_ctl_sock = NULL;
        return;
    } else {
        product_info("audio_ctl_sock_connect succ.\n");
    }
}


static void audio_data_handle_task(void *priv)
{
    int recv_len;
    char recv_buf[1024];
    struct sockaddr_in dest = {0};

    if (!(__THIS->audio_data_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("sock_reg fail.\n");
        return;
    }

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(inet_ntoa(__THIS->remote_addr.sin_addr));
    dest.sin_port = htons(AUDIO_DATA_PORT);

    if (sock_connect(__THIS->audio_data_sock, (struct sockaddr *)&dest, sizeof(struct sockaddr_in))) {
        product_err("audio sock_connect fail.\n");
        sock_unreg(__THIS->audio_data_sock);
        __THIS->audio_data_sock = NULL;
        return;
    } else {
        product_info("audio_data_sock_connect succ.\n");
    }

    for (;;) {
        if ((recv_len = sock_recvfrom(__THIS->audio_data_sock, recv_buf, sizeof(recv_buf), 0, NULL, NULL)) > 0) {
            cbuf_write(&__THIS->mic_cbuf, recv_buf, recv_len);
        }

        if (!os_sem_accept(&__THIS->audio_data_sem)) {
            os_sem_del(&__THIS->audio_data_sem, 0);
            __THIS->audio_data_pid = 0;
            return;
        }
    }
}


u8 build_audio_data_conn(void)
{
    if (__THIS->audio_ctl_pid && __THIS->audio_data_pid) {
        return TRUE;
    }

    if (__THIS->wifi_conn == FALSE) {
        return TRUE;
    }

    os_sem_create(&__THIS->audio_data_sem, 0);
    if (thread_fork("audio_ctl_handle_task", 10, 1024, 0, &__THIS->audio_ctl_pid, audio_ctl_handle_task, NULL) != OS_NO_ERR) {
        product_err("thread fork fail\n");
        return FALSE;
    }

    if (thread_fork("audio_data_handle_task", 10, 1024, 0, &__THIS->audio_data_pid, audio_data_handle_task, NULL) != OS_NO_ERR) {
        product_err("thread fork fail\n");
        return FALSE;
    }
    return TRUE;
}


void delete_audio_data_conn(void)
{
    if (__THIS->audio_ctl_pid) {
        sock_unreg(__THIS->audio_ctl_sock);
        __THIS->audio_ctl_sock = NULL;
    }

    if (__THIS->audio_data_pid) {
        sock_unreg(__THIS->audio_data_sock);
        __THIS->audio_data_sock = NULL;
        os_sem_post(&__THIS->audio_data_sem);
    }
}


static void spk_data_rx_task(void *priv)
{
    u32 rlen;
    u8 *frame = malloc(SPK_FRAME_SIZE);
    ASSERT(frame);

    for (;;) {
        if (!__THIS->audio_data_sock) {
            os_time_dly(2);
            continue;
        }

        if ((rlen = cbuf_get_data_size(&__THIS->spk_cbuf)) > 0) {
            rlen = (rlen > SPK_FRAME_SIZE) ? SPK_FRAME_SIZE : rlen;
            if ((rlen = cbuf_read(&__THIS->spk_cbuf, frame, rlen)) > 0) {
                sock_send(__THIS->audio_data_sock, frame, rlen, 0);
            }
        } else {
            os_sem_pend(&__THIS->spk_sem, 0);
        }
    }
}


static void spk_data_tx(void *priv, u8 *data, u32 size)
{
    if (!__THIS->audio_data_sock) {
        return;
    }

    cbuf_write(&__THIS->spk_cbuf, data, size);
    os_sem_set(&__THIS->spk_sem, 0);
    os_sem_post(&__THIS->spk_sem);

}


static void spk_tramsmit_init(void)
{
    __THIS->spk_buf_size = SPK_AUDIO_RATE * 4 * SPK_CHANNEL / 1000 * 30;
    __THIS->spk_buf = malloc(__THIS->spk_buf_size);
    ASSERT(__THIS->spk_buf);

    cbuf_init(&__THIS->spk_cbuf, __THIS->spk_buf, __THIS->spk_buf_size);
    os_sem_create(&__THIS->spk_sem, 0);
    thread_fork("spk_data_rx_task", 12, 1024, 0, NULL, spk_data_rx_task, NULL);
    //set_uac_speaker_rx_handler(get_product_id(), NULL, spk_data_tx);
}


static int mic_data_tx(void *priv, void *data, int len)
{
    u32 rlen;
    if ((rlen = cbuf_get_data_size(&__THIS->mic_cbuf)) > 0) {
        rlen = (rlen > len) ? len : rlen;
        if ((rlen = cbuf_read(&__THIS->mic_cbuf, data, rlen)) > 0) {
            return rlen;
        }
    }
    return 0;
}


static void mic_tramsmit_init(void)
{
    __THIS->mic_buf_size = 10 * 1024;
    __THIS->mic_buf = malloc(__THIS->mic_buf_size);
    ASSERT(__THIS->mic_buf);

    cbuf_init(&__THIS->mic_cbuf, __THIS->mic_buf, __THIS->mic_buf_size);
    //set_uac_mic_tx_handler(get_product_id(), NULL, mic_data_tx);
}


void audio_ctl_tramsmit(char *msg, u32 *param, u32 num)
{
    struct net_cmd_type cmd = {0};
    u32 *params = (u32 *)cmd.params;

    if (!__THIS->audio_ctl_sock) {
        return;
    }

    strcpy(&cmd.cmd, msg);
    if (param && num) {
        memcpy(params, param, sizeof(u32) * 2);
    }
    sock_send(__THIS->audio_ctl_sock, &cmd, sizeof(struct net_cmd_type), 0);
}


int product_usb_audio_event_handler(struct device_event *event)
{
    int value = event->value;
    const usb_dev usb_id = (const usb_dev)event->arg;

    switch (event->event) {
    case USB_AUDIO_PLAY_OPEN:
        break;
    case USB_AUDIO_PLAY_CLOSE:
        break;
    case USB_AUDIO_SET_PLAY_VOL:
        break;
    case USB_AUDIO_MIC_OPEN:
        audio_ctl_tramsmit("NET_MIC_OPEN", NULL, 0);
        break;
    case USB_AUDIO_MIC_CLOSE:
        audio_ctl_tramsmit("NET_MIC_CLOSE", NULL, 0);
        break;
    case USB_AUDIO_SET_MIC_VOL:
        break;
    default:
        return FALSE;
    }

    return TRUE;
}


#endif


static void wifi_rx_frame_cb(void *rxwi, void *header, void *data, u32 len, void *reserve)
{
    u32 size;
    u8 *buf, offset;
    struct product_conn *conn;
    struct wifi_store_info *info = get_cur_wifi_info();

    offset = (info->mode == AP_MODE) ? 27 : 51;
    size = len - offset;
    //if (size < sizeof(struct product_conn)) return;
    buf = ((u8 *)data) + offset;
    conn = buf;

    if (!strcmp(conn->str, "product_resp") && __THIS->conn_flag == FALSE) {
        __THIS->conn_flag = TRUE;
        product_info("str = %s, mode = %d, ssid = %s, pwd = %s\n", conn->str, conn->mode, conn->ssid, conn->pwd);
        strcpy(__THIS->conn_ssid, conn->ssid);
        strcpy(__THIS->conn_pwd, conn->pwd);
        broadcast_ctl(FALSE);
    }
}


static int wifi_event_callback(void *network_ctx, enum WIFI_EVENT event)
{
    u8 mac[6];
    struct wifi_store_info wifi_default_mode_parm = {0};
    static u8 ssid[33] = {0}, pwd[64] = {0};

    switch (event) {
    case WIFI_EVENT_MODULE_INIT:
        puts("|network_user_callback->WIFI_EVENT_MODULE_INIT\n");
        wifi_default_mode_parm.mode = __THIS->server_mode;
        if (__THIS->server_mode == AP_MODE) {
            wl_set_wifi_channel(PRODUCT_SER_AP_CH);
            init_net_device_mac_addr((char *)mac, 1);
            sprintf((char *)ssid, PRODUCT_SER_AP_PREFIX"%02x%02x%02x%02x%02x%02x", \
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            sprintf((char *)pwd, PRODUCT_SER_AP_PWD);
        }

        if (wifi_default_mode_parm.mode <= AP_MODE) {
            strncpy((char *)wifi_default_mode_parm.ssid[wifi_default_mode_parm.mode - STA_MODE], ssid, sizeof(wifi_default_mode_parm.ssid[wifi_default_mode_parm.mode - STA_MODE]) - 1);
            strncpy((char *)wifi_default_mode_parm.pwd[wifi_default_mode_parm.mode - STA_MODE], pwd, sizeof(wifi_default_mode_parm.pwd[wifi_default_mode_parm.mode - STA_MODE]) - 1);
        }
        wifi_set_default_mode(&wifi_default_mode_parm, 1, 0);//wifi_default_mode_parm.mode == STA_MODE);
        break;
    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");
        broadcast_ctl(TRUE);
        break;
    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;
    case WIFI_EVENT_AP_START:
        printf("|network_user_callback->WIFI_EVENT_AP_START,CH=%d\n", wifi_get_channel());
        break;
    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");
        __THIS->wifi_conn = FALSE;
        broadcast_ctl(TRUE);
        break;
    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        break;
    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;
    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
        break;
    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
        broadcast_ctl(TRUE);
        __THIS->wifi_conn = FALSE;
        break;
    case WIFI_EVENT_STA_SCAN_COMPLETED:
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        break;
    case WIFI_EVENT_STA_CONNECT_SUCC:
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", wifi_get_channel());
        __THIS->wifi_conn = TRUE;
#ifdef PRODUCT_SER_STATIC_IP_ENABLE
        ip4_addr_t addr;
        struct lan_setting info;

        ip4addr_aton(SER_IP, &addr);
        memcpy(&info.WIRELESS_IP_ADDR0, &addr, sizeof(ip4_addr_t));

        ip4addr_aton(SER_NETMASK, &addr);
        memcpy(&info.WIRELESS_NETMASK0, &addr, sizeof(ip4_addr_t));

        ip4addr_aton(SER_GATEWAY, &addr);
        memcpy(&info.WIRELESS_GATEWAY0, &addr, sizeof(ip4_addr_t));

        net_set_lan_info(&info);
        return TRUE;
#endif
        break;
    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;
    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;
    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        broadcast_ctl(TRUE);
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        breathe_ctl(TRUE);
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        broadcast_ctl(TRUE);
        break;
    case WIFI_EVENT_P2P_START:
        puts("|network_user_callback->WIFI_EVENT_P2P_START\n");
        break;
    case WIFI_EVENT_P2P_STOP:
        puts("|network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;
    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;
    case WIFI_EVENT_SMP_CFG_START:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_START\n");
        break;
    case WIFI_EVENT_SMP_CFG_STOP:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_STOP\n");
        break;
    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        break;
    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        break;
    case WIFI_EVENT_PM_SUSPEND:
        puts("|network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        break;
    case WIFI_EVENT_PM_RESUME:
        puts("|network_user_callback->WIFI_EVENT_PM_RESUME\n");
        break;
    case WIFI_EVENT_AP_ON_ASSOC:
        puts("|network_user_callback->WIFI_EVENT_AP_ON_ASSOC\n");
        __THIS->wifi_conn = TRUE;
        breathe_ctl(TRUE);
        break;
    case WIFI_EVENT_AP_ON_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_AP_ON_DISCONNECTED\n");
        __THIS->wifi_conn = FALSE;
        broadcast_ctl(TRUE);
        break;
    default:
        break;
    }

    return 0;
}


u8 product_net_main(void)
{
    u8 head[] = {
        0xc6, 0x00, 0x00, 0x04, 0xB0, 0x00, 0x04, 0x80, 0x35, 0x01, 0xB6, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x88, 0x88, /*dst*/ 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,/*src*/0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,/*BSSID*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*Seq,Frag num*/0x88, 0x88,
    };
    product_info("\r\n================ENTER PRODUCT BOX MODE================\r\n");
    while (!(comm_ops()->online())) {
        os_time_dly(10);
    }

    if (!__THIS) {
        __THIS = zalloc(sizeof(struct product_net_type));
        ASSERT(__THIS);

        __THIS->wifi_send_pkg = zalloc(1564);
        ASSERT(__THIS->wifi_send_pkg);

        memcpy(__THIS->wifi_send_pkg, head, sizeof(head));
        __THIS->server_mode = STA_MODE;
    }

    comm_ops()->init();
    thread_fork("host_data_handle_task", 10, 1024, 0, NULL, host_data_handle_task, NULL);

    os_sem_create(&__THIS->asem, 0);
    os_sem_create(&__THIS->bsem, 0);
    wifi_set_frame_cb(wifi_rx_frame_cb, NULL);
    if (__THIS->server_mode == STA_MODE) {
        wifi_set_sta_connect_timeout(20);
    }
    wifi_set_event_callback(wifi_event_callback);
    wifi_on();

    spk_tramsmit_init();
    mic_tramsmit_init();
    return TRUE;
}

void audio_transmit_init(u8 id)
{
    set_uac_mic_tx_handler(id, NULL, mic_data_tx);
    set_uac_speaker_rx_handler(id, NULL, spk_data_tx);
}
#else

#include "server/audio_server.h"
#include "server/server_core.h"

#define SPK_FRAME_SIZE		  (2 * 1024)

static struct product_net_type {
    u8 uvc_id;
    void *camera_sock;
    void *audio_ctl_sock;
    void *audio_data_sock;
    u32 camera_width, camera_height, camera_fps;
    void *spk_hdl;
    void *spk_buf;
    u32 spk_buf_size;
    cbuffer_t spk_cbuf;
    u8 spk_ready;
    OS_SEM spk_sem;
    u8 bindex;
    u8 mic_enable;
    void *mic_buf;
    void *mic_hdl;
    u16 offset;
} *__THIS = NULL;


void uvc_set_id(u8 id)
{
    __THIS->uvc_id = id;
}


void product_net_camera_info(u32 *width, u32 *height, u32 *fps)
{
    *width  = __THIS->camera_width;
    *height = __THIS->camera_height;
    *fps    = __THIS->camera_fps;
}


static int camera_transmit(void *hdr, u8 *data, u32 size)
{
#define JPEG_HEAD 0xE0FFD8FF
#define JPEG_HEAD1 0xC0FFD8FF
    u32 *type = (u32 *)data;
    struct net_cmd_type cmd = {0};
    u32 *params = (u32 *)cmd.params;

    if (!__THIS->camera_sock) {
        return 0;
    }

    strcpy(cmd.cmd, NET_CAMERA_FRAME);
    params[0] = size;

    if (*type == JPEG_HEAD || *type == JPEG_HEAD1) {
        //video
        sock_send(__THIS->camera_sock, &cmd, sizeof(struct net_cmd_type), 0);
        sock_send(__THIS->camera_sock, data, size, 0);
        putchar('V');
    } else {
        //audio
        putchar('A');
    }

    return size;
}


static void camera_conn_task(void *priv)
{
    u32 opt = 1;
    void *s_sock;
    struct net_cmd_type cmd = {0};
    u32 *params = (u32 *)cmd.params;
    struct sockaddr_in local_addr = {0};
    struct sockaddr_in remote_addr = {0};
    socklen_t len = sizeof(struct sockaddr_in);

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(CAMERA_PORT);

    if (!(s_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("%s build socket fail\n",  __FILE__);
        return;
    }

    if (sock_setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        product_err("%s sock_setsockopt fail\n", __FILE__);
        return;
    }

    if (sock_bind(s_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) {
        product_err("%s sock_bind fail\n", __FILE__);
        return;
    }

    if (sock_listen(s_sock, 0x2)) {
        product_err("%s sock_listen fail\n", __FILE__);
        return;
    }

    for (;;) {
        if (!(__THIS->camera_sock = sock_accept(s_sock, (struct sockaddr *)&remote_addr, &len, NULL, NULL))) {
            product_err("%s socket_accept fail\n",  __FILE__);
        } else {
            product_info("camera socket conn succ\n");
            for (;;) {
                if (sock_recvfrom(__THIS->camera_sock, &cmd, sizeof(struct net_cmd_type), 0, NULL, NULL) == sizeof(struct net_cmd_type)) {
                    if (!strcmp(cmd.cmd, NET_CAMERA_OPEN)) {
                        __THIS->camera_width  = params[0];
                        __THIS->camera_height = params[1];
                        __THIS->camera_fps    = params[2];
                        product_info("net camera : w = %d, h = %d, fps = %d\n", params[0], params[1], params[2]);
                        set_video_rt_cb(camera_transmit, NULL);
                        user_video_rec0_open(__THIS->uvc_id);
                    } else if (!strcmp(cmd.cmd, NET_CAMERA_CLOSE)) {
                        set_video_rt_cb(NULL, NULL);
                        user_video_rec0_close();
                        sock_unreg(__THIS->camera_sock);
                        __THIS->camera_sock = NULL;
                        break;
                    }
                }
            }
        }
    }
}


int product_user_video_callback(u8 type, u8 *buf, u32 size)
{
#define jl_ntohl(x) (u32)((((u32)(x))>>24) | ((((u32)(x))>>8)&0xff00) | (((u32)(x))<<24) | ((((u32)(x))&0xff00)<<8))
#define JL_ENDF     	jl_ntohl(0x56185719)
#define JPEG_HEAD 		0xE0FFD8FF
#define JL_000DC		jl_ntohl(0x30306463)
#define JL_001WB		jl_ntohl(0x30317762)

    u32 *len_ptr;
    u32 *head = (u32 *)buf;

    if (size <= 8 || *head == JL_ENDF) {
        return size;
    }

    if (type == VIDEO_REC_JPEG_TYPE_VIDEO) {
        if ((*head == JL_000DC && *(head + 2) == JPEG_HEAD) || *(head + 2) == JPEG_HEAD) {
            buf += 8;
            size -= 8;
        }

        len_ptr = (u32 *)buf;
        if (*(len_ptr + 2) == JPEG_HEAD) {
            buf += 8;
            size -= 8;
        }
        camera_transmit(NULL, buf, size);
    } else if (type == VIDEO_REC_PCM_TYPE_AUDIO) {
        if (__THIS->audio_data_sock) {
            sock_send(__THIS->audio_data_sock, buf, size, 0);
        }
    }

    return size;
}


void camera_conn_init(void)
{
    if (!__THIS) {
        __THIS = zalloc(sizeof(struct product_net_type));
        ASSERT(__THIS);
    }

    thread_fork("camera_conn_task", 10, 512, 0, NULL, camera_conn_task, NULL);
}


#ifdef CONFIG_AUDIO_ENABLE

static int spk_vfs_fread(void *file, void *data, u32 len)
{
    int rlen = 0;

    while (!rlen) {
        rlen = cbuf_get_data_size(&__THIS->spk_cbuf);
        if (!rlen) {
            if (!__THIS->spk_ready) {
                return -2;
            }
            os_sem_pend(&__THIS->spk_sem, 0);
        }
    }

    rlen = (rlen > len) ? len : rlen;
    return cbuf_read(&__THIS->spk_cbuf, data, rlen);
}


static int spk_vfs_fclose(void *file)
{
    return 0;
}


static int spk_vfs_flen(void *file)
{
    return 0;
}


static const struct audio_vfs_ops spk_vfs_ops = {
    .fread  = spk_vfs_fread,
    .fclose = spk_vfs_fclose,
    .flen   = spk_vfs_flen,
};


static int spk_resample(void *priv, u32 data_size, u16 *in_rate, u16 *out_rate)
{
    data_size = cbuf_get_data_size(&__THIS->spk_cbuf);

    int sample_rate = 48000;
    if (data_size >= (__THIS->spk_buf_size * 3 / 5)) {
        sample_rate += (sample_rate * 20 / 10000);  //96
    }

    if (data_size <= (__THIS->spk_buf_size * 2 / 5)) {
        sample_rate -= (sample_rate * 20 / 10000);
    }

    *in_rate = sample_rate;

    return 0;
}


static int spk_open(int value)
{
    u8 channel;
    u32 sample_rate;
    union audio_req req = {0};

    if (!(__THIS->spk_hdl = server_open("audio_server", "dec"))) {
        product_err("%s, audio server open err\n", __FUNCTION__);
        return -1;
    }

    os_sem_create(&__THIS->spk_sem, 0);
    channel = value >> 24;
    sample_rate = value & 0xffffff;
    product_info("%s, ch = %d, sample_rate = %d\n", __FUNCTION__, channel, sample_rate);

    __THIS->spk_buf_size = sample_rate * 4 * channel / 1000 * 30;
    __THIS->spk_buf = malloc(__THIS->spk_buf_size);
    ASSERT(__THIS->spk_buf);
    cbuf_init(&__THIS->spk_cbuf, __THIS->spk_buf, __THIS->spk_buf_size);

    req.dec.cmd             = AUDIO_DEC_OPEN;
    req.dec.volume          = 100;
    req.dec.output_buf_len  = __THIS->spk_buf_size;
    req.dec.channel         = channel;
    req.dec.sample_rate     = sample_rate;
    req.dec.priority        = 1;
    req.dec.vfs_ops         = &spk_vfs_ops;
    req.dec.dec_type        = "pcm";
    req.dec.sample_source   = "dac";
    req.dec.attr            = AUDIO_ATTR_REAL_TIME;
    req.dec.dec_sync        = spk_resample;

    if (server_request(__THIS->spk_hdl, AUDIO_REQ_DEC, &req)) {
        goto _spk_exit_;
    }

    req.dec.cmd = AUDIO_DEC_START;
    req.dec.attr = AUDIO_ATTR_NO_WAIT_READY;
    if (server_request(__THIS->spk_hdl, AUDIO_REQ_DEC, &req)) {
        goto _spk_exit_;
    }

    __THIS->spk_ready = 1;

    return 0;

_spk_exit_:
    if (__THIS->spk_hdl) {
        server_close(__THIS->spk_hdl);
        __THIS->spk_hdl = NULL;
    }

    return -1;
}


static int spk_close(void)
{
    union audio_req req = {0};

    if (__THIS->spk_hdl) {
        __THIS->spk_ready = 0;
        os_sem_post(&__THIS->spk_sem);
        req.dec.cmd = AUDIO_DEC_STOP;
        server_request(__THIS->spk_hdl, AUDIO_REQ_DEC, &req);
        server_close(__THIS->spk_hdl);
        __THIS->spk_hdl = NULL;
        os_sem_del(&__THIS->spk_sem, 0);
    }

    if (__THIS->spk_buf) {
        free(__THIS->spk_buf);
    }

    return 0;
}


static int mic_open(int value, u8 res)
{
    u32 frame_len;
    u8 vol = 100;
    u8 channel = value >> 24;
    u32 sample_rate = value & 0xffffff;
    static struct audio_format f = {0};
    static struct video_reqbufs breq = {0};

#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    __THIS->mic_hdl = dev_open("audio", (void *)AUDIO_TYPE_ENC_PLNK0);
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK1
    __THIS->mic_hdl = dev_open("audio", (void *)AUDIO_TYPE_ENC_PLNK1);
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS0
    __THIS->mic_hdl = dev_open("audio", (void *)AUDIO_TYPE_ENC_IIS0);
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS1
    __THIS->mic_hdl = dev_open("audio", (void *)AUDIO_TYPE_ENC_IIS1);
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_LINEIN
    __THIS->mic_hdl = dev_open("audio", (void *)AUDIO_TYPE_ENC_LINEIN);
#else
    __THIS->mic_hdl = dev_open("audio", (void *)AUDIO_TYPE_ENC_MIC);
#endif

    if (!__THIS->mic_hdl) {
        product_err("mic audio open err\n");
        return -EFAULT;
    }

    frame_len = ((sample_rate * res / 8 * channel) / 1000);
    frame_len += (sample_rate % 1000 ? (res / 8) * channel : 0);

    __THIS->mic_buf = malloc(frame_len * 3 * 4);
    ASSERT(__THIS->mic_buf);

    breq.buf  = __THIS->mic_buf;
    breq.size = frame_len * 3 * 4;
    if (dev_ioctl(__THIS->mic_hdl, AUDIOC_REQBUFS, (unsigned int)&breq)) {
        product_err("mic set buf err\n");
        goto _mic_exit_;
    }

#ifdef CONFIG_ALL_ADC_CHANNEL_OPEN_ENABLE
    f.channel_bit_map = BIT(CONFIG_UAC_MIC_ADC_CHANNEL);
#endif
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    f.sample_source = "plnk0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK1
    f.sample_source = "plnk1";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS0
    f.sample_source = "iis0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS1
    f.sample_source = "iis1";
#else
    f.sample_source = "mic";
#endif
    f.format = "pcm";
    f.volume = 100;
    f.channel = channel;
    f.sample_rate = sample_rate;
    f.frame_len = frame_len * 3;

    product_info("mic channel     : %d\n", f.channel);
    product_info("mic sample_rate : %d, %d\n", f.sample_rate, sample_rate);
    product_info("mic volume      : %d\n", f.volume);
    product_info("mic frame_len   : %d\n", f.frame_len);

    printf("%s, line = %d\n", __FUNCTION__, __LINE__);
    if (dev_ioctl(__THIS->mic_hdl, AUDIOC_SET_FMT, (unsigned int)&f)) {
        product_err("mic set fmt err\n");
        goto _mic_exit_;
    }

    printf("%s, line = %d\n", __FUNCTION__, __LINE__);
    if (dev_ioctl(__THIS->mic_hdl, AUDIOC_STREAM_ON, (u32)&__THIS->bindex)) {
        product_err("mic stream on err\n");
        goto _mic_exit_;
    }

    __THIS->offset = 0;
    product_info("mic stream on succ\n");

    return 0;

_mic_exit_:
    if (__THIS->mic_hdl) {
        dev_close(__THIS->mic_hdl);
    }
    if (__THIS->mic_buf) {
        free(__THIS->mic_buf);
        __THIS->mic_buf = NULL;
    }

    return -1;
}


static int mic_close(void)
{

}


static void mic_data_rx_task(void *priv)
{
    int err = 0;
    struct video_buffer b = {0};
    u8 data[288];
    u32 len = 288;

    for (;;) {
        if (!__THIS->mic_enable) {
            os_time_dly(2);
            continue;
        }

        b.noblock = 1;
        b.timeout = 0;
        b.index = __THIS->bindex;

        if (__THIS->offset == 0) {
            err = dev_ioctl(__THIS->mic_hdl, AUDIOC_DQBUF, (u32)&b);
            if (err || !b.len) {
                continue;
            }
        }

        memcpy(data, (u8 *)b.baddr + __THIS->offset, len);
        __THIS->offset += len;

        if (__THIS->audio_data_sock) {
            sock_send(__THIS->audio_data_sock, data, len, 0);
        }

        if (__THIS->offset == b.len) {
            dev_ioctl(__THIS->mic_hdl, AUDIOC_QBUF, (u32)&b);
            __THIS->offset = 0;
        }
    }
}


static void audio_ctl_task(void *priv)
{
    u32 opt = 1;
    void *s_sock;
    struct net_cmd_type cmd = {0};
    u32 *params = (u32 *)cmd.params;
    struct sockaddr_in local_addr = {0};
    struct sockaddr_in remote_addr = {0};
    socklen_t len = sizeof(struct sockaddr_in);

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(AUDIO_CTL_PORT);

    if (!(s_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("%s build socket fail\n",  __FILE__);
        return;
    }

    if (sock_setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        product_err("%s sock_setsockopt fail\n", __FILE__);
        return;
    }

    if (sock_bind(s_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) {
        product_err("%s sock_bind fail\n", __FILE__);
        return;
    }

    if (sock_listen(s_sock, 0x2)) {
        product_err("%s sock_listen fail\n", __FILE__);
        return;
    }

    for (;;) {
        if (!(__THIS->audio_ctl_sock = sock_accept(s_sock, (struct sockaddr *)&remote_addr, &len, NULL, NULL))) {
            product_err("%s socket_accept fail\n",  __FILE__);
        } else {
            product_info("audio ctl socket conn succ\n");
            for (;;) {
                if (sock_recvfrom(__THIS->audio_ctl_sock, &cmd, sizeof(struct net_cmd_type), 0, NULL, NULL) == sizeof(struct net_cmd_type)) {
                    product_info("%s,%s\n", __FUNCTION__, cmd.cmd);
                    if (!strcmp(cmd.cmd, NET_SPEAKER_OPEN)) {
                    } else if (!strcmp(cmd.cmd, NET_SPEAKER_CLOSE)) {
                    } else if (!strcmp(cmd.cmd, NET_SPEAKER_VOL)) {
                    } else if (!strcmp(cmd.cmd, NET_MIC_OPEN)) {
                        __THIS->mic_enable = TRUE;
                    } else if (!strcmp(cmd.cmd, NET_MIC_CLOSE)) {
                        __THIS->mic_enable = FALSE;
                    } else if (!strcmp(cmd.cmd, NET_MIC_VOL)) {
                    }
                }
            }
        }
    }
}


static void audio_data_task(void *priv)
{
    u32 opt = 1;
    void *s_sock;
    struct net_cmd_type cmd = {0};
    u32 *params = (u32 *)cmd.params;
    struct sockaddr_in local_addr = {0};
    struct sockaddr_in remote_addr = {0};
    socklen_t len = sizeof(struct sockaddr_in);
    u32 read_size, recv_size, total_size, remain_size;
    u8 *spk_frame = zalloc(SPK_FRAME_SIZE);
    int recv_len;

    ASSERT(spk_frame);

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(AUDIO_DATA_PORT);

    if (!(s_sock = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL))) {
        product_err("%s build socket fail\n",  __FILE__);
        return;
    }

    if (sock_setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        product_err("%s sock_setsockopt fail\n", __FILE__);
        return;
    }

    if (sock_bind(s_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) {
        product_err("%s sock_bind fail\n", __FILE__);
        return;
    }

    if (sock_listen(s_sock, 0x2)) {
        product_err("%s sock_listen fail\n", __FILE__);
        return;
    }

    for (;;) {
        if (!(__THIS->audio_data_sock = sock_accept(s_sock, (struct sockaddr *)&remote_addr, &len, NULL, NULL))) {
            product_err("%s socket_accept fail\n",  __FILE__);
        } else {
            product_info("audio data socket conn succ\n");
            while (!__THIS->spk_ready) {
                os_time_dly(2);
            }

            for (;;) {
                if ((recv_len = sock_recvfrom(__THIS->audio_data_sock, spk_frame, SPK_FRAME_SIZE, 0, NULL, NULL)) > 0) {
                    int ret = cbuf_write(&__THIS->spk_cbuf, spk_frame, recv_len);
                    os_sem_set(&__THIS->spk_sem, 0);
                    os_sem_post(&__THIS->spk_sem);
                }
            }
        }
    }
}


void audio_conn_init(int spk_value, int mic_value, u8 mic_res)
{
    if (!__THIS) {
        __THIS = zalloc(sizeof(struct product_net_type));
        ASSERT(__THIS);
    }
    spk_open(spk_value);
    thread_fork("audio_ctl_task", 10, 512, 0, NULL, audio_ctl_task, NULL);
    thread_fork("audio_data_task", 10, 512, 0, NULL, audio_data_task, NULL);

    mic_open(mic_value, mic_res);
    thread_fork("mic_data_rx_task", 6, 512, 0, NULL, mic_data_rx_task, NULL);
}
#endif

#endif

#endif


