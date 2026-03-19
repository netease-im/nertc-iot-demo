#include "device/device.h"
#include "event/event.h"
#include "event/device_event.h"
#include "asm/sfc_norflash_api.h"
#include "fs/fs.h"
#include "system/init.h"
#include "sdio/sdmmc.h"
#include "generic/ascii.h"
#include "app_config.h"

#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
#include "usb_stack.h"
#include "msd.h"
#include "device/hid.h"
#include "uac_audio.h"
#include "usb_host.h"
#include "audio.h"
#include "hid.h"
#include "adb.h"
#include "usbnet.h"
#include "usb_host_cdc.h"
#endif

#ifdef CONFIG_JLFAT_ENABLE
#define FAT_FILE_SYSTEM_NAME "jlfat"
#else
#define FAT_FILE_SYSTEM_NAME "fat"
#endif

#ifndef FAT_CACHE_NUM
#define FAT_CACHE_NUM 32
#endif

enum {
    SD_UNMOUNT = 0,
    SD_MOUNT_SUSS,
    SD_MOUNT_FAILD,
};

static u8 sd_mount[2];
static OS_MUTEX sd_mutex;

static int __sd_mutex_init()
{
    return os_mutex_create(&sd_mutex);
}
early_initcall(__sd_mutex_init);

static int mount_sd_to_fs(const char *sd_name)
{
    int err = 0;
    u32 class = 0, capacity = 0, block_size = 0;
    struct imount *mt;
    int id = sd_name[2] - '0';
    const char *dev = sd_name;
    void *fd = NULL;

    err = os_mutex_pend(&sd_mutex, 0);
    if (err) {
        return -EFAULT;
    }

    if (sd_mount[id] == SD_MOUNT_SUSS) {
        goto __exit;
    }
    if (sd_mount[id] == SD_MOUNT_FAILD) {
        err = -EFAULT;
        goto __exit;
    }
    if (!dev_online(dev)) {
        err = -EFAULT;
        goto __exit;
    }

#ifdef CONFIG_DMSDX_ENABLE
    u8 i = 0;
    char sub_dev_name[7] = {0};
    char mount_path_name[16] = {0};

    strcpy(sub_dev_name, dev);
    strcpy(mount_path_name, id ? "storage/sd1" : "storage/sd0");

    do {
        sub_dev_name[strlen(dev)] = '.';
        sub_dev_name[strlen(dev) + 1] = '0' + i;
        fd = dev_open(sub_dev_name, 0);
        if (fd) {
            dev_close(fd);
            mount_path_name[11] = '.';
            mount_path_name[12] = '0' + i;
            ++i;
            mt = mount(sub_dev_name, mount_path_name, FAT_FILE_SYSTEM_NAME, FAT_CACHE_NUM, NULL);
            if (!mt) {
                printf("%s mount %s %s fail, format...", sub_dev_name, mount_path_name, FAT_FILE_SYSTEM_NAME);
                f_format(sub_dev_name, FAT_FILE_SYSTEM_NAME, 32 * 1024);
                mt = mount(sub_dev_name, id ? "storage/sd1" : "storage/sd0", FAT_FILE_SYSTEM_NAME, FAT_CACHE_NUM, NULL);
            } else {
                printf("%s mount %s %s suss", sub_dev_name, mount_path_name, FAT_FILE_SYSTEM_NAME);
            }
        }
    } while (fd != NULL && i < 10);

    if (i == 0)
#endif
    {
        mt = mount(dev, id ? "storage/sd1" : "storage/sd0", FAT_FILE_SYSTEM_NAME, FAT_CACHE_NUM, NULL);
        if (!mt) {
            printf("mount %s fail\n", sd_name);
            err = -EFAULT;
            goto __err;
        } else {
            printf("mount %s suss\n", sd_name);
        }
    }

#if RCSP_MODE
    if (mt) {
        set_rcsp_mount_mt(&mt);
    }
#endif

    fd = dev_open(dev, 0);
    if (!fd) {
        err = -EFAULT;
        goto __err;
    }

    dev_ioctl(fd, SD_IOCTL_GET_CLASS, (u32)&class);
    if (class == SD_CLASS_10) {
        log_i("%s card class: 10\n", sd_name);
    } else {
        log_w("%s card class: %d\n", sd_name, class * 2);
    }

    dev_close(fd);

__err:
    sd_mount[id] = err ? SD_MOUNT_FAILD : SD_MOUNT_SUSS;
__exit:
    os_mutex_post(&sd_mutex);

    return err;
}

