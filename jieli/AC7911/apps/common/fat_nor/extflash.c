#include "fs/fs.h"
#include "spiflash.h"
#include "app_config.h"
#include "asm/sfc_norflash_api.h"
#include "event/device_event.h"
#include "system/includes.h"

#ifdef TCFG_EXTFLASH_ENABLE

#define EXTFLASH_BLOCK_SIZE (512)
#define MAX_READ_SIZE       (63 * 1024)

static struct extflash {
    u32 ext_size;
    u32 ext_addr;
} extflash_info = {0};
static struct device extflash_dev;

static int extflash_info_init(void)
{
    struct vfs_attr file_attr;
    FILE *fp;

    if (extflash_info.ext_size) {
        return 0;
    }

    fp = fopen("mnt/sdfile/EXT_RESERVED/extflash", "r");
    if (!fp) {
        printf("get_extflash_addr ERROR!!!\r\n");
        return -1;
    }

    fget_attrs(fp, &file_attr);
    extflash_info.ext_addr = sdfile_cpu_addr2flash_addr(file_attr.sclust);
    extflash_info.ext_size = file_attr.fsize;
    fclose(fp);

    printf("get_extflash_addr = 0x%x, size = 0x%x \r\n", extflash_info.ext_addr, extflash_info.ext_size);

    ASSERT(extflash_info.ext_size > 0, "%s\n", "The space of extflash is zero\n");

    return 0;
}

static int extflash_dev_init(const struct dev_node *node, void *arg)
{
    return norflash_init(node, arg);
}

static int extflash_dev_open(const char *name, struct device **device, void *arg)
{
    extflash_info_init();
    *device = &extflash_dev;
    return norflash_open(NULL, device, arg);
}

static int extflash_dev_read(struct device *device, void *buf, u32 len, u32 offset)
{
    u8 *data = (u8 *)buf;
    int total_len = 0, rlen = 0;
    u32 addr = offset * EXTFLASH_BLOCK_SIZE + extflash_info.ext_addr;
    len *= EXTFLASH_BLOCK_SIZE;

    while (total_len < len) {
        rlen = len - total_len > MAX_READ_SIZE ? MAX_READ_SIZE : len - total_len;
        rlen += norflash_origin_read(data, addr, rlen);
        data += rlen;
        addr += rlen;
        total_len += rlen;
    }

    return (len / EXTFLASH_BLOCK_SIZE); //返回扇区个数
}

static int extflash_dev_write(struct device *device, void *buf, u32 len, u32 offset)
{
    u32 align_size = 4096;
    u8 *data = NULL, *temp = NULL;
    u32 begin, end, addr, size, w_size, remain_size;

    addr = offset * EXTFLASH_BLOCK_SIZE + extflash_info.ext_addr;
    size = len * EXTFLASH_BLOCK_SIZE;

    data  = (u8 *)buf;
    begin = addr / align_size;
    end   = (addr + size) / align_size;
    temp  = malloc(align_size);
    if (!temp) {
        return 0;
    }

    w_size = 0;
    remain_size = align_size - (addr % align_size);
    norflash_origin_read(temp, begin * align_size, align_size);
    if (size <= remain_size) {
        memcpy(temp + (addr % align_size), data, size);
        norflash_erase(IOCTL_ERASE_SECTOR, begin * align_size);
        norflash_write(NULL, temp, align_size, begin * align_size);
    } else {
        memcpy(temp + (addr % align_size), data, remain_size);
        norflash_erase(IOCTL_ERASE_SECTOR, begin * align_size);
        norflash_write(NULL, temp, align_size, begin * align_size);
        w_size += remain_size;

        for (int i = begin + 1; i < end; i++) {
            norflash_erase(IOCTL_ERASE_SECTOR, i * align_size);
            norflash_write(NULL, data + w_size, align_size, i * align_size);
            w_size += align_size;
        }

        if (begin != end && size > w_size) {
            norflash_origin_read(temp, end * align_size, align_size);
            memcpy(temp, data + w_size, size - w_size);
            norflash_erase(IOCTL_ERASE_SECTOR, end * align_size);
            norflash_write(NULL, temp, align_size, end * align_size);
        }
    }
    free(temp);

    return len;
}

