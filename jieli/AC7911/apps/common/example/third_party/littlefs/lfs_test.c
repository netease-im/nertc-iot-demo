#include "system/includes.h"
#include "server/audio_server.h"
#include "lfs.h"
#include "fs/fs.h"

#include "device/ioctl_cmds.h"
#include "device/device.h"
#include "generic/version.h"
#include "stdarg.h"
#include "string.h"
#include "fs/fs.h"

static u8 lfs_hdl_init;
static u8 *prompt_buf;
static int prompt_len = 2 * 1024 * 1024;
static lfs_t lfs_hdl;



static void *audio_file_fopen(const char *path, const char *mode)
{
    lfs_file_t *file = zalloc(sizeof(lfs_file_t));
    if (!file) {
        return NULL;
    }
    lfs_file_open(&lfs_hdl, file, path, mode);
    return (void *)file;
}

static int audio_file_fread(void *file, void *buf, u32 len)
{
    return lfs_file_read(&lfs_hdl, file, buf, len);
}

static int audio_file_fwrite(void *file, void *buf, u32 len)
{
    return lfs_file_write(&lfs_hdl, file, buf, len);
}

static int audio_file_fseek(void *file, u32 offset, int orig)
{
    return lfs_file_seek(&lfs_hdl, file, offset, orig);
}

static int audio_file_ftell(void *file)
{
    return lfs_file_tell(&lfs_hdl, file);
}

static int audio_file_flen(void *file)
{
    return lfs_file_size(&lfs_hdl, file);
}

static int audio_file_fclose(void *file)
{
    lfs_file_close(&lfs_hdl, file);
    free(file);
    return 0;
}

const struct audio_vfs_ops load_sdram_ops = {
    .fopen  	= audio_file_fopen,
    .fread  	= audio_file_fread,
    .fwrite 	= audio_file_fwrite,
    .fseek  	= audio_file_fseek,
    .ftell  	= audio_file_ftell,
    .flen   	= audio_file_flen,
    .fclose 	= audio_file_fclose,
};

const struct audio_vfs_ops *get_load_sdram_ops(void)
{
    return &load_sdram_ops;
}



static int lfs_sdram_read(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, void *buffer, lfs_size_t size)
{
    memcpy(buffer, (u32)c->context + block * c->block_size + off, size);
    return 0;
}

static int lfs_sdram_prog(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, const void *buffer, lfs_size_t size)
{
    memcpy((u32)c->context + block * c->block_size + off, buffer, size);
    return 0;
}

static int lfs_sdram_erase(const struct lfs_config *c, lfs_block_t block)
{
    memset((u32)c->context + block * c->block_size, 0, c->block_size);
    return 0;
}

static int lfs_sdram_sync(const struct lfs_config *c)
{
    return 0;
}

static struct lfs_config cfg = {
    // block device operations
    .read  = lfs_sdram_read,
    .prog  = lfs_sdram_prog,
    .erase = lfs_sdram_erase,
    .sync  = lfs_sdram_sync,

    // block device configuration
    .read_size = 1,
    .prog_size = 256,
    .block_size = 4096,
    .cache_size = 4096,
    .block_cycles = 500,
};

void lfs_sdram_prompt_init()
{
    int ret;
    lfs_file_t file;

    if (lfs_hdl_init) {
        return;
    }

    prompt_buf = zalloc(prompt_len);
    if (!prompt_buf) {
        printf("malloc prompt buf fail\n\r");
        return;
    }

    // mount the filesystem
    cfg.context = (void *)prompt_buf;
    cfg.block_count = prompt_len / cfg.block_size;
    cfg.lookahead_size = (cfg.block_count / 8) + (((cfg.block_count / 8) & 7) ? ((8) - ((cfg.block_count / 8) & 7)) : 0);

    if (lfs_mount(&lfs_hdl, &cfg)) {
        // reformat if we can't mount the filesystem
        // this should only happen on the first boot
        lfs_format(&lfs_hdl, &cfg);
        if (lfs_mount(&lfs_hdl, &cfg)) {
            free(prompt_buf);
            puts("lfs_mount fail! \r\n");
            return;
        }

    }
    lfs_hdl_init = 1;
}