static void unmount_sd_to_fs(const char *sd_name)
{
    int id = sd_name[2] - '0';
    os_mutex_pend(&sd_mutex, 0);
#ifdef CONFIG_DMSDX_ENABLE
    u8 i = 0;
    char mount_path_name[16] = {0};

    strcpy(mount_path_name, id ? "storage/sd1" : "storage/sd0");
    mount_path_name[11] = '.';

    while (1) {
        mount_path_name[12] = '0' + i;
        if (!fmount_exist(mount_path_name)) {
            break;
        }
        unmount(mount_path_name);
        printf("unmount %s", mount_path_name);
        ++i;
    }

    if (i == 0)
#endif
    {
        unmount(id ? "storage/sd1" : "storage/sd0");
    }
    sd_mount[id] = SD_UNMOUNT;
    printf("unmount %s\n", sd_name);
    os_mutex_post(&sd_mutex);
}

int storage_device_ready(void)
{
    if (!dev_online(SDX_DEV)) {
        return false;
    }
    if (SDX_DEV[2] - '0' < 2 && sd_mount[SDX_DEV[2] - '0'] == SD_UNMOUNT) {
        mount_sd_to_fs(SDX_DEV);
    }

    return fmount_exist(CONFIG_STORAGE_PATH);
}

int sdcard_storage_device_ready(const char *sd_name)
{
    int id = sd_name[2] - '0';

    if (!dev_online(sd_name)) {
        return false;
    }
    if (sd_mount[id] == SD_UNMOUNT) {
        mount_sd_to_fs(sd_name);
    }

    return fmount_exist(id ? "storage/sd1" : "storage/sd0");
}

int sdcard_storage_device_format(const char *sd_name)
{
    int err;

    if (!sd_name || strlen(sd_name) != 3) {
        return -EPERM;
    }

    unmount_sd_to_fs(sd_name);

    err = f_format(sd_name, FAT_FILE_SYSTEM_NAME, 32 * 1024);
    if (err == 0) {
        mount_sd_to_fs(sd_name);
    }

    return err;
}

#ifdef CONFIG_DMSDX_ENABLE
int sdcard_storage_subdevice_format(const char *dev_name)
{
    int err;
    char sd_name[6] = {0};

    if (!dev_name || strlen(dev_name) != 5) {
        return -EPERM;
    }

    memcpy(sd_name, dev_name, 3);

    unmount_sd_to_fs(sd_name);

    err = f_format(dev_name, FAT_FILE_SYSTEM_NAME, 32 * 1024);
    if (err == 0) {
        mount_sd_to_fs(sd_name);
    }

    return err;
}
#endif

/*
 * sd卡插拔事件处理
 */
static void sd_event_handler(struct device_event *event)
{
    switch (event->event) {
    case DEVICE_EVENT_IN:
        printf("%s: in\n", (const char *)event->arg);
        mount_sd_to_fs(event->arg);
        break;
    case DEVICE_EVENT_OUT:
        printf("%s: out\n", (const char *)event->arg);
        unmount_sd_to_fs(event->arg);
        break;
    }
}


#if TCFG_UDISK_ENABLE

enum {
    UDISK_UNMOUNT = 0,
    UDISK_MOUNT_SUSS,
    UDISK_MOUNT_FAILD,
};

static OS_MUTEX udisk_mutex;
static u8 udisk_mount[2];
static u8 udisk_online[2];

static int __udisk_mutex_init()
{
    return os_mutex_create(&udisk_mutex);
}
early_initcall(__udisk_mutex_init);

