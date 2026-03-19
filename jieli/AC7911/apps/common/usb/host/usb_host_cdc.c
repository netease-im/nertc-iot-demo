#include "device/ioctl_cmds.h"
#include "device_drive.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "cdc_defs.h"
#include "usb_host_cdc.h"
#include "usb_host.h"
#include "usb_config.h"
#include "app_config.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_HOST_CDC_ENABLE

static int usb_cdc_set_power(struct usb_host_device *host_dev, u32 value);
static int usb_cdc_get_power(struct usb_host_device *host_dev, u32 value);
static int usb_cdc_rx_task_create(struct usb_host_device *host_dev);
static void husb_cdc_isr(struct usb_host_device *host_dev, u32 ep);
static int usb_cdc_close(struct device *device);
int cdc_rx_callback(u8 usb_id, void *buf, int len);

static const struct interface_ctrl cdc_ctrl = {
    .interface_class = USB_CLASS_COMM,
    .set_power = usb_cdc_set_power,
    .get_power = usb_cdc_get_power,
};

static struct usb_interface_info cdc_itf[USB_MAX_HW_NUM] = {
    {
        .dev.cdc = NULL,
        .ctrl = (struct interface_ctrl *) &cdc_ctrl,
    },
#if USB_MAX_HW_NUM > 1
    {
        .dev.cdc = NULL,
        .ctrl = (struct interface_ctrl *) &cdc_ctrl,
    },
#endif
};

#define device_to_cdc(device)  (cdc_itf[usb_id].dev.cdc)

#define check_ret(ret) \
    if ((int)(ret) < DEV_ERR_NONE) { \
        log_error("err ret = %d\n", ret); \
        goto __exit_fail; \
    }

struct usb_cdc_line_coding g_cdc_line_coding[USB_MAX_HW_NUM];
static int usb_cdc_rx_task_id[USB_MAX_HW_NUM] = {0};
static OS_SEM usbcdc_irq_sem[USB_MAX_HW_NUM];
static u8 usbcdc_task_kill[USB_MAX_HW_NUM];
static struct device cdc_device[USB_MAX_HW_NUM];

static int usb_cdc_set_power(struct usb_host_device *host_dev, u32 value)
{
    usb_dev usb_id = host_device2id(host_dev);
    struct device *device = &cdc_device[usb_id];
    struct h_usb_cdc *cdc = device_to_cdc(device);
    if (!cdc) {
        return -DEV_ERR_INVALID_HANDLE;
    }
    cdc->active = value;
    return 0;
}

static int usb_cdc_get_power(struct usb_host_device *host_dev, u32 value)
{
    usb_dev usb_id = host_device2id(host_dev);
    struct device *device = &cdc_device[usb_id];
    struct h_usb_cdc *cdc = device_to_cdc(device);
    if (!cdc) {
        return 0;
    }
    return cdc->active;
}

/**
 * @brief usb_cdc_parser
 *
 * @param host_dev
 * @param idx
 * @param pBuf
 *
 * @return
 */
