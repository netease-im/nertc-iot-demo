#include "product_main.h"

#ifdef PRODUCT_TEST_ENABLE

static struct dev_ctl_type {
    u8 *args_str;
    u8 conn_status;
    OS_SEM conn_sem;
    u32 ping_on, ping_total;
} *__THIS = NULL;


__attribute__((weak)) u8 product_res_write(u8 *data, u32 len)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_res_read(u8 *data, u32 len)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_rtc_default_wr(struct product_rtc_time *time, u8 is_write)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_uuid_wr(u8 *uuid, u8 is_write)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_sn_wr(u8 *sn, u8 is_write)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_rf_mac_wr(u8 *type, u8 *mac, u8 is_write)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_write_license(u8 idx, u8 *buf, u32 len, u32 file_size)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_read_license(u8 *idx, u8 *buf, u32 *len)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_erase_license(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_erase_screens(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_write_firmware(u32 idx, u8 *buf, u32 len, u32 file_size)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_write_bootscreens(u8 idx, u8 *buf, u32 len, u32 file_size)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_read_bootscreens(u8 *buf, u32 len)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_write_shutdown_screens(u8 idx, u8 *buf, u32 len, u32 file_size)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_read_shutdown_screens(u8 *buf, u32 len)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_read_user_data(u8 *idx, u8 *buf, u32 *len)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_write_user_data(u8 idx, u8 *buf, u32 len, u32 file_size)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_write_options(json_object *options_obj)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_sd_get_info(u32 *status, u32 *cap_size, u32 *block_size)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_sd_testfile_wr_check(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_battery_get_info(u8 *power_percent)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_lcd_init(void)
{
    return ERR_NULL;
}


__attribute__((weak)) u8 product_lcd_color_test(u8 on)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_lcd_get_info(u32 *width, u32 *height)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_camera_reg_wr(u32 addr, u32 *value, u8 is_write, u8 off)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_camera_ntv_ctl(u8 on)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_camera_light_ctl(u8 on)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_camera_get_info(struct procudt_camera_info *info)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_pir_init(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_pir_monitor_ctl(u8 on)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}


__attribute__((weak)) u8 product_license_flag_wr(u8 *flag, u8 is_write)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_motor_init(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_motor_ctl(u8 cmd, int flag, int step)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_gsensor_init(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_gesnsor_monitor_ctl(u8 on)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_touchpanel_init(void)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_touchpanel_monitor_ctl(u8 on)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) int product_key_event_handler(struct sys_event *event)
{
    return ERR_NULL;
}

__attribute__((weak)) u8 product_led_ctl(u8 cmd)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

__attribute__((weak)) u8 product_uvc_switch(u8 idx)
{
    return ERR_NO_SUPPORT_DEV_CMD;
}

u8 *get_args_str(void)
{
    return __THIS->args_str;
}


static u8 *json_itoa(u32 value, u8 base)
{
    static u8 str[16];

    switch (base) {
    case 10:
        sprintf(str, "%d", value);
        break;

    case 16:
        sprintf(str, "0x%x", value);
        break;

    default:
        sprintf(str, "%d", 0);
        break;
    }
    return str;
}


static u8 sd_control_handler(u8 id, cmd, json_object *args_obj)
{
    u8 rscorr = ERR_NULL;
    u32 status, cap_size, block_size;
    struct json_object *sub_obj;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        if (rscorr = product_sd_get_info(&status, &cap_size, &block_size)) {
            break;
        }
        sub_obj = json_object_new_object();
        json_object_object_add(sub_obj, "online", json_object_new_string(json_itoa(status, 10)));
        json_object_object_add(sub_obj, "cap_size", json_object_new_string(json_itoa(cap_size / 1024 * block_size, 10)));
        json_object_object_add(sub_obj, "block_size", json_object_new_string(json_itoa(block_size, 10)));
        asprintf(&__THIS->args_str, "%s", json_object_to_json_string(sub_obj));
        json_object_put(sub_obj);
        break;

    case CTL_SD_FILE_WR:
        rscorr = product_sd_testfile_wr_check();
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 battery_control_handler(u8 id, cmd, json_object *args_obj)
{
    u8 percent;
    u8 rscorr = ERR_NULL;
    struct json_object *sub_obj;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        if (rscorr = product_battery_get_info(&percent)) {
            break;
        }
        sub_obj = json_object_new_object();
        json_object_object_add(sub_obj, "power", json_object_new_string(json_itoa(percent, 10)));
        asprintf(&__THIS->args_str, "%s", json_object_to_json_string(sub_obj));
        json_object_put(sub_obj);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 lcd_control_handler(u8 id, cmd, json_object *args_obj)
{
    json_object *sub_obj;
    u32 width, height;
    u8 rscorr = ERR_NULL, *str;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        if (rscorr = product_lcd_get_info(&width, &height)) {
            break;
        }
        sub_obj = json_object_new_object();
        json_object_object_add(sub_obj, "width", json_object_new_string(json_itoa(width, 10)));
        json_object_object_add(sub_obj, "height", json_object_new_string(json_itoa(height, 10)));
        asprintf(&__THIS->args_str, "%s", json_object_to_json_string(sub_obj));
        json_object_put(sub_obj);
        break;

    case CTL_LCD_COLOR_SWITCH_ON:
        rscorr = product_lcd_color_test(1);
        break;

    case CTL_LCD_COLOR_SWITCH_OFF:
        rscorr = product_lcd_color_test(0);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}





static u8 pir_control_handler(u8 id, cmd, json_object *args_obj)
{
    u8 rscorr = ERR_NULL;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        break;

    case CTL_DEV_MONITOR_START:
        product_pir_monitor_ctl(1);
        break;

    case CTL_DEV_MONITOR_STOP:
        product_pir_monitor_ctl(0);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 motor_control_handler(u8 id, cmd, json_object *args_obj)
{
    int step = 0, flag;
    u8 rscorr = ERR_NULL;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_MOTOR_LEFT:
    case CTL_MOTOR_RIGHT:
    case CTL_MOTOR_UP:
    case CTL_MOTOR_DOWN:
    case CTL_MOTOR_STOP:
        flag = json_object_get_int(json_object_object_get(args_obj, "flag"));
        if (flag) {
            step = json_object_get_int(json_object_object_get(args_obj, "step"));
        }
        rscorr = product_motor_ctl(cmd, flag, step);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 gsensor_control_handler(u8 id, cmd, json_object *args_obj)
{
    u8 rscorr = ERR_NULL;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        break;

    case CTL_DEV_MONITOR_START:
        product_gesnsor_monitor_ctl(1);
        break;

    case CTL_DEV_MONITOR_STOP:
        product_gesnsor_monitor_ctl(0);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 touchpanel_control_handler(u8 id, cmd, json_object *args_obj)
{
    u8 rscorr = ERR_NULL;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        break;

    case CTL_DEV_MONITOR_START:
        product_touchpanel_monitor_ctl(1);
        break;

    case CTL_DEV_MONITOR_STOP:
        product_touchpanel_monitor_ctl(0);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 camera_control_handler(u8 id, cmd, json_object *args_obj)
{
    u32 addr, value;
    json_object *sub_obj;
    struct procudt_camera_info info;
    u8 rscorr = ERR_NULL, *str, array_cnt, idx;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        rscorr = product_camera_get_info(&info);
        sub_obj = json_object_new_object();
        json_object_object_add(sub_obj, "name", json_object_new_string(info.name));
        json_object_object_add(sub_obj, "fps", json_object_new_string(json_itoa(info.fps, 10)));
        json_object_object_add(sub_obj, "width", json_object_new_string(json_itoa(info.width, 10)));
        json_object_object_add(sub_obj, "height", json_object_new_string(json_itoa(info.height, 10)));
        asprintf(&__THIS->args_str, "%s", json_object_to_json_string(sub_obj));
        json_object_put(sub_obj);
        break;

    case CTL_READ_REG:
        if (!(array_cnt = json_object_array_length(args_obj))) {
            rscorr = ERR_PARAMS;
            break;
        }
        for (u8 i = 0; i < array_cnt; i++) {
            sub_obj = json_object_array_get_idx(args_obj, i);
            addr = strtoul(json_object_get_string(json_object_object_get(sub_obj, "addr")) + 2, NULL, 16);
            value = strtoul(json_object_get_string(json_object_object_get(sub_obj, "value")) + 2, NULL, 16);
            if (rscorr = product_camera_reg_wr(addr, &value, 0, 0)) {
                break;
            }
            json_object_string_set(json_object_object_get(sub_obj, "value"), json_itoa(value, 16), strlen(json_itoa(value, 16)));
        }
        asprintf(&__THIS->args_str, "%s", json_object_get_string(args_obj));
        product_camera_reg_wr(0, 0, 0, 1);
        break;

    case CTL_WRITE_REG:
        if (!(array_cnt = json_object_array_length(args_obj))) {
            rscorr = ERR_PARAMS;
            break;
        }
        for (u8 i = 0; i < array_cnt; i++) {
            sub_obj = json_object_array_get_idx(args_obj, i);
            addr = strtoul(json_object_get_string(json_object_object_get(sub_obj, "addr")) + 2, NULL, 16);
            value = strtoul(json_object_get_string(json_object_object_get(sub_obj, "value")) + 2, NULL, 16);
            if (rscorr = product_camera_reg_wr(addr, &value, 1, 0)) {
                break;
            }
        }
        product_camera_reg_wr(0, 0, 0, 1);
        break;

    case CTL_CAMERA_NVG_ON:
        rscorr = product_camera_ntv_ctl(1);
        break;

    case CTL_CAMERA_NVG_OFF:
        rscorr = product_camera_ntv_ctl(0);
        break;

    case CTL_CAMERA_LIGHT_ON:
        rscorr = product_camera_light_ctl(1);
        break;

    case CTL_CAMERA_LIGHT_OFF:
        rscorr = product_camera_light_ctl(0);
        break;

    case CTL_CAMERA_SWITCH:
        idx = json_object_get_int(json_object_object_get(args_obj, "idx"));
        rscorr = product_uvc_switch(idx);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


static u8 led_control_handler(u8 id, cmd, json_object *args_obj)
{
    u8 rscorr = ERR_NULL;

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_LED_ON:
    case CTL_LED_OFF:
        rscorr = product_led_ctl(cmd);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }
    return rscorr;
}


#if(defined(CONFIG_WIFI_ENABLE) && !defined(PRODUCT_NET_CLIENT_ENABLE))

static int wifi_event_callback(void *network_ctx, enum WIFI_EVENT event)
{
    struct wifi_store_info wifi_default_mode_parm = {0};
    static u8 sign_ssid[33] = "GJ111", sign_pwd[65] = "8888888899";

    switch (event) {
    case WIFI_EVENT_MODULE_INIT:
        puts("|network_user_callback->WIFI_EVENT_MODULE_INIT\n");

        //wifi_default_mode_parm.mode = AP_MODE;
        wifi_default_mode_parm.mode = SMP_CFG_MODE;

        if (wifi_default_mode_parm.mode <= AP_MODE) {
            strncpy((char *)wifi_default_mode_parm.ssid[wifi_default_mode_parm.mode - STA_MODE], sign_ssid, sizeof(wifi_default_mode_parm.ssid[wifi_default_mode_parm.mode - STA_MODE]) - 1);
            strncpy((char *)wifi_default_mode_parm.pwd[wifi_default_mode_parm.mode - STA_MODE], sign_pwd, sizeof(wifi_default_mode_parm.pwd[wifi_default_mode_parm.mode - STA_MODE]) - 1);
        }
        wifi_set_default_mode(&wifi_default_mode_parm, 1, 0);//wifi_default_mode_parm.mode == STA_MODE);
        break;

    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");
        break;

    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;

    case WIFI_EVENT_AP_START:
        printf("|network_user_callback->WIFI_EVENT_AP_START,CH=%d\n", wifi_get_channel());
        break;
    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");
        break;

    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        break;
    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;
    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
        break;
    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
        __THIS->conn_status = 0;
        break;
    case WIFI_EVENT_STA_SCAN_COMPLETED:
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
        break;
    case WIFI_EVENT_STA_CONNECT_SUCC:
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", wifi_get_channel());
        break;

    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;
    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        //cpu_reset();
        os_sem_post(&__THIS->conn_sem);
        __THIS->conn_status = 0;
        break;

    //case WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL:
    //    puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL .....\n");
    //    //cpu_reset();
    //    os_sem_post(&__THIS->conn_sem);
    //    __THIS->conn_status = 0;
    //    break;

    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        os_sem_post(&__THIS->conn_sem);
        __THIS->conn_status = 1;
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_P2P_START:
        puts("|network_user_callback->WIFI_EVENT_P2P_START\n");
        break;
    case WIFI_EVENT_P2P_STOP:
        puts("|network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;
    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_SMP_CFG_START:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_START\n");
        break;
    case WIFI_EVENT_SMP_CFG_STOP:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_STOP\n");
        break;
    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        break;
    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        break;

    case WIFI_EVENT_PM_SUSPEND:
        puts("|network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        break;
    case WIFI_EVENT_PM_RESUME:
        puts("|network_user_callback->WIFI_EVENT_PM_RESUME\n");
        break;
    case WIFI_EVENT_AP_ON_ASSOC:
        puts("WIFI_EVENT_AP_ON_ASSOC\n");
        break;
    case WIFI_EVENT_AP_ON_DISCONNECTED:
        puts("WIFI_EVENT_AP_ON_DISCONNECTED\n");
        break;
    default:
        break;
    }

    return 0;
}


static u8 wifi_switch_mode(u8 mode, u8 *ssid, u8 *pwd)
{

    u8 rscorr = ERR_NULL;

    if (!ssid || !pwd || mode < STA_MODE || mode > P2P_MODE) {
        return ERR_PARAMS;
    }
    product_info("mode = %d, ssid = %s, pwd = %s\n", mode, ssid, pwd);

    switch (mode) {
    case STA_MODE:
        wifi_enter_sta_mode(ssid, pwd);
        __THIS->conn_status = 0;
        os_sem_set(&__THIS->conn_sem, 0);
        os_sem_pend(&__THIS->conn_sem, 30 * 100);
        rscorr = __THIS->conn_status ? ERR_NULL : ERR_DEV_FAULT;
        if (rscorr) {
            wifi_enter_smp_cfg_mode();
        }
        break;

    case AP_MODE:
        //rscorr = wifi_enter_ap_mode(ssid, pwd) ? ERR_DEV_FAULT : ERR_NULL;
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;

    case SMP_CFG_MODE:
        break;

    case MP_TEST_MODE:
        break;

    case P2P_MODE:
        break;

    default:
        break;
    }
    return rscorr;
}


static void ping_cb(void *priv, u32 cnt, u32 time)
{
    u8 *pingstr;
    s32 interval;
    static u32 last_cnt = 0;

    if (!cnt) {
        last_cnt = cnt;
        return;
    }

    interval = (cnt != last_cnt) ? time : -999;

    asprintf(&pingstr, "{\"opcode\":\"%d\",\"rscorr\":\"%d\",\"params\":{\"id\":\"%d\",\"type\":\"%d\",\"cmd\":\"%d\",\"args\":{\"idx\":\"%d\",\"interval\":\"%d\"}}}", \
             OPC_DEV_CTL, ERR_NULL, 0, DEV_TYPE_WIFI, CTL_NETWORK_PING_RSP, cnt, interval);

    product_info("--->resp ping\n\n%s\n\n", pingstr);
    data_respond(0, DATA_TYPE_OPCODE, pingstr, strlen(pingstr));

    if (pingstr) {
        free(pingstr);
    }
    last_cnt = cnt;
    __THIS->ping_on = (__THIS->ping_on && cnt >= __THIS->ping_total) ? FALSE : TRUE;
}


static u8 wifi_control_handler(u8 id, cmd, json_object *args_obj)
{
    char *rssi;
    u8 *evm, *mac, mac_str[24], strength[8];;
    u32 num = 0, entry_num = 0;
    struct wifi_mode_info cur_info;
    struct wifi_scan_ssid_info *info;
    u8 rscorr = ERR_NULL, mode, *ssid, *pwd, *array_str;
    json_object *mode_obj, *ssid_obj, *pwd_obj, *scan_obj, *array_obj, \
    *sub_obj, *entry_obj, *ip_obj, *interval_obj, *count_obj;

    while (!(wifi_is_on())) {
        os_time_dly(1);
    }

    __THIS->args_str = NULL;

    switch (cmd) {
    case CTL_GET_DEV_INFO:
        break;

    case CTL_WIFI_CHANGE_MODE:
        mode_obj = json_object_object_get(args_obj, "mode");
        ssid_obj = json_object_object_get(args_obj, "ssid");
        pwd_obj  = json_object_object_get(args_obj, "pwd");
        if (!mode_obj || !ssid_obj || !pwd_obj) {
            rscorr = ERR_PARAMS;
            break;
        }

        mode = json_object_get_int(mode_obj);
        ssid = json_object_get_string(ssid_obj);
        pwd  = json_object_get_string(pwd_obj);
        rscorr = wifi_switch_mode(mode, ssid, pwd);
        break;

    case CTL_WIFI_LAUNCH_SCAN:
        rscorr = wifi_scan_req() ? ERR_DEV_FAULT : ERR_NULL;
        break;

    case CTL_WIFI_GET_SCAN_RES:
        if (!(info = wifi_get_scan_result(&num))) {
            rscorr = ERR_NULL;
            break;
        }

        array_obj = json_object_new_array();
        product_info("wifi_sta_scan ssid_num =%d \r\n", num);
        for (int i = 0; i < num; i++) {
            sub_obj = json_object_new_object();
            json_object_object_add(sub_obj, "ssid", json_object_new_string(info[i].ssid));
            json_object_object_add(sub_obj, "rssi", json_object_new_string(json_itoa(info[i].rssi, 10)));
            json_object_object_add(sub_obj, "snr", json_object_new_string(json_itoa(info[i].snr, 10)));
            json_object_array_add(array_obj, sub_obj);
        }

        asprintf(&__THIS->args_str, "%s", json_object_to_json_string(array_obj));
        wifi_clear_scan_result();
        json_object_put(array_obj);
        free(info);
        break;

    case CTL_WIFI_GET_STA_ENTRY_INFO:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break; //exit

        array_obj = json_object_new_array();
        for (int i = 0; i < 8; i++) {
            if (wifi_get_sta_entry_rssi(i, &rssi, &evm, &mac)) {
                break;
            }
            if (*rssi) {
                entry_num++;
                sprintf(mac_str, "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                sub_obj = json_object_new_object();
                json_object_object_add(sub_obj, "mac", json_object_new_string(mac_str));
                json_object_object_add(sub_obj, "rssi", json_object_new_string(json_itoa(*rssi, 10)));
                json_object_object_add(sub_obj, "evm", json_object_new_string(json_itoa(*evm, 10)));
                json_object_array_add(array_obj, sub_obj);
            }
        }

        if (entry_num) {
            asprintf(&__THIS->args_str, "%s", json_object_to_json_string(array_obj));
        }
        break;

    case CTL_NETWORK_PING_START:
#if(defined(CONFIG_WIFI_ENABLE) && !defined(PRODUCT_NET_CLIENT_ENABLE))
#if LWIP_RAW
        if (!__THIS->conn_status) {
            rscorr = ERR_NETWORK_DISCONNECT;
            break;
        }
        if (__THIS->ping_on) {
            rscorr = ERR_NULL;
            break;
        }
        ip_obj = json_object_object_get(args_obj, "ip");
        interval_obj = json_object_object_get(args_obj, "interval");
        count_obj  = json_object_object_get(args_obj, "count");
        if (!ip_obj || !interval_obj || !count_obj) {
            rscorr = ERR_PARAMS;
            break;
        }

        rscorr = ping_init(json_object_get_string(ip_obj), \
                           json_object_get_int(interval_obj), \
                           json_object_get_int(count_obj), \
                           ping_cb, NULL) ? ERR_DEV_FAULT : ERR_NULL;
        __THIS->ping_on = rscorr == ERR_NULL ? TRUE : FALSE;
        __THIS->ping_total = json_object_get_int(count_obj);
#else
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
#endif
#endif
        break;

    case CTL_WIFI_GET_STA_CONN_INFO:
        if (!__THIS->conn_status) {
            rscorr = ERR_NETWORK_DISCONNECT;
            break;
        }

        cur_info.mode = STA_MODE;
        wifi_get_mode_cur_info(&cur_info);

        memset(strength, 0, sizeof(strength));
        sprintf(strength, "%ddBm", wifi_get_rssi());

        sub_obj = json_object_new_object();
        json_object_object_add(sub_obj, "ssid", json_object_new_string(cur_info.ssid));
        json_object_object_add(sub_obj, "rssi", json_object_new_string(strength));
        //json_object_object_add(sub_obj, "cqi", json_object_new_string(json_itoa(wifi_get_cqi(), 10)));
        asprintf(&__THIS->args_str, "%s", json_object_to_json_string(sub_obj));
        json_object_put(sub_obj);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV_CMD;
        break;
    }

    return rscorr;
}
#endif


static u8 devices_list_check(u8 type)
{
    char *str;
    u8 array_cnt;
    json_object *new_obj, *list_obj, *dev_obj;

    asprintf(&str, "{%s}", DEVICES_LIST);
    new_obj = json_tokener_parse(str);
    ASSERT(new_obj != NULL);

    list_obj = json_object_object_get(new_obj, "devs_list");
    ASSERT(list_obj != NULL);

    array_cnt = json_object_array_length(list_obj);
    ASSERT(array_cnt != 0);

    for (u8 i = 0; i < array_cnt; i++) {
        dev_obj = json_object_array_get_idx(list_obj, i);
        if (type == json_object_get_int(json_object_object_get(dev_obj, "type"))) {
            free(str);
            json_object_put(new_obj);
            return TRUE;
        }
    }

    free(str);
    json_object_put(new_obj);
    return FALSE;
}


u8 devices_control_handler(json_object *params_obj)
{
    u8 id, type, cmd, rscorr;
    json_object *id_obj, *type_obj, *cmd_obj, *args_obj;

    id_obj   = json_object_object_get(params_obj, "id");
    type_obj = json_object_object_get(params_obj, "type");
    cmd_obj  = json_object_object_get(params_obj, "cmd");
    args_obj = json_object_object_get(params_obj, "args");

    if (!id_obj || !type_obj || !cmd_obj || !args_obj) {
        return ERR_PARAMS;
    }

    id   = json_object_get_int(id_obj);
    type = json_object_get_int(type_obj);
    cmd  = json_object_get_int(cmd_obj);

    product_info("%s, id = %d, type = %d, cmd = %d\n", __func__, id, type, cmd);

    if (!devices_list_check(type)) {
        return ERR_NO_SUPPORT_DEV;
    }

    switch (type) {
    case DEV_TYPE_SD:
        rscorr = sd_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_LCD:
        rscorr = lcd_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_MIC:
        break;

    case DEV_TYPE_BUGLE:
        break;

    case DEV_TYPE_CAMERA:
        rscorr = camera_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_KEYPAD:
        break;

    case DEV_TYPE_BATTERY:
        rscorr = battery_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_WIFI:
#if(defined(CONFIG_WIFI_ENABLE) && !defined(PRODUCT_NET_CLIENT_ENABLE))
        rscorr = wifi_control_handler(id, cmd, args_obj);
#else
        rscorr = ERR_NO_SUPPORT_DEV;
#endif
        break;

    case DEV_TYPE_PIR:
        rscorr = pir_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_MOTOR:
        rscorr = motor_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_GSENSOR:
        rscorr = gsensor_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_TOUCHPANEL:
        rscorr = touchpanel_control_handler(id, cmd, args_obj);
        break;

    case DEV_TYPE_LED:
        rscorr = led_control_handler(id, cmd, args_obj);
        break;

    default:
        rscorr = ERR_NO_SUPPORT_DEV;
        break;
    }

    return rscorr;
}


u8 devices_module_init(void)
{
    u8 rscorr;

    if (!__THIS) {
        __THIS = zalloc(sizeof(struct dev_ctl_type));
        ASSERT(__THIS);
    }

    product_port_init();

#if(defined(CONFIG_WIFI_ENABLE) && !defined(PRODUCT_NET_CLIENT_ENABLE))
    os_sem_create(&__THIS->conn_sem, 0);
    //wifi_set_sta_connect_timeout(30);
    wifi_set_event_callback(wifi_event_callback);
    wifi_on();
#endif
    rscorr = product_lcd_init();
    rscorr = product_pir_init();
    rscorr = product_motor_init();
    rscorr = product_gsensor_init();
    rscorr = product_touchpanel_init();
    ASSERT(rscorr == ERR_NULL);

    register_sys_event_handler(SYS_KEY_EVENT, 0, 1, product_key_event_handler);
    return rscorr;
}


#endif


