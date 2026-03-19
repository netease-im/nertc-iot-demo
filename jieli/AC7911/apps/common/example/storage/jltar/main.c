
#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "fs/fs.h"
#include "jltar.h"

extern int storage_device_ready(void);
static void jltar_test(void *priv)
{
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(2);
    }

    JLFILE *fp = jltar_fopen(CONFIG_ROOT_PATH"music.jlp", "b7da096e05ed4af745830cefd2ba4c37.mp3", "r");
    if (fp == NULL) {
        printf("fopen err!");
        return;
    }

    int file_len = jltar_flen(fp);

    char *r_buf = malloc(file_len);
    if (NULL == r_buf) {
        printf("malloc fail\n");
        return;
    }

    //read test
    int ret = jltar_fread(r_buf, 1, fp->fileInfo.filesize, fp);
    if (ret > 0) {
        FILE *out = fopen(CONFIG_ROOT_PATH"out.mp3", "w+");
        if (out) {
            fwrite(r_buf, 1, fp->fileInfo.filesize, out);
            fclose(out);
        } else {
            printf("fopen err!\n");
        }
    }

    //feek test
    memset(r_buf, 0, file_len);
    jltar_fseek(fp, 0, SEEK_SET);
    ret = jltar_fread(r_buf, 1, 128, fp);
    if (ret > 0) {
        put_buf(r_buf, 200);
    }

    memset(r_buf, 0, file_len);
    jltar_fseek(fp, 12, SEEK_SET);
    ret = jltar_fread(r_buf, 1, 128, fp);
    if (ret > 0) {
        put_buf(r_buf, 200);
    }

    jltar_close(fp);
    free(r_buf);
}

static int c_main(void)
{
    if (thread_fork("jltar_test", 10, 512, 0, NULL, jltar_test, NULL) != OS_NO_ERR) {
        printf("thread fork fail\n");
    }
    return 0;
}
late_initcall(c_main);



