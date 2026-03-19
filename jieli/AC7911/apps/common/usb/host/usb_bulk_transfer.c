/**
 * @file usb_bulk_transfer.c
 * @brief bulk transfer driver
 * @author chenrixin@zh-jieli.com
 * @version 1.00
 * @date 2017-02-09
 */

#include <string.h>
#include "jiffies.h"
#include "usb_config.h"
#include "usb_bulk_transfer.h"
#include "usb_ctrl_transfer.h"
#include "usb_storage.h"
#include "usb/host/usb_host.h"
#include "usb/usb_phy.h"
#include "app_config.h"
#include "device_drive.h"

#if TCFG_USB_HOST_ENABLE
#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

struct usb_request_block {
    void *ptr;
    u32 len;
    u32 target_ep;
    u16 rxmap;
    u16 txmap;
    u32 msg;
    u32 async_mode;
};
static struct usb_request_block urb[USB_MAX_HW_NUM];

u8 get_async_mode(usb_dev usb_id)
{
#if UDISK_READ_512_ASYNC_ENABLE
    return urb[usb_id].async_mode;
#endif
    return BULK_ASYNC_MODE_EXIT;
}
void set_async_mode(usb_dev usb_id, u8 mode)
{
#if UDISK_READ_512_ASYNC_ENABLE
    CPU_CRITICAL_ENTER();
    urb[usb_id].async_mode = mode;
    CPU_CRITICAL_EXIT();
#endif
}
static void usb_bulk_rx_isr(struct usb_host_device *host_dev, u32 ep)
{
    usb_dev usb_id = host_device2id(host_dev);
    int l  = min(urb[usb_id].len, urb[usb_id].rxmap);

    l = usb_h_ep_read_async(usb_id, ep, urb[usb_id].target_ep, urb[usb_id].ptr, l, USB_ENDPOINT_XFER_BULK, 0);
    /* g_printf("%s() %d %d", __func__, l, urb[usb_id].len); */
    if (l > 0) {
        urb[usb_id].len -= l;
        if (urb[usb_id].ptr) {
            urb[usb_id].ptr += l;
        }
        urb[usb_id].msg = 0;
    } else {
        urb[usb_id].msg = l;
        urb[usb_id].len = 0;
    }

    if (urb[usb_id].len == 0) {
        u32 async_mode = get_async_mode(usb_id);

        if (async_mode == BULK_ASYNC_MODE_EXIT) {
            usb_sem_post(host_dev);
        } else if (async_mode == BULK_ASYNC_MODE_SEM_PEND) {
            if (urb[usb_id].msg == 0) {
                usb_sem_post(host_dev);
            } else {
                r_printf("usb async error");
            }
        }
    } else {
        usb_h_ep_read_async(usb_id, ep, urb[usb_id].target_ep, urb[usb_id].ptr, urb[usb_id].len, USB_ENDPOINT_XFER_BULK, 1);
    }
}
s32 usb_bulk_only_receive_async(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len)
{
    struct usb_host_device *host_dev = device_to_usbdev(device);
    const usb_dev usb_id = host_device2id(host_dev);
    u8 devnum = host_dev->private_data.devnum;

    urb[usb_id].ptr = pBuf;
    urb[usb_id].len = len;
    urb[usb_id].target_ep = target_ep;

#ifdef CONFIG_USB_SUPPORT_MRX_TX
    urb[usb_id].rxmap = 1 * 1024;
#else
    if (usb_id == 0) {
        urb[usb_id].rxmap = 0x40;
    } else {
        urb[usb_id].rxmap = rxmaxp;
    }
#endif

    urb[usb_id].msg = -DEV_ERR_OFFLINE;

    usb_h_set_ep_isr(host_dev, host_ep | USB_DIR_IN, usb_bulk_rx_isr, host_dev);
    usb_set_intr_rxe(usb_id, host_ep);
    usb_write_rxfuncaddr(usb_id, host_ep, devnum);

    int ret = usb_h_ep_read_async(usb_id, host_ep, target_ep, urb[usb_id].ptr, len, USB_ENDPOINT_XFER_BULK, 1);
    if (ret < 0) {
        return ret;
    }
    ret = usb_sem_pend(host_dev, 300);

    usb_clr_intr_rxe(usb_id, host_ep);
    usb_h_set_ep_isr(host_dev, host_ep | USB_DIR_IN, NULL, host_dev);
    if (ret) {
        return -DEV_ERR_TIMEOUT;
    }
    return urb[usb_id].msg ? urb[usb_id].msg : len;
}

