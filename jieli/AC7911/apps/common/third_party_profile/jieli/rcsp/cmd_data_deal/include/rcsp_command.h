#ifndef __RCSP_COMMAND_H__
#define __RCSP_COMMAND_H__

#include "typedef.h"
#include "app_config.h"

void rcsp_command_send_bt_scan_result(char *name, u8 name_len, u8 *addr, u32 dev_class, char rssi);
void rcsp_command_send_conncecting_bt_status(u8 *addr, u8 status);

#endif // __RCSP_COMMAND_H__