void lfs_sdram_prompt_load(char *path, char *name)
{
    int ret;
    lfs_file_t lfs_file;
    char fname[256] = {0};
    int read_len = 4096;
    u8 *read_buf = malloc(read_len);
    if (!read_buf) {
        printf("malloc, lfs file open %s fail\n\r", name);
        return;
    }
    sprintf(fname, "%s%s", path, name);
    FILE *f_file = fopen(fname, "r");
    if (f_file) {
        ret = lfs_file_open(&lfs_hdl, &lfs_file, name, LFS_O_RDWR | LFS_O_CREAT);
        if (ret < 0) {
            printf("lfs file open %s fail\n\r", name);
            fclose(f_file);
            free(read_buf);
            return;
        }
        ret = 1;
        while (ret > 0) {
            ret = fread(read_buf, read_len, 1, f_file);
            if (ret) {
                lfs_file_write(&lfs_hdl, &lfs_file, read_buf, ret);
            }
        }
        lfs_file_close(&lfs_hdl, &lfs_file);
        fclose(f_file);
        free(read_buf);
    }
}

int lfs_sdram_prompt_check(char *fname)
{
    int ret;
    lfs_file_t lfs_file;
    ret = lfs_file_open(&lfs_hdl, &lfs_file, fname, LFS_O_RDONLY);
    if (ret < 0) {
        return -1;
    }
    lfs_file_close(&lfs_hdl, &lfs_file);
    return 0;
}

void *lfs_sdram_prompt_open(char *fname)
{
    int ret;
    lfs_file_t *lfs_file = zalloc(sizeof(lfs_file_t));
    if (!lfs_file) {
        printf("%s, %d, malloc fail\n\r", __func__, __LINE__);
        return NULL;
    }
    ret = lfs_file_open(&lfs_hdl, lfs_file, fname, LFS_O_RDONLY);
    if (ret < 0) {
        printf("%s, %d, lfs file open %s fail\n\r", __func__, __LINE__, fname);
        free(lfs_file);
        return NULL;
    }
    return lfs_file;
}

void lfs_sdram_prompt_close(void *lfs_file)
{
    lfs_file_close(&lfs_hdl, lfs_file);
    free(lfs_file);
}

void close_avi_test(void *priv)
{
    printf(">>>>>>close_avi_test$$$$$$$$$$$$$$$$$$yuyu");

}

#if 0
// entry point
void lfs_test(void)
{
    int ret;
    lfs_file_t file;

    lfs_hdl = lfs_dev_mount();
    if (lfs_hdl == NULL) {
        printf("lfs_test lfs_dev_mount fail! \r\n");
    }

    // read current count
    u32 boot_count = 0;
    ret = lfs_file_open(lfs_hdl, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    if (ret < 0) {
        printf("lfs_test lfs_file_open fail! \r\n");
    }

    lfs_file_read(lfs_hdl, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(lfs_hdl, &file);
    lfs_file_write(lfs_hdl, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(lfs_hdl, &file);

    // print the boot count
    printf("lfs_test boot_count: %d\n", boot_count);

    //reboot the device
}
#endif


#if 0
lfs_file_t lfs_file;
static lfs_t *lfs_h;

static int __lfs_mount(struct imount *mt, int cache_num)
{

    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    int ret;
    lfs_h = lfs_dev_mount();
    if (lfs_h == NULL) {
        printf("lfs_test lfs_dev_mount fail! \r\n");
    }

    struct vfs_partition *part = &mt->part;

    mt->part_num = 0;
    part->offset = 0;
    part->dir[0] = 'C';
    part->dir[1] = '\0';

    return 0;
}

static int __lfs_fopen(FILE *_file, const char *path, const char *mode)
{

    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    int ret = 0;
    ret = lfs_file_open(lfs_h, &lfs_file, path, LFS_O_RDWR | LFS_O_CREAT);
    if (ret < 0) {
        printf("lfs_test lfs_file_open fail! \r\n");
    }
    return 0;
}

static int __lfs_fread(FILE *_file, void *buf, u32 len)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return lfs_file_read(lfs_h, &lfs_file, buf, len);;
}

static int __lfs_fwrite(FILE *_file, void *buf, u32 len)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return lfs_file_write(lfs_h, &lfs_file, buf, len);
}

static int __lfs_fseek(FILE *_file, int offset, int orig)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return lfs_file_seek(lfs_h, &lfs_file, offset, orig);
}

static int __lfs_flen(FILE *_file)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return lfs_file_size(lfs_h, &lfs_file);
}

static int __lfs_fpos(FILE *_file)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return lfs_file_tell(lfs_h, &lfs_file);
}

