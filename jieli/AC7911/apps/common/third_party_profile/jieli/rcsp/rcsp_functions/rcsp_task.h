#ifndef __RCSP_TASK_H__
#define __RCSP_TASK_H__

#include "typedef.h"
#include "app_config.h"

enum {
    RCSP_TASK_ACTION_FILE_TRANSFER = 0x0,
    RCSP_TASK_ACTION_FILE_DELETE,
    RCSP_TASK_ACTION_DEV_FORMAT,
    RCSP_TASK_ACTION_WATCH_TRANSFER,
    RCSP_TASK_ACTION_BLUK_TRANSFER,
    RCSP_TASK_ACTION_RESET_EX_FLASH,
};

void app_rcsp_task_prepare(u8 type, u8 action, u8 OpCode_SN);
void app_rcsp_task(void);

void app_rcsp_task_switch_stop(void);

#endif // __RCSP_TASK_H__


