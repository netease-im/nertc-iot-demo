#include "app_config.h"
#include "system/includes.h"
#include "os/os_api.h"

#ifdef USE_UGFX_UI_DEMO
static void ugfx_main_task(void *priv)
{
    user_lcd_init();
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(2);
        puts("waitting sd on... ");
    }
    int ugfx_main(void);
    ugfx_main();

}

static int ugfx_main_task_init(void)
{
    puts("ugfx_main_task_init \n\n");
    return thread_fork("ugfx_main_task", 28, 1024, 0, 0, ugfx_main_task, NULL);
}
late_initcall(ugfx_main_task_init);
#endif