static int __lfs_fclose(FILE *_file)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    lfs_file_close(lfs_h, &lfs_file);
    /*free(&lfs_file);*/
    return 0;
}

static int __lfs_fscan(struct vfscan *fsn, const char *path)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int __lfs_fsel(struct vfscan *fsn, int sel_mode, FILE *_file, int num)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return 0;
}
static void __lfs_fscan_release(struct vfscan *fsn)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
}

static int __lfs_fget_name(FILE *_file, u8 *name, int len)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int __lfs_fget_attr(FILE *_file, int *attr)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int __lfs_fget_attrs(FILE *_file, struct vfs_attr *attr)
{
    printf("\n [lfs_msg] %s -note %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int __lfs_fget_free_space(struct vfs_devinfo *dev, struct vfs_partition *part, u32 *space)
{
    printf("\n [ERROR] %s -jiji %d\n", __FUNCTION__, __LINE__);
    return 0;
}

REGISTER_VFS_OPERATIONS(lfs_vfs_ops) = {
    .fs_type = "lfs",
    .mount 	= __lfs_mount,
    .fopen 	= __lfs_fopen,
    .fread 	= __lfs_fread,
    .fwrite = __lfs_fwrite,
    .fseek 	= __lfs_fseek,
    .flen 	= __lfs_flen,
    .fpos 	= __lfs_fpos,
    .fclose = __lfs_fclose,
    .fscan  = __lfs_fscan,
    .fsel  = __lfs_fsel,
    .fscan_release = __lfs_fscan_release,
    .fget_free_space = __lfs_fget_free_space,
    .fget_name = __lfs_fget_name,
    .fget_attr = __lfs_fget_attr,
    .fget_attrs = __lfs_fget_attrs,
};

static void Calculation_w_len_frame(u16 len)
{
    static u32 tstart = 0, tdiff = 0;
    static u32 w_len = 0;
    w_len += len ;
    if (!tstart) {
        tstart = timer_get_ms();
    } else {
        tdiff = timer_get_ms() - tstart;
        if (tdiff >= 1000) {
            printf("\n [MSG]lfs_w_len = %dKB/s\n", w_len *  1000 / tdiff / 1024);
            tstart = 0;
            w_len = 0;
        }
    }
}

static u8 data_buf[1024] ALIGNE(4);
static void lfs_to_flash_task(void *priv)
{
    os_time_dly(3);

    FILE *fp;
    FILE *fp1;
    int ret = 0;
    u8 buf[32] = {0};
    u8 str[] = "lfsflash wr test";
    u8 data = 0;

    ret = mount(NULL, "mnt/lfs", "lfs", 0, NULL);
    if (ret == 0) {
        printf(">>>>>>>lfs mount fail, ret = %d\n", ret);
        return;
    } else {
        printf(">>>>>>>lfs mount succ");
    }

    if (ret) {
        fp = fopen("mnt/lfs/C/test.txt", "w+");
        memset(buf, 0, sizeof(buf));
        fread(buf, sizeof(buf), 1, fp);
        printf("test.txt : %s\n", buf);
        fclose(fp);

        fp = fopen("mnt/lfs/C/test.txt", "w+");
        if (fp) {
            printf("fopen succ\n");
            fwrite(str, strlen(str), 1, fp);
            printf("[msg]>>>>>>>>>>> fwrite");
            fclose(fp);
            printf("[msg]>>>>>>>>>>> fclose");

            memset(buf, 0, sizeof(buf));
            fp = fopen("mnt/lfs/C/test.txt", "w+");
            fread(buf, sizeof(buf), 1, fp);
            printf("test.txt : %s\n", buf);
            fclose(fp);
        } else {
            printf("[msg]>>>>>>>>>>> open fail");
        }
    } else {
        printf("extflash mount failed!!!");
    }

#if 0
    u16 w_len = 256;
    if (ret) {
        fp = fopen("mnt/lfs/C/test1.txt", "w+");
        memset(data_buf, 0x55, 1024);
        while (1) {
            fwrite(data_buf, w_len, 1, fp);
            Calculation_w_len_frame(w_len);
            os_time_dly(1);
        }
    }
#endif
    while (1) {
        os_time_dly(100);
    }
}

static int lfs_to_flash_test(void)
{
    puts("lfs_to_flash_task \n\n");
    return thread_fork("lfs_to_flash_task", 11, 1024, 32, 0, lfs_to_flash_task, NULL);
}

late_initcall(lfs_to_flash_test);
#endif