/**
 * @brief usb_bulk_receive_async_no_wait 启动USB Bulk 异步预读
 *        不用等信号量，启动传输后返回
 * @param device 设备句柄
 * @param pBuf 读buffer缓冲区，芯片所有memory都可以
 * @param len 需要预读的长度
 *
 * @return 负数表示失败
 */
s32 usb_bulk_receive_async_no_wait(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len)
{
    struct usb_host_device *host_dev = device_to_usbdev(device);
    const usb_dev usb_id = host_device2id(host_dev);
    u8 devnum = host_dev->private_data.devnum;

    set_async_mode(usb_id, BULK_ASYNC_MODE_SEM_PEND);
    urb[usb_id].ptr = pBuf;
    urb[usb_id].len = len;
    urb[usb_id].target_ep = target_ep;

#ifdef CONFIG_USB_SUPPORT_MRX_TX
    urb[usb_id].rxmap = 1 * 1024;
#else
    if (usb_id == 0) {
        urb[usb_id].rxmap = 0x40;
    } else {
        urb[usb_id].rxmap = rxmaxp;
    }
#endif

    urb[usb_id].msg = -DEV_ERR_OFFLINE;

    usb_h_set_ep_isr(host_dev, host_ep | USB_DIR_IN, usb_bulk_rx_isr, host_dev);
    usb_set_intr_rxe(usb_id, host_ep);
    usb_write_rxfuncaddr(usb_id, host_ep, devnum);

    int ret = usb_h_ep_read_async(usb_id, host_ep, target_ep, urb[usb_id].ptr, len, USB_ENDPOINT_XFER_BULK, 1);
    if (ret < 0) {
        if (ret == -DEV_ERR_RXSTALL) {
            ret = usb_clear_feature(host_dev, target_ep | USB_DIR_IN);
            if (ret == 0) {
                return -DEV_ERR_RXSTALL;
            }
        }
        return ret;
    }
    return len;
}
static s32 _usb_bulk_only_receive(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len)
{
#if 0
    struct usb_host_device *host_dev = device_to_usbdev(device);
    const usb_dev usb_id = host_device2id(host_dev);
    return usb_h_bulk_read(usb_id, host_ep, rxmaxp, target_ep, pBuf, len);
#else
    return usb_bulk_only_receive_async(device, host_ep, rxmaxp, target_ep, pBuf, len);
#endif
}
s32 usb_bulk_only_receive(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len)
{

    struct usb_host_device *host_dev = device_to_usbdev(device);
    int ret = _usb_bulk_only_receive(device, host_ep, rxmaxp, target_ep, pBuf, len);
    if (ret == -DEV_ERR_RXSTALL) {
        ret = usb_clear_feature(host_dev, target_ep | USB_DIR_IN);
        if (ret == 0) {
            return -DEV_ERR_RXSTALL;
        }
    }
    return ret;
}
static void usb_bulk_tx_isr(struct usb_host_device *host_dev, u32 ep)
{
    usb_dev usb_id = host_device2id(host_dev);
    int l  = min(urb[usb_id].len, urb[usb_id].txmap);
    l = usb_h_ep_write_async(usb_id, ep, urb[usb_id].txmap, urb[usb_id].target_ep, urb[usb_id].ptr, l, USB_ENDPOINT_XFER_BULK, 0);

    if (l > 0) {
        urb[usb_id].len -= l;
        urb[usb_id].ptr += l;
        urb[usb_id].msg = 0;
    } else {
        urb[usb_id].msg = l;
        urb[usb_id].len = 0;
    }

    if (urb[usb_id].len == 0) {
        if (urb[usb_id].msg || l == 0) {
            usb_sem_post(host_dev);
        }
    }
}
s32 usb_bulk_only_send_async(struct device *device, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *pBuf, u32 len)
{
    struct usb_host_device *host_dev = device_to_usbdev(device);
    const usb_dev usb_id = host_device2id(host_dev);
    u8 devnum = host_dev->private_data.devnum;

    urb[usb_id].target_ep = target_ep;
#ifdef CONFIG_USB_SUPPORT_MRX_TX
    urb[usb_id].txmap = 8 * 1024;
#else
    if (usb_id == 0) {
        urb[usb_id].txmap = 0x40;
    } else {
        urb[usb_id].txmap = txmaxp;
    }
#endif


    urb[usb_id].msg = -DEV_ERR_OFFLINE;
    urb[usb_id].len = len - min(len, urb[usb_id].txmap);
    urb[usb_id].ptr = (u8 *)pBuf + min(len, urb[usb_id].txmap);


    usb_h_set_ep_isr(host_dev, host_ep, usb_bulk_tx_isr, host_dev);
    usb_set_intr_txe(usb_id, host_ep);

    usb_write_txfuncaddr(usb_id, host_ep, devnum);

    int ret = usb_h_ep_write_async(usb_id, host_ep, txmaxp, target_ep, pBuf, min(len, urb[usb_id].txmap), USB_ENDPOINT_XFER_BULK, 1);
    if (ret < 0) {
        return ret;
    }
    ret = usb_sem_pend(host_dev, 250);

    usb_clr_intr_txe(usb_id, host_ep);
    usb_h_set_ep_isr(host_dev, host_ep, NULL, host_dev);

    if (ret) {
        r_printf("ret %d", ret);
        return -DEV_ERR_TIMEOUT;
    }
    /* g_printf("%s() %d %d", __func__, urb[usb_id].len, urb[usb_id].msg); */
    return urb[usb_id].msg ? urb[usb_id].msg : len;
}
/**
 * @brief usb_bulk_only_send
 *
 * @param device
 * @param host_ep   主机的端点号
 * @param target_ep 目标设备的端点号
 * @param pBuf
 * @param len
 *
 * @return 负数失败
 * 		正数发送的字节数
 */