static int mount_udisk_to_fs(int id)
{
    int err = 0;
    struct imount *mt;
    u32 sta = 0;
    const char *name = id ? "udisk1" : "udisk0";

    err = os_mutex_pend(&udisk_mutex, 0);
    if (err) {
        return -EFAULT;
    }

    if (udisk_mount[id] == UDISK_MOUNT_SUSS) {
        goto __exit;
    }
    if (udisk_mount[id] == UDISK_MOUNT_FAILD) {
        err = -EFAULT;
        goto __exit;
    }

    sta = dev_online(name);
    if (!sta) {
        err = -EFAULT;
        goto __exit;
    }

    mt = mount(name, id ? "storage/udisk1" : "storage/udisk0", FAT_FILE_SYSTEM_NAME, FAT_CACHE_NUM, NULL);
    if (!mt) {
        printf("mount %s fail\n", name);
        err = -EFAULT;
    } else {
        printf("mount %s suss\n", name);
    }

__err:
    udisk_mount[id] = err ? UDISK_MOUNT_FAILD : UDISK_MOUNT_SUSS;
__exit:
    os_mutex_post(&udisk_mutex);

    return err;
}

static void unmount_udisk_to_fs(int id)
{
    os_mutex_pend(&udisk_mutex, 0);
    unmount(id ? "storage/udisk1" : "storage/udisk0");
    udisk_mount[id] = UDISK_UNMOUNT;
    printf("unmount udisk%d\n", id);
    os_mutex_post(&udisk_mutex);
}

int udisk_storage_device_ready(int id)
{
    u32 sta = 0;

    sta = dev_online(id ? "udisk1" : "udisk0");
    if (!sta) {
        return false;
    }
    if (udisk_mount[id] == UDISK_UNMOUNT) {
        mount_udisk_to_fs(id);
    }
    return fmount_exist(id ? "storage/udisk1" : "storage/udisk0");
}

int udisk_storage_device_format(int id)
{
    int err;

    unmount_udisk_to_fs(id);

    err = f_format(id ? "udisk1" : "udisk0", FAT_FILE_SYSTEM_NAME, 32 * 1024);
    if (err == 0) {
        mount_udisk_to_fs(id);
    }

    return err;
}


#endif

static u8 incharge;

int sys_power_is_charging(void)
{
    return incharge;
}

static void otg_event_handler(struct device_event *event)
{
    if (event->event == DEVICE_EVENT_IN) {
        if (((const char *)event->value)[0] == 'h') {
            int usb_id = ((const char *)event->value)[2] - '0';
            log_i("usb%d connect, host mode\n", usb_id);
#if TCFG_USB_HOST_ENABLE
            usb_host_mount(usb_id, MOUNT_RETRY, MOUNT_RESET, MOUNT_TIMEOUT);
#endif
        } else if (((const char *)event->value)[0] == 'c') {
            incharge = 1;
            //插入充电，插入检测会有2秒延迟
        }
    } else if (event->event == DEVICE_EVENT_OUT) {
        if (((const char *)event->value)[0] == 'h') {
            int usb_id = ((const char *)event->value)[2] - '0';
            log_i("usb%d disconnect, host mode\n", usb_id);
#if TCFG_USB_HOST_ENABLE
            usb_host_unmount(usb_id);
#endif
        } else if (((const char *)event->value)[0] == 'c') {
            incharge = 0;
            //拔出充电，拔出检测会有5秒延迟
        }
    }
}

