#include "app_config.h"
#include "system/includes.h"
#include "os/os_api.h"

#ifdef USE_AWTK_UI_DEMO
static void awtk_main_task(void *priv)
{
    user_lcd_init();
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(2);
        puts("waitting sd on... ");
    }
    extern void awtk_test(void);
    awtk_test();

    void tk_mem_dump(void);
    sys_timer_add_to_task("sys_timer", NULL, tk_mem_dump, 60 * 1000);
}

static int awtk_main_task_init(void)
{
    puts("awtk_main_task_init \n\n");
    return thread_fork("awtk_main_task", 28, 1024, 0, 0, awtk_main_task, NULL);
}
late_initcall(awtk_main_task_init);
#endif