static s32 _usb_bulk_only_send(struct device *device, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *pBuf, u32 len)
{
#if 0
    struct usb_host_device *host_dev = device_to_usbdev(device);
    const usb_dev usb_id = host_device2id(host_dev);
    return usb_h_bulk_write(usb_id, host_ep, txmaxp, target_ep, pBuf, len);
#elif 0
    if (len < 512) {
        struct usb_host_device *host_dev = device_to_usbdev(device);
        const usb_dev usb_id = host_device2id(host_dev);
        return usb_h_bulk_write(usb_id, host_ep, txmaxp, target_ep, pBuf, len);
    } else {
        return usb_bulk_only_send_async(device, host_ep, txmaxp, target_ep, pBuf, len);
    }
#else
    return usb_bulk_only_send_async(device, host_ep, txmaxp, target_ep, pBuf, len);
#endif
}
s32 usb_bulk_only_send(struct device *device, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *pBuf, u32 len)
{
    struct usb_host_device *host_dev = device_to_usbdev(device);
    int ret = _usb_bulk_only_send(device, host_ep, txmaxp, target_ep, pBuf, len);

    if (ret == -DEV_ERR_TXSTALL) {
        ret = usb_clear_feature(host_dev, target_ep);
        if (ret == 0) {
            return -DEV_ERR_TXSTALL;
        }
    }
    return ret;

}
#endif
