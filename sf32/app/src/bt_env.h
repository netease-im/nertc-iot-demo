#ifndef __BT_APP_ENV_H__
#define __BT_APP_ENV_H__
#include "bts2_app_inc.h"
#include "ble_connection_manager.h"
#include "bt_connection_manager.h"
typedef struct
{
    BOOL bt_connected;
    bt_notify_device_mac_t bd_addr;
    rt_timer_t pan_connect_timer;
} bt_app_t;


extern bt_app_t g_bt_app_env; // 声明外部变量

#endif // __BT_APP_ENV_H__