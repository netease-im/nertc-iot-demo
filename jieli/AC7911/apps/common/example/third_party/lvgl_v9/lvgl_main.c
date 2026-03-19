#include <stdbool.h>
#include "app_config.h"

#if LV_USE_PERF_MONITOR || LV_USE_MEM_MONITOR
#include "widgets/lv_label.h"
#endif

#ifdef USE_LVGL_UI_DEMO

#include "lv_conf.h"
#include "system/includes.h"
#include "lvgl.h"
#include "lv_demo_benchmark.h"
#include "lv_demo_widgets.h"
#include "lv_example_freetype.h"
#include "lv_demo_flex_layout.h"
#include "lv_demo_multilang.h"
#include "lv_port_disp.h"
#include "lv_demo_render.h"
#include "lv_demo_scroll.h"
#include "lv_demo_stress.h"
#include "lv_demo_transform.h"
#include "lv_demo_vector_graphic.h"
#include "lv_port_indev.h"
#include "lv_port_fs.h"

static void lvgl_fs_test(void)
{
    extern int storage_device_ready(void);
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(5);
        puts("lvgl waitting sd on... ");
    }

#if 1
    lv_obj_t *obpng = lv_img_create(lv_scr_act());
    lv_img_set_src(obpng, CONFIG_ROOT_PATH"icon.png");
    lv_obj_align(obpng, LV_ALIGN_CENTER, 0, 0);
#else
    lv_obj_t *imgbtn = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_PRESSED, NULL, CONFIG_ROOT_PATH"press_icon.bin", NULL);
    lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_RELEASED, NULL, CONFIG_ROOT_PATH"release_icon.bin", NULL);
    lv_obj_align(imgbtn, LV_ALIGN_CENTER, 0, 0);
#endif
}

static void lvgl_main_task(void *priv)
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    lv_port_fs_init();
    lv_tick_set_cb(&timer_get_ms);

    // uncomment one of these demos
    //    lv_example_arc_1();
    lv_demo_benchmark();
    /* lv_example_freetype_1(); */
    /*lv_demo_widgets();*/
    //lv_example_rlottie_1();
    //lv_example_rlottie_2();
    /* lv_example_btn_1(); */
    /*lv_demo_keypad_encoder();*/
    /*lv_demo_music();          */
    /*lv_demo_stress();           */
    /*lv_demo_flex_layout();*/
    /* lv_demo_transform(); */
    /*lvgl_fs_test();*/

    while (1) {
        u32 time_till_next = lv_task_handler();

        if (LV_DEF_REFR_PERIOD > 1 && time_till_next >= 1000 / OS_TICKS_PER_SEC) {
            msleep(time_till_next);
        }
    }
}

static int lvgl_main_task_init(void)
{
    puts("lvgl_main_task_init \n\n");
    //说明由于LVGL没有加延时让出CPU需要配置为最低优先级 高优先级不能长时间占用CPU不然LVGL运行卡顿
    return thread_fork("lvgl_main_task", 1, 8 * 1024, 0, 0, lvgl_main_task, NULL);
}
late_initcall(lvgl_main_task_init);


#endif


