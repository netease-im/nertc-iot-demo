// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _BLE_RCSP_ADV_H
#define _BLE_RCSP_ADV_H

#include <stdint.h>
#include "bt_common.h"
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_RCSP_DEMO)

u8 get_ble_adv_notify(void);
u8 get_ble_adv_modify(void);
u8 adv_tws_both_in_charge_box(u8 type);
u8 get_adv_notify_state(void);
void set_adv_notify_state(u8 notify);
int rcsp_make_set_rsp_data(void);
int rcsp_make_set_adv_data(void);
void bt_ble_init_do(void);
int bt_ble_adv_ioctl(u32 cmd, u32 priv, u8 mode);
void set_connect_flag(u8 value);
void upay_ble_mode_enable(u8 enable);
int upay_ble_adv_data_set(void);
int upay_ble_send_data(const uint8_t *data, u16 len);
void upay_ble_regiest_recv_handle(void (*handle)(const uint8_t *data, u16 len));

#endif

#endif
