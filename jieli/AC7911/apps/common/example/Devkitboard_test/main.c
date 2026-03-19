#include "app_config.h"

#ifdef USE_DevKitBoard_TEST_DEMO
#include "device/device.h"//u8
#include "system/includes.h"//GPIO
#include "sys_common.h"
#include "storage_device.h" //fs
#include "ename.h"

#include "lcd_config.h"//LCD_h
#include "yuv_soft_scalling.h"//YUV420p_Soft_Scaling
#include "get_yuv_data.h"//get_yuv_init
#include "wifi/wifi_connect.h"
#include "lcd_drive.h"

#define POST_TASK_NAME  "ui_main_task"

int isc_log_en(void)
{
    return 0;
}

static u8 camera_open_ok = 0;
void get_yuv(u8 *yuv_buf, u32 len, int yuv_in_w, int yuv_in_h)//YUV数据回调线程
{
    /*******将YUV输出数据发送到lcd_data_driver.c处理数据*********************/
    lcd_show_frame(yuv_buf, len, yuv_in_w, yuv_in_h);
}


extern int reverberation_test_close(void);
extern int reverberation_test_open(int sample_rate, u32 msec, const char *sample_source);
extern void set_touch_enable(void);

void DevKitBoard_task(void *priv)
{
    int msg[32];
    os_time_dly(10);
    while (1) {
        os_taskq_pend("taskq", msg, ARRAY_SIZE(msg)); //接收app_music.c中发来的消息 没有消息在这行等待
        switch (msg[1]) {
        case ACTION_MSG_MIC_OPEN://开启mic测试
            printf(">>>>>>>open__mic_test");
            reverberation_test_open(16000, 0, "mic");
            break;
        case ACTION_MSG_MIC_CLOSE://关闭mic测试
            printf(">>>>>>>close_mic_test");
            reverberation_test_close();
            break;
        case ACTION_MSG_LINEIN_OPEN://开启linein测试
            printf(">>>>>>>open__linein_test");
            reverberation_test_open(16000, 0, "linein");
            break;
        case ACTION_MSG_LINEIN_CLOSE://关闭lieni测试
            printf(">>>>>>>close_linein_test");
            reverberation_test_close();
            break;
        case ACTION_MSG_GET_WIFI_SSID://连接wifi
            printf(">>>>>>>get_wifi_ssid");
            wifi_enter_sta_mode("JL_WIFI", "12345678");
            break;
        case ACTION_MSG_SCAN_WIFI://连接wifi
            printf(">>>>>>>scan_wifi");
            /*wifi_scan_req();*/
            break;
        }
    }

}

static int DevKitBoard_task_init(void)
{
    puts("DevKitBoard_task_init \n\n");
    return thread_fork("DevKitBoard_task", 11, 1024, 32, 0, DevKitBoard_task, NULL);
}
late_initcall(DevKitBoard_task_init);

#endif