s32 usb_cdc_parser(struct usb_host_device *host_dev, s32 idx, u8 *pBuf)
{
    int len = 0;
    struct usb_interface_descriptor if_desc;
    struct usb_endpoint_descriptor end_desc;
    struct usb_private_data *private_data = &host_dev->private_data;
    usb_dev usb_id = host_device2id(host_dev);
    int cur_len;
    int cur_type;
    int cur_sub_type;
    u8 ItfSubClass = 0;
    u8 cur_itf_num = 0;

    struct h_usb_cdc *cdc = (struct h_usb_cdc *)zalloc(sizeof(struct h_usb_cdc));
    if (!cdc) {
        len = -DEV_ERR_INVALID_BUF;
        goto __exit_fail;
    }

    while (1) {
        cur_len = pBuf[len];
        cur_type = pBuf[len + 1];
        cur_sub_type = pBuf[len + 2];

        if (!cur_len) {
            break;
        }

        if (cur_type == USB_DT_INTERFACE_ASSOCIATION) {
            //	composite device
            log_info("USB_DT_INTERFACE_ASSOCIATION");
            log_info_hexdump(pBuf + len, cur_len);
        } else if (cur_type == USB_DT_INTERFACE) {
            if (cur_len != USB_DT_INTERFACE_SIZE) {
                len = -USB_DT_INTERFACE;
                goto __exit_fail;
            }
            if (pBuf[len + 5] != 0x2 && pBuf[len + 5] != 0xa) {
                break;
            }
            log_info("USB_DT_INTERFACE");
            log_info_hexdump(pBuf + len, cur_len);

            if (pBuf[len + 5] == 0x2) {  //InterfaceClass, Communation and CDC control
                if (pBuf[len + 6] == 0x02) {  //InterfaceSubClass, Abstract Control Model
                    cdc->itf_num = pBuf[len + 2];
                    ItfSubClass = 0x02;
                } else {
                    log_error("invalid cdc subclass");
                    len = -USB_DT_INTERFACE;
                    goto __exit_fail;
                }
            } else if (pBuf[len + 5] == 0xa) {
                ItfSubClass = pBuf[len + 6];
            }
            cur_itf_num = pBuf[len + 2];
            memcpy(&if_desc, pBuf + len, cur_len);
        } else if (cur_type == USB_DT_CS_INTERFACE) {
            log_info_hexdump(pBuf + len, cur_len);
        } else if (cur_type == USB_DT_ENDPOINT) {
            log_info("USB_DT_ENDPOINT");
            log_info_hexdump(pBuf + len, cur_len);
            for (int endnum = 0; endnum < if_desc.bNumEndpoints; endnum++) {
                memcpy(&end_desc, pBuf + len, cur_len);

                if (end_desc.bDescriptorType != USB_DT_ENDPOINT ||
                    end_desc.bLength < USB_DT_ENDPOINT_SIZE) {

                    log_error("invalid descriptor for endpoint bDescriptorType = %d bLength= %d",
                              end_desc.bDescriptorType, end_desc.bLength);
                    len = -USB_DT_ENDPOINT;
                    goto __exit_fail;
                }

                if ((end_desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {

                    if (end_desc.bEndpointAddress & USB_DIR_IN) {
                        cdc->epin = end_desc.bEndpointAddress & 0x0f;
                        cdc->rxmaxp = end_desc.wMaxPacketSize;
                    } else {
                        cdc->epout = end_desc.bEndpointAddress & 0x0f;
                        cdc->txmaxp = end_desc.wMaxPacketSize;
                    }
                }
            }
        } else if (cur_type == USB_DT_CS_ENDPOINT) {
            log_info("USB_DT_CS_ENDPOINT");
            log_info_hexdump(pBuf + len, cur_len);
        } else {
            break;
        }

        len += cur_len;
    }

    cdc_itf[usb_id].dev.cdc = cdc;
    host_dev->interface_info[idx] = &cdc_itf[usb_id];
    return len;

__exit_fail:
    if (cdc) {
        free(cdc);
    }
    return len;
}

/**
 * @brief usb_cdc_init
 *
 * @param host_dev
 *
 * @return
 */
s32 usb_cdc_init(usb_dev usb_id)
{
    struct usb_host_device *host_dev = host_id2device(usb_id);
    struct usb_private_data *private_data = &host_dev->private_data;
    struct device *device = &cdc_device[usb_id];
    struct h_usb_cdc *cdc = device_to_cdc(device);
    s32 ret = -1;
    struct usb_cdc_line_coding line_coding = {0};
    log_info("device=%x", (u32)device);

    if (!host_dev_status(host_dev)) {
        return -DEV_ERR_OFFLINE;
    }
    if (!cdc) {
        return -DEV_ERR_OFFLINE;
    }

    device->private_data = host_dev;

    cdc->host_epin = usb_get_ep_num(usb_id, USB_DIR_IN, USB_ENDPOINT_XFER_BULK);
    if (cdc->host_epin == (u8) - 1) {
        ret = -DEV_ERR_INUSE;
        goto __exit_fail;
    }
    cdc->epin_buffer = usb_h_alloc_ep_buffer(usb_id, cdc->host_epin | USB_DIR_IN, cdc->rxmaxp * 2);
    if (!cdc->epin_buffer) {
        ret = -DEV_ERR_INVALID_BUF;
        goto __exit_fail;
    }
    cdc->host_epout = usb_get_ep_num(usb_id, USB_DIR_OUT, USB_ENDPOINT_XFER_BULK);
    if (cdc->host_epout == (u8) - 1) {
        ret = -DEV_ERR_INUSE;
        goto __exit_fail;
    }
    cdc->epout_buffer = usb_h_alloc_ep_buffer(usb_id, cdc->host_epout | USB_DIR_OUT, cdc->txmaxp);
    if (!cdc->epout_buffer) {
        ret = -DEV_ERR_INVALID_BUF;
        goto __exit_fail;
    }

    usb_h_ep_config(usb_id, cdc->host_epin | USB_DIR_IN, USB_ENDPOINT_XFER_BULK, 1, 0, cdc->epin_buffer, cdc->rxmaxp);
    usb_write_rxfuncaddr(usb_id, cdc->host_epin, private_data->devnum);
    usb_h_set_ep_isr(host_dev, cdc->host_epin | USB_DIR_IN, husb_cdc_isr, host_dev);
    log_debug("CDC_EP_IN %d(H) <-- %d(D)", cdc->host_epin, cdc->epin);

    usb_h_ep_config(usb_id, cdc->host_epout | USB_DIR_OUT, USB_ENDPOINT_XFER_BULK, 0, 0, cdc->epout_buffer, cdc->txmaxp);
    usb_write_txfuncaddr(usb_id, cdc->host_epout, private_data->devnum);
    log_debug("CDC_EP_OUT %d(H) --> %d(D)", cdc->host_epout, cdc->epout);

    ret = usb_control_msg(host_dev,
                          CDC_GET_LINE_CODING,
                          USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0,
                          cdc->itf_num,
                          &line_coding,
                          sizeof(struct usb_cdc_line_coding));
    log_info("ret=%x", ret);
    /* check_ret(ret); */

    os_mutex_create(&cdc->mutex);

    usb_cdc_rx_task_create(host_dev);

    return DEV_ERR_NONE;

__exit_fail:
    if (cdc) {
        if (cdc->epout_buffer) {
            usb_h_free_ep_buffer(usb_id, cdc->epout_buffer);
            cdc->epout_buffer = NULL;
        }
        if (cdc->epin_buffer) {
            usb_h_free_ep_buffer(usb_id, cdc->epin_buffer);
            cdc->epin_buffer = NULL;
        }
    }
    return ret;
}

static void husb_cdc_isr(struct usb_host_device *host_dev, u32 ep)
{
    /* printf("\n\n\n\n[ debug ]--func=%s line=%d \n\n", __func__, __LINE__); */
    usb_dev usb_id = host_device2id(host_dev);
    struct device *device = &cdc_device[usb_id];
    struct h_usb_cdc *cdc = device_to_cdc(device);
    if (cdc && cdc->active) {
        os_sem_post(&usbcdc_irq_sem[usb_id]);
    }
}

static void usb_cdc_rx_task(void *p)
{
    int ret = 0;
    struct usb_host_device *host_dev = (struct usb_host_device *)p;
    struct usb_private_data *private_data = &host_dev->private_data;
    usb_dev usb_id = host_device2id(host_dev);
    struct device *device = &cdc_device[usb_id];
    struct h_usb_cdc *cdc = device_to_cdc(device);
    u8 *recv_buffer;

    recv_buffer = malloc(cdc->rxmaxp);
    ASSERT(recv_buffer, "%s() malloc buffer fail", __func__);

    while (1) {
        ret = os_sem_pend(&usbcdc_irq_sem[usb_id], 0);
        if (ret) {
            continue;
        }
        if (usbcdc_task_kill[usb_id]) {
            usbcdc_task_kill[usb_id] = 0;
            break;
        }

        if (!host_dev_status(host_dev)) {
            continue;
        }
        cdc = device_to_cdc(device);
        if (!cdc || !cdc->active) {
            continue;
        }

        os_mutex_pend(&cdc->mutex, 0);
        ret = usb_h_ep_read_async(usb_id, cdc->host_epin, cdc->epin, recv_buffer, cdc->rxmaxp, USB_ENDPOINT_XFER_BULK, 0);
        if (ret >= 0) {
            cdc_rx_callback(usb_id, recv_buffer, ret);
        }
        //触发下一次接收
        usb_h_ep_read_async(usb_id, cdc->host_epin, cdc->epin, NULL, 0, USB_ENDPOINT_XFER_BULK, 1);
        os_mutex_post(&cdc->mutex);
    }
    free(recv_buffer);
}

static int usb_cdc_rx_task_create(struct usb_host_device *host_dev)
{
    usb_dev usb_id = host_device2id(host_dev);
    os_sem_create(&usbcdc_irq_sem[usb_id], 0);
    thread_fork("usb_cdc_rx_task", 20, 1024, 0, &usb_cdc_rx_task_id[usb_id], usb_cdc_rx_task, host_dev);
    return 0;
}

int usb_cdc_exit(usb_dev usb_id)
{
    struct usb_host_device *host_dev = host_id2device(usb_id);
    struct h_usb_cdc *cdc = cdc_itf[usb_id].dev.cdc;
    struct device *device = &cdc_device[usb_id];

    if (!cdc) {
        return DEV_ERR_NONE;
    }
    log_debug("cdc exit!!");
    usb_cdc_close(device);
    os_mutex_pend(&cdc->mutex, 0);
    if (cdc->epout_buffer) {
        usb_h_free_ep_buffer(usb_id, cdc->epout_buffer);
        cdc->epout_buffer = NULL;
    }
    if (cdc->epin_buffer) {
        usb_h_free_ep_buffer(usb_id, cdc->epin_buffer);
        cdc->epin_buffer = NULL;
    }
    os_mutex_del(&cdc->mutex, 1);
    free(cdc);
    cdc_itf[usb_id].dev.cdc = NULL;
    usbcdc_task_kill[usb_id] = 1;
    os_sem_post(&usbcdc_irq_sem[usb_id]);
    thread_kill(&usb_cdc_rx_task_id[usb_id], KILL_WAIT);
    return DEV_ERR_NONE;
}




/**
 * @brief usb_cdc_dev_init
 *
 * @param node
 * @param _data
 *
 * @return
 */
static int usb_cdc_dev_init(const struct dev_node *node, void *_data)
{
    struct usb_cdc_platform_data *data 	= (struct usb_cdc_platform_data *)_data;
    usb_dev usb_id;
    for (usb_id = 0; usb_id < USB_MAX_HW_NUM; usb_id++) {
        g_cdc_line_coding[usb_id].dwDTERate = data->baud_rate;
        g_cdc_line_coding[usb_id].bCharFormat = data->stop_bits;
        g_cdc_line_coding[usb_id].bParityType = data->parity;
        g_cdc_line_coding[usb_id].bDataBits = data->data_bits;
    }

    return 0;
}

/**
 * @brief usb_cdc_open
 *
 * @param node
 * @param device
 * @param arg
 *
 * @return
 */
static int usb_cdc_dev_open(const char *name, struct device **device, void *arg)
{
    usb_dev usb_id;
    s32 ret = DEV_ERR_NONE;
    struct usb_cdc_line_coding line_coding = {0};
    struct usb_cdc_platform_data *data;
    struct usb_host_device *host_dev;
    struct h_usb_cdc *cdc;

    usb_id = name[strlen(name) - 1] - '0';
    *device = &cdc_device[usb_id];
    //空句柄检查
    host_dev = (*device)->private_data;
    if (!host_dev) {
        return -DEV_ERR_OFFLINE;
    }
    cdc = cdc_itf[usb_id].dev.cdc;
    if (!cdc) {
        return -DEV_ERR_OFFLINE;
    }

    log_info("device %x %x", (u32)*device, (u32)cdc);
    os_mutex_pend(&cdc->mutex, 0);
    // reentrant protect
    if (atomic_read(&((*device)->ref))) {
        os_mutex_post(&cdc->mutex);
        return DEV_ERR_NONE;
    }
    log_info("cdc dev name %s", name);
    if (arg) {
        data = (struct usb_cdc_platform_data *)arg;
        g_cdc_line_coding[usb_id].dwDTERate = data->baud_rate;
        g_cdc_line_coding[usb_id].bCharFormat = data->stop_bits;
        g_cdc_line_coding[usb_id].bParityType = data->parity;
        g_cdc_line_coding[usb_id].bDataBits = data->data_bits;
    }
    ret = usb_control_msg(host_dev,
                          CDC_SET_LINE_CODING,
                          USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0,
                          cdc->itf_num,
                          &g_cdc_line_coding[usb_id],
                          sizeof(struct usb_cdc_line_coding));
    check_ret(ret);

    ret = usb_control_msg(host_dev,
                          CDC_GET_LINE_CODING,
                          USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0,
                          cdc->itf_num,
                          &line_coding,
                          sizeof(struct usb_cdc_line_coding));

    check_ret(ret);

    if (memcmp(&line_coding, &g_cdc_line_coding[usb_id],
               sizeof(struct usb_cdc_line_coding))) {
        ret = -EINVAL;
        goto __exit_fail;
    }

    //Set Contorl Line State
    ret = usb_control_msg(host_dev,
                          CDC_SET_CONTROL_LINE_STATE,
                          USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          1,
                          cdc->itf_num,
                          NULL,
                          0);
    check_ret(ret);

    //触发接收
    usb_h_ep_read_async(usb_id, cdc->host_epin, cdc->epin, NULL, 0, USB_ENDPOINT_XFER_BULK, 1);

    os_mutex_post(&cdc->mutex);

    return DEV_ERR_NONE;

__exit_fail:
    os_mutex_post(&cdc->mutex);
    return ret;
}

static s32 usb_cdc_dev_write(struct device *device, void *buf,  u32 len, u32 offset)
{
    s32 ret = DEV_ERR_NONE;
    struct usb_host_device *host_dev;
    struct usb_private_data *private_data;
    struct h_usb_cdc *cdc;
    usb_dev usb_id;

    //空句柄检查
    host_dev = device->private_data;
    if (!host_dev) {
        return -DEV_ERR_OFFLINE;
    }
    private_data = &host_dev->private_data;
    usb_id = host_device2id(host_dev);
    cdc = device_to_cdc(device);
    if (!cdc) {
        return -DEV_ERR_OFFLINE;
    }

    os_mutex_pend(&cdc->mutex, 0);
    ret = usb_bulk_only_send(device,
                             cdc->host_epout,
                             cdc->txmaxp,
                             cdc->epout,
                             (u8 *)buf,
                             len);
    os_mutex_post(&cdc->mutex);
    /* check_ret(ret); */

    return ret;
}

static int usb_cdc_dev_read(struct device *device, void *buf, u32 len, u32 offset)
{
    s32 ret;
    struct usb_host_device *host_dev;
    struct usb_private_data *private_data;
    struct h_usb_cdc *cdc;
    usb_dev usb_id;

    //空句柄检查
    host_dev = device->private_data;
    if (!host_dev) {
        return -DEV_ERR_OFFLINE;
    }
    private_data = &host_dev->private_data;
    usb_id = host_device2id(host_dev);
    cdc = device_to_cdc(device);
    if (!cdc) {
        return -DEV_ERR_OFFLINE;
    }

    os_mutex_pend(&cdc->mutex, 0);
    ret = usb_bulk_only_receive(device,
                                cdc->host_epin,
                                cdc->rxmaxp,
                                cdc->epin,
                                (u8 *)buf,
                                len);
    os_mutex_post(&cdc->mutex);
    /* check_ret(ret); */

    return ret;
}

static int usb_cdc_close(struct device *device)
{
    struct usb_host_device *host_dev;
    struct usb_private_data *private_data;
    struct h_usb_cdc *cdc;
    usb_dev usb_id;

    //空句柄检查
    host_dev = device->private_data;
    if (!host_dev) {
        return -DEV_ERR_OFFLINE;
    }
    private_data = &host_dev->private_data;
    usb_id = host_device2id(host_dev);
    cdc = device_to_cdc(device);
    if (!cdc) {
        return -DEV_ERR_OFFLINE;
    }

    os_mutex_pend(&cdc->mutex, 0);
    usb_h_set_ep_isr(host_dev, cdc->host_epin | USB_DIR_IN, NULL, NULL);
    usb_clr_intr_rxe(usb_id, cdc->host_epin);
    os_mutex_post(&cdc->mutex);

    return 0;
}

static bool usb_cdc_dev_online(const struct dev_node *node)
{
    usb_dev usb_id = node->name[strlen(node->name) - 1] - '0';
    struct device *device = &cdc_device[usb_id];
    struct h_usb_cdc *cdc = device_to_cdc(device);
    if (cdc && cdc->active) {
        return true;
    }

    return false;
}

const struct device_operations usb_cdc_ops = {
    .init = usb_cdc_dev_init,
    .online = usb_cdc_dev_online,
    .open = usb_cdc_dev_open,
    /* .read = usb_cdc_dev_read, */
    .write = usb_cdc_dev_write,
    /* .ioctl = usb_cdc_dev_ioctl, */
    .close = usb_cdc_close,
};


/**
 * @brief cdc接收数据弱定义函数
 *
 * @param buf: 接收数据buffer
 * @param len: 接收到数据长度,小于最大包长
 */
__attribute__((weak))
int cdc_rx_callback(u8 usb_id, void *buf, int len)
{
    log_debug("usb%d cdc receive", usb_id);
    log_debug_hexdump(buf, len);
    return 0;
}


//cdc发送demo
void host_cdc_send_demo(u8 usb_id)
{
    int len = 0;
    void *cdc_dev = NULL;
    char cdc_name[16];
    sprintf(cdc_name, "cdc%d", usb_id);
    cdc_dev = dev_open(cdc_name, NULL); /* 需要在板级定义cdc0设备 */
    if (cdc_dev) {
        /* os_time_dly(100); */
        printf("cdc%d dev open success\n", usb_id);
        u8 buf[4] = {0x01, 0x02, 0x03, 0x55};
        len = dev_write(cdc_dev, buf, 4);
        printf("len=%d\n", len);
        u8 buf1[4] = {0x09, 0x0a, 0x0b, 0x5a};
        len = dev_write(cdc_dev, buf1, 4);
        printf("len=%d\n", len);
        os_time_dly(10);
        dev_close(cdc_dev);
    }
}

#endif
