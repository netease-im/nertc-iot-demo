#ifndef _PRODUCT_PORT_H_
#define _PRODUCT_PORT_H_

void product_port_init(void);
u8 product_uuid_wr(u8 *uuid, u8 is_write);
u8 product_sn_wr(u8 *sn, u8 is_write);
u8 product_rf_mac_wr(u8 *type, u8 *mac, u8 is_write);
u8 product_write_license(u8 idx, u8 *buf, u32 len, u32 file_size);
u8 product_read_license(u8 *idx, u8 *buf, u32 *len);
u8 product_erase_license(void);
u8 product_license_wr(u8 idx, u8 *buf, u32 len, u32 file_size, u8 type);
u8 product_erase_screens(void);
u8 product_write_bootscreens(u8 idx, u8 *buf, u32 len, u32 file_size);
u8 product_read_bootscreens(u8 *buf, u32 len);
u8 product_write_shutdown_screens(u8 idx, u8 *buf, u32 len, u32 file_size);
u8 product_read_shutdown_screens(u8 *buf, u32 len);
u8 product_write_options(json_object *options_obj);
u8 product_sd_get_info(u32 *status, u32 *cap_size, u32 *block_size);
u8 product_sd_testfile_wr_check(void);
u8 product_battery_get_info(u8 *power_percent);
u8 product_lcd_init(void);
u8 product_lcd_color_test(u8 on);
u8 product_lcd_get_info(u32 *width, u32 *height);
u8 product_camera_reg_wr(u32 addr, u32 *value, u8 is_write, u8 off);
u8 product_camera_ntv_ctl(u8 on);
u8 product_camera_light_ctl(u8 on);
u8 product_camera_get_info(struct procudt_camera_info *info);
u8 product_pir_init(void);
u8 pir_monitor_ctl(u8 on);
u8 product_license_flag_wr(u8 *flag, u8 is_write);
u8 product_motor_init(void);
u8 product_motor_ctl(u8 cmd, int flag, int step);
u8 product_gsensor_init(void);
u8 product_gsensor_monitor_ctl(u8 cmd);
u8 product_touchpanel_init(void);
u8 product_touchpanel_monitor_ctl(u8 cmd);
void product_touchpanel_coord_post(int x, int y);
int product_key_event_handler(struct sys_event *event);
u8 product_led_ctl(u8 cmd);




#endif


