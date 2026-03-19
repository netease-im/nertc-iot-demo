#include "product_main.h"

#ifdef PRODUCT_TEST_ENABLE


#ifndef PRODUCT_NET_CLIENT_ENABLE

static OS_SEM cdc_sem;
static u8 product_usb_id, online_flag = 0;


static int product_device_event_handler(struct sys_event *e)
{
    struct device_event *event = (struct device_event *)e->payload;
    const char *usb_msg = (const char *)event->value;

    if (e->from == DEVICE_EVENT_FROM_OTG) {
        if (usb_msg[0] == 's') {
            if (event->event == DEVICE_EVENT_IN) {
                product_info("usb %c online", usb_msg[2]);
                online_flag = 1;
                product_usb_id = usb_msg[2] - '0';
#ifdef PRODUCT_NET_SERVER_ENABLE
                void audio_transmit_init(u8 id);
                audio_transmit_init(product_usb_id);
#endif
            } else if (event->event == DEVICE_EVENT_OUT) {
                product_info("usb %c offline", usb_msg[2]);
                online_flag = 0;
            }
        }
    }
    return 0;
}


SYS_EVENT_STATIC_HANDLER_REGISTER(product_device_event) = {
    .event_type     = SYS_DEVICE_EVENT,
    .prob_handler   = product_device_event_handler,
    .post_handler   = NULL,
};


u8 product_tool_cdc_post(void)
{
    if (is_product_mode()) {
        os_sem_post(&cdc_sem);
    }
    return is_product_mode();
}


static s8 comm_dev_init(void)
{
    os_sem_create(&cdc_sem, 0);
    return 0;
}


static s8 comm_dev_online(void)
{
    if (!online_flag) {
        product_err("communication devices no online\n");
    }
    online_flag = 1;
    return online_flag;
}


static s32 comm_dev_read(u8 *data, u32 size)
{
    //os_sem_set(&cdc_sem, 0);
    os_sem_pend(&cdc_sem, 0);
    return cdc_read_data(product_usb_id, data, size);
}


static s32 comm_dev_write(u8 *data, u32 size)
{
    return cdc_write_data(product_usb_id, data, size);
}


static prod_comm_type comm = {
    .init   = comm_dev_init,
    .online = comm_dev_online,
    .read   = comm_dev_read,
    .write  = comm_dev_write,
};


int host_data_read(u8 *data, u32 len)
{
    return comm_ops()->read(data, len);
}


int host_data_write(u8 *data, u32 len)
{
    return comm_ops()->write(data, len);
}


u8 get_product_id(void)
{
    return product_usb_id;
}


#else


#include "sock_api/sock_api.h"
#include "os/os_api.h"
#include "app_config.h"
#include "system/includes.h"
#include "wifi/wifi_connect.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#define TXINFO_SIZE			  4
#define USB_DIR_OUT	          0
#define USB_DIR_IN	          0x80
#define WIFI_TXWI_HEAD_SIZE   20
#define WIFI_80211_FILL_SIZE  27 //保留一些字节不填充, 为了避开duration段和上层发送1个字节也支持


static void *host_sock = NULL;
static u8 online_flag = 0;
static u8 conn_flag = 0, reset;
static struct product_conn conn;
static __attribute__((aligned(4))) u8 *wifi_send_pkg = NULL;


static void wifi_tx_data(u8 *pkg, int len, u32 rate, u8 bw, u8 short_gi)// 最大包1513
{
    u16 *PktLen = &wifi_send_pkg[0];
    u16 *MPDUtotalByteCount = &wifi_send_pkg[10];
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
        memcpy(&wifi_send_pkg[WIFI_TXWI_HEAD_SIZE + WIFI_80211_FILL_SIZE], pkg, len);
    } else {
        memset(&wifi_send_pkg[WIFI_TXWI_HEAD_SIZE + WIFI_80211_FILL_SIZE], 0xaa, len);
    }

    struct urb urb;
    urb.pipe = USB_DIR_OUT;
    urb.complete = NULL;
    urb.transfer_buffer = wifi_send_pkg;
    urb.transfer_buffer_length = len + WIFI_TXWI_HEAD_SIZE + WIFI_80211_FILL_SIZE + 4;

    PTXWI_STRUC pTxWI = TXINFO_SIZE + (u8 *)urb.transfer_buffer;
    pTxWI->PHYMODE    = tx_rate_tab[rate].phy;
    pTxWI->MCS        = tx_rate_tab[rate].mcs;
    pTxWI->BW         = bw;
    pTxWI->ShortGI    = !short_gi;
    usb_submit_urb(&urb, 0);
}


