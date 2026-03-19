#include "app_config.h"
#include "system/includes.h"
#include "device/device.h"
#include "fs/fs.h"

#ifdef USE_SD_TEST_DEMO

static void sd_fs_test(void)
{
    puts("-------sd_fs_test--------");

    extern int storage_device_ready(void);

    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(2);
    }

    //读写测试
    u8 *test_data = "sd test 123456";
    u8 read_buf[32];
    int wlen;
    int rlen;
    int ret;
    FILE *newFile = NULL; //保存移动文件操作成功后文件在新路径的文件流指针

    memset(read_buf, 0, sizeof(read_buf));

    //1.创建文件
    FILE *fd = fopen(CONFIG_ROOT_PATH"111/test.txt", "w+");
    if (fd) {
        fwrite(test_data, 1, strlen(test_data), fd);
        wlen = flen(fd);
        fclose(fd);
        printf("write file len: %d \n", wlen);
    }

    //2.读取文件
    fd = fopen(CONFIG_ROOT_PATH"111/test.txt", "r");
    if (fd) {
        rlen = flen(fd);//获取整个文件大小
        fread(read_buf, 1, rlen, fd);
        fclose(fd);
        printf("read file : %s  rlen : %d\n", read_buf, rlen);
    }

    //3.创建目录storage/sd0/C/222,并将storage/sd0/C/111/test.txt文件移动过来
    ret = fmk_dir(CONFIG_ROOT_PATH, "/222", 0);
    if (!ret) {
        fd = fopen(CONFIG_ROOT_PATH"111/test.txt", "r");
        if (fd) {
            ret = fmove(fd, "/222/", &newFile, 0, 5); //第二个参数是目标目录，由根目录开始，比如目录是storage/sd0/C/222，则填/222/
        }

        if (newFile) {
            memset(read_buf, 0, sizeof(read_buf));
            rlen = flen(newFile);
            fread(read_buf, 1, rlen, newFile);
            fclose(newFile);
            printf("move read file : %s  rlen : %d\n", read_buf, rlen);

        }
    }


    while (1) {
        os_time_dly(2);
    }

}
static int c_main(void)
{
    os_task_create(sd_fs_test, NULL, 12, 1000, 0, "sd_fs_test");
    return 0;
}
late_initcall(c_main);
#endif