#if TCFG_USB_HOST_ENABLE
static void usb_host_event_handler(struct device_event *event)
{
    int usb_id;

    if (event->event == DEVICE_EVENT_IN) {
        usb_id = ((const char *)event->arg)[8] - '0';
        log_i("usb host%d mount succ\n", usb_id);
#if TCFG_UDISK_ENABLE
        if (!strncmp((const char *)event->value, "udisk", 5)) {
            //上电发现U盘原本挂在USB则出现不进中断，无法挂载文件系统，U盘断电就解决
            mount_udisk_to_fs(usb_id);
        }
#endif
#if TCFG_HOST_UVC_ENABLE
        extern int usb_host_video_init(const usb_dev usb_id, const u8 sub_id);
        if (!strncmp((const char *)event->value, "uvc", 3)) {
            usb_host_video_init(usb_id, usb_id);
        }
#endif
#if TCFG_HOST_AUDIO_ENABLE
        if (!strncmp((const char *)event->value, "audio", 5)) {
            usb_audio_start_process(usb_id);
        }
#endif
#if TCFG_HID_HOST_ENABLE
        if (!strncmp((const char *)event->value, "hid", 3)) {
            hid_process(usb_id);
        }
#endif
#if TCFG_HOST_CDC_ENABLE
        if (!strncmp((const char *)event->value, "cdc", 3)) {
            usb_cdc_init(usb_id);
            //host_cdc_send_demo(usb_id);
        }
#endif
    } else if (event->event == DEVICE_EVENT_OUT) {
        usb_id = ((const char *)event->arg)[8] - '0';
        log_i("usb host%d unmount\n", usb_id);
#if TCFG_UDISK_ENABLE
        if (!strncmp((const char *)event->value, "udisk", 5)) {
            unmount_udisk_to_fs(usb_id);
        }
#endif
#if TCFG_HOST_UVC_ENABLE
        extern int uvc_host_camera_out(u8 usb_id);
        if (!strncmp((const char *)event->value, "uvc", 3)) {
            uvc_host_camera_out(usb_id);
        }
#endif
#if TCFG_HOST_AUDIO_ENABLE
        if (!strncmp((const char *)event->value, "audio", 5)) {
            usb_audio_stop_process(usb_id);
        }
#endif
#if TCFG_HID_HOST_ENABLE
        if (!strncmp((const char *)event->value, "hid", 3)) {
            hid_stop_process(usb_id);
        }
#endif
#if TCFG_ADB_ENABLE
        if (!strncmp((const char *)event->value, "adb", 3)) {
            adb_stop_process(usb_id);
        }
#endif
#if TCFG_HOST_WIRELESS_ENABLE
        if (!strncmp((const char *)event->value, "wireless", 8)) {
            usb_net_stop_process(usb_id);
        }
        if (!strncmp((const char *)event->value, "at_port", 7)) {
            usb_net_at_port_stop_process(usb_id);
        }
#endif
#if TCFG_HOST_CDC_ENABLE
        if (!strncmp((const char *)event->value, "cdc", 3)) {
            usb_cdc_exit(usb_id);
        }
#endif
    }
}
#endif

static int device_event_handler(struct sys_event *e)
{
    struct device_event *event = (struct device_event *)e->payload;

    if (e->from == DEVICE_EVENT_FROM_OTG) {
#if TCFG_USB_SLAVE_ENABLE
        extern int pc_device_event_handler(struct sys_event * e);
        if (pc_device_event_handler(e)) {
            return 1;
        }
#endif
        otg_event_handler(event);
    } else if (e->from == DEVICE_EVENT_FROM_SD) {
        sd_event_handler(event);
#if TCFG_USB_SLAVE_AUDIO_ENABLE
    } else if (e->from == DEVICE_EVENT_FROM_UAC) {
        extern int usb_audio_event_handler(struct device_event * event);
        return usb_audio_event_handler(event);
#endif
#if TCFG_USB_HOST_ENABLE
    } else if (e->from == DEVICE_EVENT_FROM_USB_HOST) {
        usb_host_event_handler(event);
#endif
#if TCFG_EQ_ENABLE && TCFG_EQ_ONLINE_ENABLE && defined TCFG_COMM_TYPE && (TCFG_COMM_TYPE == TCFG_USB_COMM)
    } else if (e->from == DEVICE_EVENT_FROM_CFG_TOOL) {
        extern void usb_cdc_read_data_handler(struct device_event * event);
        usb_cdc_read_data_handler(event);
#endif
    }

    return 0;
}

/*
 * 静态注册设备事件回调函数，优先级为0
 */
SYS_EVENT_STATIC_HANDLER_REGISTER(device_event) = {
    .event_type     = SYS_DEVICE_EVENT,
    .prob_handler   = device_event_handler,
    .post_handler   = NULL,
};

extern int sdfile_ext_init(const char *dev_name, u32 sdfile_ext_base_addr, u32 sdfile_ext_end_addr);
extern u32 boot_info_get_sfc_base_addr(void);
extern u32 ex_app_info_get_base_addr(void);

#ifdef CONFIG_SDFILE_EXT_ENABLE
void sdfile_ext_mount_init(void)
{
    while (!dev_online(SDX_DEV)) {
        os_time_dly(1);
    }

    /* sdfile_ext_init(SDX_DEV, boot_info_get_sfc_base_addr(), __FLASH_SIZE__); */
    sdfile_ext_init(SDX_DEV, ex_app_info_get_base_addr(), CONFIG_SDNAND_HFS_LEN);
}
#endif
