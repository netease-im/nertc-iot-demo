#include "system/includes.h"
#include "app_config.h"
#include "update/update.h"
#include "fs/fs.h"
#include "update/update_loader_download.h"

#ifdef USE_LC_FLASH_UPGRADE_DEMO

// 测试例程使用说明
// 本例程为lc_flash_ufw_update.c文件对应的测试例程
//
// 1.将接收到的update.ufw写入到扩展预留区
//   扩展预留区的命名可参考如下, LEN需要根据实际UFW大小预留足够空间
//   UPGRADE.UFW_ADR=AUTO;
//   UPGRADE.UFW_LEN=0x400000;
//   UPGRADE.UFW_OPT=0;
//
// 2.将扩展预留区名传给lc_flash_update_ufw_init()

#define RESERVED_AREA_NAME   "UPGRADE.UFW"

extern void lc_flash_update_ufw_init(char *reserved_area_name);
static void lc_flash_update_start(void *priv)
{
    os_time_dly(300);

    // TODO: 用户将接收到的update.ufw写入到 扩展预留区（）

    lc_flash_update_ufw_init(RESERVED_AREA_NAME);
}

void c_main(void)
{
    if (thread_fork("lc_flash_update_start", 10, 512, 0, NULL, lc_flash_update_start, NULL)) {
        printf("thread fork fail\n");
    }
}

late_initcall(c_main);

#endif