static int extflash_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    int err = 0;

    switch (cmd) {
    case IOCTL_GET_ID:
        break;

    case IOCTL_GET_SECTOR_SIZE:
        *((u32 *)arg) = 256;
        break;

    case IOCTL_GET_BLOCK_SIZE:
        *((u32 *)arg) = EXTFLASH_BLOCK_SIZE;
        break;

    case IOCTL_GET_CAPACITY:
        extflash_info_init();
        *((int *)arg) = ((extflash_info.ext_size + EXTFLASH_BLOCK_SIZE - 1) / EXTFLASH_BLOCK_SIZE);
        break;

    case IOCTL_GET_UNIQUE_ID:
        break;

    case IOCTL_ERASE_BLOCK:
        break;

    case IOCTL_ERASE_CHIP:
        break;

    case IOCTL_SET_WRITE_PROTECT:
        break;

    case IOCTL_GET_BLOCK_NUMBER:
        extflash_info_init();
        *(u32 *)arg = ((extflash_info.ext_size + EXTFLASH_BLOCK_SIZE - 1) / EXTFLASH_BLOCK_SIZE);
        break;

    case IOCTL_GET_STATUS:
        *(u32 *)arg = 1;
        break;

    default:
        err = -EINVAL;
        break;
    }

    return err;
}

static int extflash_dev_close(struct device *device)
{
    return 0;
}

const struct device_operations extflash_dev_ops = {
    .init 	= extflash_dev_init,
    .open 	= extflash_dev_open,
    .read 	= extflash_dev_read,
    .write	= extflash_dev_write,
    .ioctl 	= extflash_dev_ioctl,
    .close  = extflash_dev_close,
};

static int extflash_device_event_handler(struct sys_event *e)
{
    struct device_event *event = (struct device_event *)e->payload;
    const char *usb_msg = (const char *)event->value;

    if (e->from == DEVICE_EVENT_FROM_OTG) {
        if (usb_msg[0] == 's') {
            if (event->event == DEVICE_EVENT_IN) {
                //extflash枚举为usb设备上线事件
                //extflash将被枚举为udisk上线后，需要限制小机访问extflash，否则数据将被篡改
                printf("usb %c, extflash online", usb_msg[2]);
            } else if (event->event == DEVICE_EVENT_OUT) {
                //extflash枚举为usb设备下线事件
                printf("usb %c, extflash offline", usb_msg[2]);
            }
        }
    }

    return 0;
}

SYS_EVENT_STATIC_HANDLER_REGISTER(extflash_device_event) = {
    .event_type     = SYS_DEVICE_EVENT,
    .prob_handler   = extflash_device_event_handler,
    .post_handler   = NULL,
};


#ifdef CONFIG_JLFAT_ENABLE
#define FILE_SYSTEM_NAME "jlfat"
#else
#define FILE_SYSTEM_NAME "fat"
#endif
#define FAT_CACHE_NUM 5
int extflash_mount_to_fs_test(void)
{
    FILE *fp;
    u8 buf[] = "extflash wr test";

    int err;
    err	= f_format("extflash", FILE_SYSTEM_NAME, 5 * 1024);
    printf("format err : %d", err);

    if (err == 0 && mount("extflash", "mnt/extflash", FILE_SYSTEM_NAME, FAT_CACHE_NUM, NULL)) {
        printf("extflash mount succ");
        fp = fopen("mnt/extflash/C/test.txt", "w+");
        if (fp) {
            printf("fopen succ\n");
            fwrite(buf, sizeof(buf), 1, fp);
            fclose(fp);

            memset(buf, 0, sizeof(buf));
            fp = fopen("mnt/extflash/C/test.txt", "w+");
            fread(buf, sizeof(buf), 1, fp);
            printf("test.txt : %s\n", buf);
            fclose(fp);
        }
    } else {
        printf("extflash mount failed!!!");
    }

    return 0;
}
static int demo_extflash(void)
{
    return thread_fork("extflash_mount_to_fs_test", 10, 1000, 0, NULL, extflash_mount_to_fs_test, NULL);
}

late_initcall(demo_extflash);
#endif