static void broadcast_resp_task(void *priv)
{
    u8 idx = 0, mac[6];
    struct wifi_store_info *info = get_cur_wifi_info();
    //u8 ch[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    u8 ch[] = {2, 4, 6, 8, 10, 12};
    //u8 ch[] = {1, 3, 5, 7, 9, 11, 13};
    struct product_conn conn = {
        .str = "product_resp",
        .reset = 0,
    };
    u8 head[] = {
        0xc6, 0x00, 0x00, 0x04, 0xB0, 0x00, 0x04, 0x80, 0x35, 0x01, 0xB6, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x88, 0x88, /*dst*/ 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,/*src*/0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,/*BSSID*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*Seq,Frag num*/0x88, 0x88,
    };

    if (!wifi_send_pkg) {
        wifi_send_pkg = zalloc(1564);
        ASSERT(wifi_send_pkg);
        memcpy(wifi_send_pkg, head, sizeof(head));
    }


    conn.mode = info->mode;
    if (info->mode == AP_MODE) {
        wifi_get_mac(mac);
        sprintf(conn.ssid, PRODUCT_AP_PREFIX"%02x%02x%02x%02x%02x%02x", \
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        sprintf(conn.pwd, PRODUCT_AP_PWD);
    }

    for (u8 i = 0; i < ARRAY_SIZE(ch) * 2; i++) {
        wifi_set_channel(ch[(++idx) % ARRAY_SIZE(ch)]);
        for (u8 i = 0; i < 5; i++) {
            wifi_tx_data(&conn, sizeof(struct product_conn), 0, 0, 0);
        }
        product_info("%s, ch = idx = %d\n", __FUNCTION__, idx);
    }

    if (reset) {
        cpu_reset();
    } else {
        wifi_off();
        product_main();
    }
}


static u8 broadcast_resp(void)
{
    return thread_fork("broadcast_resp_task", 30, 1024, 0, NULL, broadcast_resp_task, NULL);
}


static void breathe_task(void *priv)
{
    void *socket_fd;
    int recv_len = 0;
    u8 recv_buf[50] = {0};
    struct sockaddr_in local_addr = {0};
    struct sockaddr_in remote_addr = {0};
    socklen_t len = sizeof(remote_addr);

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(BREATHE_PORT);

    if (!(socket_fd = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL))) {
        product_err("%s build socket fail\n",  __FILE__);
        return;
    }

    if (sock_bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) {
        product_err("%s sock_bind fail\n", __FILE__);
        return;
    }

    for (;;) {
        if ((recv_len = sock_recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&remote_addr, &len)) > 0) {
            //product_info("\n recv_len is %d, %s\n", recv_len, recv_buf);
            sock_sendto(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr_in));
        }
    }
}


static void breathe_init(void)
{
    thread_fork("breathe_task", 30, 512, 0, NULL, breathe_task, NULL);
}


static void host_conn_task(void *priv)
{
    u32 opt = 1;
    void *s_sock;
    struct sockaddr_in local_addr = {0};
    struct sockaddr_in remote_addr = {0};
    socklen_t len = sizeof(struct sockaddr_in);

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(HOST_PORT);

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

    if (!(host_sock = sock_accept(s_sock, (struct sockaddr *)&remote_addr, &len, NULL, NULL))) {
        product_err("%s socket_accept fail\n",  __FILE__);
        return;
    } else {
        product_info("host socket conn succ\n");
        online_flag = 1;
    }
}


static void host_conn_init(void)
{
    thread_fork("host_conn_task", 20, 512, 0, NULL, host_conn_task, NULL);
}


static int wifi_event_callback(void *network_ctx, enum WIFI_EVENT event)
{
    u8 mac[6];
    struct wifi_store_info wifi_default_mode_parm = {0};
    static u8 ssid[33] = {0}, pwd[65] = {0};

    switch (event) {
    case WIFI_EVENT_MODULE_INIT:
        puts("|network_user_callback->WIFI_EVENT_MODULE_INIT\n");

        if (conn.mode == AP_MODE) {
            strcpy(ssid, conn.ssid);
            strcpy(pwd, conn.pwd);
            wifi_default_mode_parm.mode = STA_MODE;
        } else {
            init_net_device_mac_addr((char *)mac, 1);
            sprintf((char *)ssid, PRODUCT_AP_PREFIX"%02x%02x%02x%02x%02x%02x", \
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            sprintf((char *)pwd, PRODUCT_AP_PWD);
            wifi_default_mode_parm.mode = AP_MODE;
        }

        if (wifi_default_mode_parm.mode <= AP_MODE) {
            strncpy((char *)wifi_default_mode_parm.ssid[wifi_default_mode_parm.mode - STA_MODE], ssid, sizeof(wifi_default_mode_parm.ssid[wifi_default_mode_parm.mode - STA_MODE]) - 1);
            strncpy((char *)wifi_default_mode_parm.pwd[wifi_default_mode_parm.mode - STA_MODE], pwd, sizeof(wifi_default_mode_parm.pwd[wifi_default_mode_parm.mode - STA_MODE]) - 1);
        }
        wifi_set_default_mode(&wifi_default_mode_parm, 1, 0);//wifi_default_mode_parm.mode == STA_MODE);
        break;
    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");
        break;
    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;
    case WIFI_EVENT_AP_START:
        printf("|network_user_callback->WIFI_EVENT_AP_START,CH=%d\n", wifi_get_channel());
        host_conn_init();
        breathe_init();
        camera_conn_init();
#ifdef CONFIG_AUDIO_ENABLE
        audio_conn_init(conn.spk_value, conn.mic_value, conn.mic_res);
#endif
        break;
    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");
        break;
    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        host_conn_init();
        breathe_init();
        camera_conn_init();
#ifdef CONFIG_AUDIO_ENABLE
        audio_conn_init(conn.spk_value, conn.mic_value, conn.mic_res);
#endif
        break;
    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;
    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
        break;
    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
        break;
    case WIFI_EVENT_STA_SCAN_COMPLETED:
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        break;
    case WIFI_EVENT_STA_CONNECT_SUCC:
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", wifi_get_channel());
        break;
    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;
    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;
    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
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
        puts("WIFI_EVENT_AP_ON_ASSOC\n");
        break;
    case WIFI_EVENT_AP_ON_DISCONNECTED:
        puts("WIFI_EVENT_AP_ON_DISCONNECTED\n");
        break;
    default:
        break;
    }

    return 0;
}


u8 product_info_check(void)
{
    static u8 init = 0;
    struct product_conn clean = {0};

    if (conn_flag) {
        syscfg_write(CFG_PRODUCT_CFG_INDEX, &clean, sizeof(struct product_conn));
        return TRUE;
    }

    if (!init) {
        init = 1;
        syscfg_read(CFG_PRODUCT_CFG_INDEX, &conn, sizeof(struct product_conn));
    }

    if (!strcmp(conn.str, "product_ask")) {
        product_info("str = %s, mode = %d, ssid = %s, pwd = %s\n", conn.str, conn.mode, conn.ssid, conn.pwd);
        syscfg_write(CFG_PRODUCT_CFG_INDEX, &clean, sizeof(struct product_conn));
        return TRUE;
    }

    return FALSE;
}


static void wifi_rx_frame_cb(void *rxwi, void *header, void *data, u32 len, void *reserve)
{
    u32 size = len;
    u8 *buf, offset;
    struct product_conn *conn;
    PRXWI_STRUC pRxWI = (PRXWI_STRUC *)rxwi;
    s8 rssi0 = pRxWI->RSSI0;

    if (rssi0 < PRODUCT_CLI_RSSI_THR) {
        product_info("RSSI = %d, THR = %d\n", rssi0, PRODUCT_CLI_RSSI_THR);
        return;
    }

    struct wifi_store_info *info = get_cur_wifi_info();
    offset = (info->mode == AP_MODE) ? 27 : 51;
    size = len - offset;
    //if (size < sizeof(struct product_conn)) {
    //    putchar('N');
    //	return;
    //}
    buf = ((u8 *)data) + offset;
    conn = buf;

    if (!strcmp(conn->str, "product_ask") && !conn_flag) {
        conn_flag = 1;
        reset = conn->reset;
        product_info("str = %s, mode = %d, ssid = %s, pwd = %s\n", conn->str, conn->mode, conn->ssid, conn->pwd);
        syscfg_write(CFG_PRODUCT_CFG_INDEX, conn, sizeof(struct product_conn));
        broadcast_resp();
    }
}


static void net_check_timer_hdl(void *priv)
{
    product_info("%s\n", __FUNCTION__);
    wifi_set_frame_cb(NULL, NULL);
}


void product_net_client_init(void)
{
    wifi_set_frame_cb(wifi_rx_frame_cb, NULL);
    if (PRODUCT_NET_CHECK_TIMEOUT) {
        sys_timeout_add(NULL, net_check_timer_hdl, PRODUCT_NET_CHECK_TIMEOUT);
    }
}


static s8 comm_dev_init(void)
{
    wifi_set_event_callback(wifi_event_callback);
    wifi_on();
}


static s8 comm_dev_online(void)
{
    return online_flag;
}


static s32 comm_dev_read(u8 *data, u32 size)
{
    if (!online_flag) {
        return 0;
    }
    return sock_recvfrom(host_sock, data, size, 0, NULL, NULL);
}


static s32 comm_dev_write(u8 *data, u32 size)
{
    if (!online_flag) {
        return 0;
    }
    return sock_send(host_sock, data, size, 0);
}


static prod_comm_type comm = {
    .init   = comm_dev_init,
    .online = comm_dev_online,
    .read   = comm_dev_read,
    .write  = comm_dev_write,
};


#endif


prod_comm_type *comm_ops(void)
{
    return &comm;
}


#endif


